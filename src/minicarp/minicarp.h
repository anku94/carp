#pragma once


#include "concurrentqueue.h"
#include "pdlfs-common/env.h"


namespace pdlfs{
namespace minicarp{

struct MiniCarpOptions {
  uint32_t my_rank;
  uint32_t num_ranks;
  std::string mount_path;
  std::string fname;
  Env* env;
};

typedef struct particle_list{
  Slice& keys;
  Slice& values;
}particle_list_t;

class MiniCarp{

protected:
 
  std::vector<moodycamel::ConcurrentQueue<particle_list_t>> queues_;

  const MiniCarpOptions& options_;

public:

  MiniCarp(const MiniCarpOptions& options){
    options_ = options;
  }
  
  //producer needs to implement this
  //producer should spawn read from file do some calculations for pivots
  virtual Status Produce(){
    logf(LOG_INFO, "%s:%d: hello world\n", __func__, __LINE__);
    return Status::OK();
  }

  virtual Status Consume(moodycamel::ConcurrentQeueue<particle_list_t>){
    logf(LOG_INFO, "%s:%d: hello world\n", __func__, __LINE__);
    return Status::OK();
  }

  virtual Status Insert(moodycamel::ConcurrentQeueue<Slice>, 
                          particle_list_t *p_ctx);

  virtual Status Flush(){
    logf(LOG_INFO, "%s:%d: hello world\n", __func__, __LINE__);
    return Status::OK();
  }

  //close the file used for reading key vval pairs
  virtual Status Close(){
    logf(LOG_INFO, "%s:%d: hello world\n", __func__, __LINE__);
    return Status::OK();
  }
};


} //minicarp
} //pdlfs
