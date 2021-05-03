

#pragma once

#include "common.h"
#include "minicarp_types.h"
#include "concurrentqueue.h"

#include <pdlfs-common/env.h>


namespace pdlfs{
namespace carp{


class WholeFileReader : public Producer {
 public:
  WholeFileReader(const MiniCarpOptions& options, std::vector<KVQueue>& queues,
                  std::vector<float>& pivots)
      : Producer(options, queues, pivots) {}

  virtual void Run() override;

  Status WholeFileReader::OpenFileHandle(SequentialFile** fh, uint64_t *fsz);

  int WholeFileReader::ComputeShuffleTarget(KVItem& kv);
};
}
}