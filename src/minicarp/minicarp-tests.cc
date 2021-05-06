//
// Created by Ankush J on 4/29/21.
//

#include "minicarp.h"

#include "pdlfs-common/testharness.h"
#include "pdlfs-common/testutil.h"

namespace pdlfs {
namespace carp {
class MiniCarpTest {
 public:
  void Hello() {
    printf("hello world!\n");
  }
};

TEST(MiniCarpTest, HelloWorld) {
  Hello();
  MiniCarpOptions options;
  MiniCarp mini_carp(options);
  mini_carp.Run();
}

TEST(MiniCarpTest, ReceiverTest) {
  CarpReceiver r(options);
  r.Run();
}
}  // namespace carp
}  // namespace pdlfs

int main(int argc, char* argv[]) {
  return pdlfs::test::RunAllTests(&argc, &argv);
}