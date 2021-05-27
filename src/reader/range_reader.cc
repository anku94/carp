//
// Created by Ankush J on 1/22/21.
//

#include "range_reader.h"

#include "query_utils.h"
#include "reader_base.h"

namespace pdlfs {
namespace plfsio {

template <typename T>
Status RangeReader<T>::ReadManifest(const std::string& dir_path) {
  logger_.RegisterBegin("MFREAD");

  Status s = Status::OK();

  dir_path_ = dir_path;
  fdcache_.ReadDirectory(dir_path_, num_ranks_);

  manifest_reader_.EnableManifestOutput(dir_path);

  RandomAccessFile* src;
  std::vector<ManifestReadWorkItem<T>> work_items;
  work_items.resize(num_ranks_);

  task_tracker_.Reset();

  for (int rank = 0; rank < num_ranks_; rank++) {
    work_items[rank].rank = rank;
    work_items[rank].fdcache = &fdcache_;
    work_items[rank].task_tracker = &task_tracker_;
    work_items[rank].manifest_reader = &manifest_reader_;
    thpool_->Schedule(ManifestReadWorker, (void*)(&work_items[rank]));
  }

  task_tracker_.WaitUntilCompleted(num_ranks_);

  uint64_t key_sz, val_sz;
  manifest_.GetKVSizes(key_sz, val_sz);
  logf(LOG_INFO, "Key/Value Sizes: %lu/%lu\n", key_sz, val_sz);

  logger_.RegisterEnd("MFREAD");

  if (options_.analytics_on) {
    logf(LOG_INFO, "Running analytics...\n");
    /* write manifest to plfs/particle/../../plots */
    std::string exp_path = dir_path + "/../../plots";

    Env* env = options_.env;
    if (!env->FileExists(exp_path.c_str())) {
      s = env->CreateDir(exp_path.c_str());
    }

    if (s.ok()) manifest_.GenOverlapStats(exp_path.c_str(), env);
  }

  return s;
}

template <typename T>
Status RangeReader<T>::QueryParallel(int epoch, float rbegin, float rend) {
  logger_.RegisterBegin("SSTREAD");

  PartitionManifestMatch match_obj;
  manifest_.GetOverlappingEntries(epoch, rbegin, rend, match_obj);

  logf(LOG_INFO, "Query Match: %llu SSTs found (%llu items)", match_obj.Size(),
       match_obj.TotalMass());

  match_obj.Print();

  std::vector<KeyPair> query_results;
  RankwiseReadSSTs(match_obj, query_results);

  logger_.RegisterEnd("SSTREAD");
  logger_.RegisterBegin("SORT");

#if defined(PDLFS_TBB)
  std::sort(std::execution::par, query_results.begin(), query_results.end(),
            KeyPairComparator());
#else
  std::sort(query_results.begin(), query_results.end(), KeyPairComparator());
#endif
  logger_.RegisterEnd("SORT");

  logf(LOG_INFO, "Query Results: %zu elements found\n", query_results.size());

#define ITEM(ptile) \
  query_results[((ptile) * (query_results.size() - 1) / 100)].key

  if (!query_results.empty()) {
    logf(LOG_INFO, "Query Results: preview: %.3f %.3f %.3f ... %.3f\n", ITEM(0),
         ITEM(10), ITEM(50), ITEM(100));
  }

  double qsel = match_obj.GetSelectivity();

  logger_.PrintStats();
  logger_.LogQuery(dir_path_.c_str(), epoch, rbegin, rend, qsel);

  return Status::OK();
}

template <typename T>
Status RangeReader<T>::QuerySequential(int epoch, float rbegin, float rend) {
  logger_.RegisterBegin("SSTREAD");

  PartitionManifestMatch match_obj;
  manifest_.GetOverlappingEntries(epoch, rbegin, rend, match_obj);
  logf(LOG_INFO, "Query Match: %llu SSTs found (%llu items)", match_obj.Size(),
       match_obj.TotalMass());

  Slice slice;
  std::string scratch;
  for (uint32_t i = 0; i < match_obj.Size(); i++) {
    PartitionManifestItem& item = match_obj[i];
    // logf(LOG_DBUG, "Item Rank: %d, Offset: %llu\n", item.rank,
    // item.offset);
    ReadBlock(item.rank, item.offset, item.part_item_count * 60, slice,
              scratch);
  }

  logger_.RegisterEnd("SSTREAD");

  logger_.RegisterBegin("SORT");
#if defined(PDLFS_TBB)
  std::sort(std::execution::par, query_results_.begin(), query_results_.end(),
            KeyPairComparator());
#else
  std::sort(query_results_.begin(), query_results_.end(), KeyPairComparator());
#endif
  logger_.RegisterEnd("SORT");

  logf(LOG_INFO, "Query Results: %zu elements found\n", query_results_.size());

  logger_.PrintStats();

  return Status::OK();
}

template <typename T>
Status RangeReader<T>::AnalyzeManifest(const std::string& dir_path) {
  Status s = Status::OK();
  s = ReadManifest(dir_path);
  if (!s.ok()) return s;

  logger_.PrintSingleStat("MFREAD");

  QueryUtils::SummarizeManifest(manifest_);

  std::vector<Query> queries;
  s = QueryUtils::GenQueryPlan(manifest_, queries);
  if (!s.ok()) return s;

  for (size_t qi = 0; qi < queries.size(); qi++) {
    Query& q = queries[qi];
    PartitionManifestMatch match;
    manifest_.GetOverlappingEntries(q, match);

    logf(LOG_INFO, "%s: %s", q.ToString().c_str(), match.ToString().c_str());
    //    QueryParallel(q);
  }

  return s;
}

template <typename T>
void RangeReader<T>::ManifestReadWorker(void* arg) {
  ManifestReadWorkItem<T>* item = static_cast<ManifestReadWorkItem<T>*>(arg);

  RandomAccessFile* src;
  uint64_t src_sz;

  ParsedFooter pf;

  //  item->fdcache->GetFileHandle(item->rank, &src, &src_sz);
  //  RangeReader::ReadFooter(src, src_sz, pf);
  item->fdcache->ReadFooter(item->rank, pf);
  item->manifest_reader->UpdateKVSizes(pf.key_sz, pf.val_sz);
  item->manifest_reader->ReadManifest(item->rank, pf.manifest_data,
                                      pf.manifest_sz);
  item->task_tracker->MarkCompleted();
}

template <typename T>
Status RangeReader<T>::ReadSSTs(PartitionManifestMatch& match,
                                std::vector<KeyPair>& query_results) {
  Slice slice;
  std::string scratch;

  std::vector<SSTReadWorkItem<T>> work_items;
  work_items.resize(match.Size());
  query_results.resize(match.TotalMass());
  task_tracker_.Reset();

  uint64_t mass_sum = 0;
  uint64_t key_sz, val_sz;
  match.GetKVSizes(key_sz, val_sz);

  for (uint32_t i = 0; i < match.Size(); i++) {
    PartitionManifestItem& item = match[i];
    work_items[i].item = &item;
    work_items[i].key_sz = key_sz;
    work_items[i].val_sz = val_sz;

    work_items[i].query_results = &query_results;
    work_items[i].qrvec_offset = mass_sum;
    mass_sum += item.part_item_count;

    work_items[i].fdcache = &fdcache_;
    work_items[i].task_tracker = &task_tracker_;

    thpool_->Schedule(QueryUtils::SSTReadWorker<T>, (void*)&work_items[i]);
  }

  assert(mass_sum == match.TotalMass());

  task_tracker_.WaitUntilCompleted(work_items.size());

  return Status::OK();
}

template <typename T>
Status RangeReader<T>::RankwiseReadSSTs(PartitionManifestMatch& match,
                                        std::vector<KeyPair>& query_results) {
  Slice slice;
  std::string scratch;

  task_tracker_.Reset();

  uint64_t mass_sum = 0;
  uint64_t key_sz, val_sz;
  match.GetKVSizes(key_sz, val_sz);

  std::vector<int> ranks;
  match.GetUniqueRanks(ranks);

  std::vector<RankwiseSSTReadWorkItem<T>> work_items;
  work_items.resize(ranks.size());
  query_results.resize(match.TotalMass());

  logf(LOG_INFO, "Matching ranks: %zu\n", ranks.size());

  for (uint32_t i = 0; i < ranks.size(); i++) {
    int rank = ranks[i];
    work_items[i].rank = rank;
    uint64_t mass_rank = match.GetMatchesByRank(rank, work_items[i].wi_vec);
    work_items[i].key_sz = key_sz;
    work_items[i].val_sz = val_sz;

    work_items[i].query_results = &query_results;
    work_items[i].qrvec_offset = mass_sum;
    mass_sum += mass_rank;

    work_items[i].fdcache = &fdcache_;
    work_items[i].task_tracker = &task_tracker_;

    thpool_->Schedule(QueryUtils::RankwiseSSTReadWorker<T>, (void*)&work_items[i]);
  }

  assert(mass_sum == match.TotalMass());

  task_tracker_.WaitUntilCompleted(work_items.size());

  return Status::OK();
}

template <typename T>
void RangeReader<T>::ReadBlock(int rank, uint64_t offset, uint64_t size,
                               Slice& slice, std::string& scratch,
                               bool preview) {
  Status s = Status::OK();

  scratch.resize(size);
  ReadRequest req;
  req.offset = offset;
  req.bytes = size;
  req.scratch = &scratch[0];
  s = fdcache_.Read(rank, req);
  if (!s.ok()) {
    logf(LOG_ERRO, "Bad Status");
    return;
  }

  slice = req.slice;

  uint64_t num_items = size / 60;
  uint64_t vec_off = query_results_.size();

  uint64_t block_offset = 0;
  while (block_offset < size) {
    KeyPair kp;
    kp.key = DecodeFloat32(&slice[block_offset]);
    // XXX: val?
    query_results_.push_back(kp);

    block_offset += 60;
  }
}

template class RangeReader<SequentialFile>;
template class RangeReader<RandomAccessFile>;
}  // namespace plfsio
}  // namespace pdlfs
