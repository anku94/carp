#pragma once

#include "common.h"
#include "concurrentqueue.h"
#include "minicarp_types.h"

#include <pdlfs-common/env.h>

namespace pdlfs {
namespace carp {

// static?
// static - for functions -

class MiniCarp;
class WholeFileReader : public Producer {
 public:
  WholeFileReader(const MiniCarpOptions& options, std::vector<KVQueue>& queues,
                  MiniCarp* mini_carp)
      : Producer(options, queues), mini_carp_(mini_carp) {}

  virtual void Run() override;

 private:
  Status OpenFileHandle(SequentialFile** fh, uint64_t* fsz);

  int ComputeShuffleTarget(KVItem& kv);

  std::vector<float> pivots_;

  MiniCarp* const mini_carp_;
};
}  // namespace carp
}  // namespace pdlfs