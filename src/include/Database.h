#pragma once

#include <cassandra.h>
#include <memory>

#define DB_CLUSTER_LOCAL

namespace Database {
  /**
   * Keyspace to connect to, this avoids having to explicit
   * set the keyspace in every query
   */
  constexpr const char* connectKeyspace = "bc";

  constexpr const char* username = "bc-h9u0XvYWuDFmFUQr4mqH3mzpM16ujP";
  constexpr const char* password = "h9u0XvYWuDFmFUQr4mqH3mzpM16ujPh9u0XvYWuDFmFUQr4mqH3mzpM16ujP";

  /**
   * Timeout options
   * All in MS time
   */
  constexpr int connectTimeout = 10000;
  constexpr int requestTimeout = 5000;
  constexpr int dnsResolveTimeout = 3000;
  constexpr int connectionIdleTimeout = 60; // In seconds not MS

  // Amount of milliseconds before a reconnection is attempted
  constexpr int reconnectionAttemptTimeout = 0;

  /**
   * Setups clusters SSL, currently only used for connecting
   * to the remote cluster
   */
  void setupSSL();

  namespace LocalCluster {
    // Network connections options
    constexpr const char* contactPoints = "127.0.0.1";
    constexpr int port = 9042;

    void setupOptions();
  }

  namespace RemoteCluster {
    // Network connections options
    constexpr const char* contactPoints = "db-prod-canteen.buildinger.org";
    constexpr int port = 9042;

    void setupOptions();
  }

  void connect();
  void disconnect();
  void disconnect(CassSession* connection); // Overload for connection unique_ptr

  /**
   * Prepares a statement to be used for a query
   */
  void prepareStatement(const std::unique_ptr<const CassPrepared, void(*)(const CassPrepared*)>& statement, const std::string& query);
  void prepareStatement(std::unique_ptr<const CassPrepared, void(*)(const CassPrepared*)>& statement, const std::string& query);

  inline std::unique_ptr<CassSession, void(*)(CassSession*)> connection(cass_session_new(), Database::disconnect);
  inline std::unique_ptr<CassCluster, void(*)(CassCluster*)> cluster(cass_cluster_new(), cass_cluster_free);
  inline std::unique_ptr<CassSsl, void(*)(CassSsl*)> ssl(cass_ssl_new(), cass_ssl_free);
  inline std::unique_ptr<CassRetryPolicy, void(*)(CassRetryPolicy*)> retryPolicy(cass_retry_policy_default_new(), cass_retry_policy_free);
} // namespace Database