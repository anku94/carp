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
	options.fname = "/tmp/RDB-00000000.tbl"; //TODO let the producer pick it
	options.num_ranks = 5;
	options.env = Env::Default();
  MiniCarp mini_carp(options);
  mini_carp.Run();
}

// TEST(MiniCarpTest, ReceiverTest) {
// 	// MiniCarpOptions options;
//   // CarpReceiver r(options);
//   r.Run();
// }
}  // namespace carp
}  // namespace pdlfs

int main(int argc, char* argv[]) {
  return pdlfs::test::RunAllTests(&argc, &argv);
}