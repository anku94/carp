/**
 * @file whole_file_reader.cc
 * @author Amol (amolkulk@andrew.cmu.edu)
 * @brief
 * @date 2021-05-01
 *
 */

#include "whole_file_reader.h"

#include "minicarp.h"

namespace pdlfs {

namespace carp {

Status WholeFileReader::OpenFileHandle(SequentialFile** fh, uint64_t* fsz) {
  if (!fh || !fsz) {
    return Status::InvalidArgument("No file Handle");
  }

  Status s = Status::OK();

	if(options_.env->FileExists(options_.fname.c_str()) != true){
		logf(LOG_INFO, "file does not exist");
		return Status::FileExpected("File does not exist");
	}

	logf(LOG_INFO, "Opening File for data %s", options_.fname.c_str());

  s = options_.env->NewSequentialFile(options_.fname.c_str(), fh);
  if (!s.ok()) return s;

	logf(LOG_INFO, "getting file size");
  options_.env->GetFileSize(options_.fname.c_str(), fsz);
  if (!s.ok()){

		logf(LOG_INFO, "Unable to get file size");
		//TODO close seq file
	}

  return s;
}

int WholeFileReader::ComputeShuffleTarget(KVItem& kv) {
  auto rank_iter = std::lower_bound(pivots_.begin(), pivots_.end(), kv.key);

  int rank = rank_iter - pivots_.begin() -1;

  return rank;
}

void WholeFileReader::Run() {
  logf(LOG_INFO,
       "Maybe get file path from CarpOptions or something, start "
       "reading/shuffling it");

  // open file
  SequentialFile* seq_fh;
  uint64_t fsz;
  Status s = OpenFileHandle(&seq_fh, &fsz);

  if (!(s.ok())) {
    logf(LOG_ERRO, "Error in opening file handle");
  }


	logf(LOG_INFO, "reading kv pairs and file size is %ld", fsz);
  // read key val from file and enqueue it
  while (!shutdown_ && fsz > 0) {  //&& kv pairs left?
    Slice k_sl = Slice();
    float key;

    Slice v_sl = Slice();
    char v_buff[v_size];
		memset(v_buff, 0, v_size);

    uint64_t bytes_rd = 0;

    s = seq_fh->Read(k_size, &k_sl, reinterpret_cast<char*>(&key));

		logf(LOG_INFO, "read key %f", key);

    if (!(s.ok())) {
      logf(LOG_ERRO, "Error in reading from file handle");
    }

    s = seq_fh->Read(v_size, &v_sl, v_buff);

    if (!(s.ok())) {
      logf(LOG_ERRO, "Error in reading from file handle");
    }

    KVItem kv = KVItem(key, v_sl.data());


		//if buffer full shuffle
		//else buffer the kv item
    // shuffling logic: decide which KVItem will be enqueued in which queue

    int rank = ComputeShuffleTarget(kv);

		logf(LOG_INFO, "Adding kv pair to rank %d", rank);
    // TODO: enqqueue_bulk will be faster
    queues_[rank].enqueue(kv);

		fsz -= 24;
  }

  mini_carp_->Shutdown();
}

}  // namespace carp

}  // namespace pdlfs