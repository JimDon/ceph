// -*- mode:C++; tab-width:8; c-basic-offset:2; indent-tabs-mode:t -*-
// vim: ts=8 sw=2 smarttab

#pragma once

#include <boost/intrusive_ptr.hpp>
#include <boost/smart_ptr/intrusive_ref_counter.hpp>
#include <boost/smart_ptr/local_shared_ptr.hpp>
#include <seastar/core/future.hh>

#include "osd/osd_types.h"

template<typename T> using Ref = boost::intrusive_ptr<T>;
class OSDMap;

namespace ceph::net {
  class Messenger;
}

namespace ceph::os {
  class CyanStore;
}

class PG : public boost::intrusive_ref_counter<
  PG,
  boost::thread_unsafe_counter>
{
  using ec_profile_t = std::map<std::string,std::string>;
  using cached_map_t = boost::local_shared_ptr<OSDMap>;

public:
  PG(spg_t pgid,
     pg_shard_t pg_shard,
     pg_pool_t&& pool,
     std::string&& name,
     ec_profile_t&& ec_profile,
     cached_map_t osdmap,
     ceph::net::Messenger* msgr);

  epoch_t get_osdmap_epoch() const;
  const pg_info_t& get_info() const;
  const PastIntervals& get_past_intervals() const;
  pg_shard_t get_primary() const;
  bool is_primary() const;
  pg_shard_t get_whoami() const;

  seastar::future<> read_state(ceph::os::CyanStore* store);

private:
  void update_primary_state(const std::vector<int>& new_up,
			    int new_up_primary,
			    const std::vector<int>& new_acting,
			    int new_acting_primary);
private:
  const spg_t pgid;
  pg_shard_t whoami;
  pg_pool_t pool;
  //< pg state
  pg_info_t info;
  //< last written info, for fast info persistence
  pg_info_t last_written_info;
  PastIntervals past_intervals;
  // primary state
  using pg_shard_set_t = std::set<pg_shard_t>;
  pg_shard_t primary, up_primary;
  std::vector<int> acting, up;
  pg_shard_set_t actingset, upset;

  cached_map_t osdmap;
  ceph::net::Messenger* msgr = nullptr;
};
