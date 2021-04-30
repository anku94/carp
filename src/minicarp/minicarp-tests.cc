//
// Created by Ankush J on 4/29/21.
//

#include "pdlfs-common/testharness.h"
#include "pdlfs-common/testutil.h"

namespace pdlfs {
namespace carp {
class MiniCarpTest {
 public:
};

TEST(MiniCarpTest, HelloWorld) { printf("hello world!\n"); }
}  // namespace carp
}  // namespace pdlfs

int main(int argc, char* argv[]) {
  return pdlfs::test::RunAllTests(&argc, &argv);
}