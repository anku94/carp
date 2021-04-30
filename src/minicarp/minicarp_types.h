//
// Created by Ankush J on 4/29/21.
//

#pragma once

namespace pdlfs {
namespace carp {
struct MiniCarpOptions {
  uint32_t num_ranks;
  std::string mount_path;
  std::string fname;
  Env* env;
};

struct KVItem {
  Slice& key;
  Slice& value;
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
  Producer(const MiniCarpOptions& options, std::vector<KVQueue>& queues)
      : options_(options), queues_(queues){};

 private:
  const MiniCarpOptions& options_;
  std::vector<KVQueue>& queues_;
};

/* This is probably complex enough to deserve its own .h/.cc */
class WholeFileReader : public Producer {
 public:
  WholeFileReader(const MiniCarpOptions& options, std::vector<KVQueue>& queues)
      : Producer(options, queues) {}

  virtual void Run() override {
    logf(LOG_INFO,
         "Maybe get file path from CarpOptions or something, start "
         "reading/shuffling it");
  }
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