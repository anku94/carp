#pragma once

#include "minicarp_types.h"
#include "receiver.h"
#include "whole_file_reader.h"

#include <pdlfs-common/env.h>

namespace pdlfs {
namespace carp {

class MiniCarp {
 public:
  MiniCarp(const MiniCarpOptions& options)
      : options_(options), producer_(nullptr) {
    queues_.resize(options_.num_ranks);

    producer_ = new WholeFileReader(options_, queues_, this);

    for (int i = 0; i < options_.num_ranks; i++) {
      CarpReceiver* receiver = new CarpReceiver(options_, queues_[i]);
      consumers_.push_back(receiver);
    }

    // execute producer.run and consumers.run using threadpool
  }

  ~MiniCarp() {
    for (int i = 0; i < options_.num_ranks; i++) {
      delete consumers_[i];
    }
    delete producer_;
  }

  void Run() { printf("hell oworld\n"); }

  /* called by a producer */
  void Shutdown() {
    num_shutdown_++;
    if (num_shutdown__ == num_producers_) {
    for (consumer in consumers) {
      consuymer.shutdwon();
    }
    }
    return;
  }

  // XXX: Not sure if these functions are required here, uncomment if necessary
  // producer needs to implement this
  // producer should spawn read from file do some calculations for pivots
  //  virtual Status Produce() {
  //    logf(LOG_INFO, "%s:%d: hello world\n", __func__, __LINE__);
  //    return Status::OK();
  //  }
  //
  //  virtual Status Consume(KVQueue& q) {
  //    logf(LOG_INFO, "%s:%d: hello world\n", __func__, __LINE__);
  //    return Status::OK();
  //  }
  //
  //  virtual Status Insert(moodycamel::ConcurrentQueue<Slice>, KVItem* p_ctx) =
  //  0;
  //
  //  virtual Status Flush() {
  //    logf(LOG_INFO, "%s:%d: hello world\n", __func__, __LINE__);
  //    return Status::OK();
  //  }

 private:
  // close the file used for reading key vval pairs
  friend class CarpReceiver;
  void Close() { logf(LOG_INFO, "%s:%d: hello world\n", __func__, __LINE__); }

  ThreadPool* thpool_;
  std::vector<KVQueue> queues_;
  Producer* producer_;
  std::vector<Consumer*> consumers_;
  const MiniCarpOptions& options_;
};

}  // namespace carp
}  // namespace pdlfs
