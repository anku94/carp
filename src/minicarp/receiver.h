//
// Created by Ankush J on 5/5/21.
//

#pragma once

#include <deltafs/deltafs_api.h>

namespace pdlfs {
namespace carp {
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

    deltafs_plfsdir* plfshdl =
        deltafs_plfsdir_create_handle(conf.c_str(), O_WRONLY, io_engine);
    deltafs_plfsdir_set_fixed_kv(pctx.plfshdl, 1);
    deltafs_plfsdir_force_leveldb_fmt(pctx.plfshdl, force_leveldb_fmt);
    deltafs_plfsdir_set_unordered(pctx.plfshdl, unordered);
    deltafs_plfsdir_set_side_io_buf_size(pctx.plfshdl,
                                         pctx.particle_buf_size);
    deltafs_plfsdir_set_side_filter_size(pctx.plfshdl,
                                         pctx.particle_count);
    pctx.plfsparts = deltafs_plfsdir_get_memparts(pctx.plfshdl);
    pctx.plfstp = deltafs_tp_init(pctx.bgdepth);
    deltafs_plfsdir_set_thread_pool(pctx.plfshdl, pctx.plfstp);
    pctx.plfsenv = deltafs_env_init(
        1, reinterpret_cast<void**>(const_cast<char**>(&env)));
    deltafs_plfsdir_set_env(pctx.plfshdl, pctx.plfsenv);
    deltafs_plfsdir_set_err_printer(pctx.plfshdl, &plfsdir_error_printer,
                                    NULL);
    rv = deltafs_plfsdir_open(pctx.plfshdl, path)
    deltafs_plfsdir_epoch_flush();
    deltafs_plfsdir_append();
    deltafs_plfsdir_flush();
    deltafs_plfsdir_destroy();
  }
};

};
}  // namespace carp
}  // namespace pdlfs
