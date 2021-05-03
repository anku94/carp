/**
 * @file whole_file_reader.cc
 * @author Amol (amolkulk@andrew.cmu.edu)
 * @brief 
 * @date 2021-05-01
 * 
 */

#include "whole_file_reader.h"


namespace pdlfs{

namespace carp{

Status WholeFileReader::OpenFileHandle(SequentialFile** fh, uint64_t *fsz) {

  if(!fh || !fsz){
    return Status::InvalidArgument("No file Handle");
  }

  Status s = Status::OK();

  s = options_.env->NewSequentialFile(options_.fname.c_str(), fh);
  if (!s.ok()) return s;

  options_.env->GetFileSize(options_.fname.c_str(), fsz);
  if(!s.ok()) return s;

  return s;
}

int WholeFileReader::ComputeShuffleTarget(KVItem& kv) {

  auto rank_iter = std::lower_bound(pivots_.begin(), pivots_.end(), kv.key);

  int rank = rank_iter - pivots_.begin() - 1;

  return rank;
}

void WholeFileReader::Run() {
  logf(LOG_INFO,
          "Maybe get file path from CarpOptions or something, start "
          "reading/shuffling it");

  //open file
  SequentialFile* seq_fh;
  uint64_t fsz;
  Status s = OpenFileHandle(&seq_fh, &fsz);

  if(!(s.ok())){
      logf(LOG_ERRO, "Error in opening file handle");
  }

  //read key val from file and enqueue it
  while(!shutdown_ ){ //&& kv pairs left?
    Slice k_sl = Slice();
    char *k_buff = new char[k_size];

    Slice v_sl = Slice();
    char *v_buff = new char[v_size];

    uint64_t bytes_rd = 0;

    s = seq_fh->Read(k_size, &k_sl, k_buff);

    float key = *(float*)(k_sl.data());
  
    if(!(s.ok())){
      logf(LOG_ERRO, "Error in reading from file handle");
    }

    s = seq_fh->Read(k_size, &v_sl, v_buff);

    if(!(s.ok())){
      logf(LOG_ERRO, "Error in reading from file handle");
    }

    KVItem *kv = new KVItem(key, v_sl.data());

    //shuffling logic: decide which KVItem will be enqueued in which queue

    int rank = ComputeShuffleTarget(*kv);


    //TODO: enqqueue_bulk will be faster
    queues_[rank].enqueue(*kv);
  }

}

}

}