//
// Created by Ankush J on 4/29/21.
//

#pragma once

#include "concurrentqueue.h"

namespace pdlfs {
namespace carp {

static const int kv_size = 24;
static const int k_size = 4;
static const int v_size = 20;

struct MiniCarpOptions {
  uint32_t num_ranks;
  std::string mount_path;
  std::string fname;
  Env* env;
};

struct KVItem {

//TODO: can be const
  // Slice& key;
  // Slice& value;

  float key;
  char value[v_size];
  
  KVItem(float k, const char* v):key(k)
  {
    memcpy(value, v, v_size);
  }

  KVItem(const KVItem& kv){
    key = kv.key;
    memcpy(value, kv.value, v_size);
  }

};

typedef moodycamel::ConcurrentQueue<KVItem> KVQueue;

class Worker {
 public:
  Worker() : shutdown_(false) {}
  void Shutdown() { shutdown_ = true; }
  virtual void Run() = 0;

 protected:
  bool shutdown_;
};

class Producer : public Worker {
 public:
  Producer(const MiniCarpOptions& options, std::vector<KVQueue>& queues,
            std::vector<float>& pivots)
      : options_(options), queues_(queues), pivots_(pivots){};

 protected:
  const MiniCarpOptions& options_;
  std::vector<KVQueue>& queues_;
  std::vector<float>& pivots_;

};

class Consumer : public Worker {
 public:
  Consumer(const MiniCarpOptions& options, KVQueue& queue)
      : options_(options), queue_(queue) {}

 protected:
  const MiniCarpOptions& options_;
  KVQueue& queue_;
};

class CarpReceiver : public Consumer {
 public:
  CarpReceiver(const MiniCarpOptions& options, KVQueue& queue)
      : Consumer(options, queue) {
    logf(LOG_INFO, "Init CarpDB instance here");
  }

  ~CarpReceiver() { logf(LOG_INFO, "Close CarpDB"); }

  virtual void Run() override {
    logf(LOG_DBUG, "CarpDB receiver");
    while (!shutdown_) {
      // pull from queue and insert
    }
  }
};

}  // namespace carp
}  // namespace pdlfs