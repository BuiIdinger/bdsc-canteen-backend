#pragma once

#include <cassandra.h>
#include <memory>

namespace Database {
  void connect();
  void disconnect();
  void disconnect(CassSession* connection); // Overload for connection unique_ptr

  inline std::unique_ptr<CassSession, void(*)(CassSession*)> connection(cass_session_new(), Database::disconnect);
  inline std::unique_ptr<CassCluster, void(*)(CassCluster*)> cluster(cass_cluster_new(), cass_cluster_free);
  inline std::unique_ptr<CassSsl, void(*)(CassSsl*)> ssl(cass_ssl_new(), cass_ssl_free);
  inline std::unique_ptr<CassRetryPolicy, void(*)(CassRetryPolicy*)> retryPolicy(cass_retry_policy_default_new(), cass_retry_policy_free);
} // namespace Database