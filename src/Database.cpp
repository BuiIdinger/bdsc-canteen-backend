#include "Database.h"
#include <cassandra.h>
#include <memory>
#include <fstream>
#include <ios>
#include <sstream>
#include "Log.h"

/**
 * Used for SSL certificates when connecting
 */
static std::stringstream openCertificate(const std::string& path) {
  std::ifstream certificateStream(path, std::ios::in);
  if (!certificateStream.is_open()) {
    Log::critical("Failed to open file:: " + path);
    UNREACHABLE;
  }

  std::stringstream file;
  file << certificateStream.rdbuf();
  return std::move(file);
}

/**
 * Local cluster functions
 */
void Database::LocalCluster::setupOptions() {
  // Set timeouts
  cass_cluster_set_connect_timeout(cluster.get(), connectTimeout);
  cass_cluster_set_request_timeout(cluster.get(), requestTimeout);
  cass_cluster_set_resolve_timeout(cluster.get(), dnsResolveTimeout);
  cass_cluster_set_connection_idle_timeout(cluster.get(), connectionIdleTimeout);
  cass_cluster_set_constant_reconnect(cluster.get(), reconnectionAttemptTimeout);

  // Other options
  cass_cluster_set_load_balance_round_robin(cluster.get());
  cass_cluster_set_retry_policy(cluster.get(), retryPolicy.get());

  const CassError setPortError = cass_cluster_set_port(cluster.get(), port);
  if (setPortError != CASS_OK) {
    Log::Database::critical("Failed to set port: " + std::string(cass_error_desc(setPortError)));
    UNREACHABLE;
  }

  const CassError setHostsError = cass_cluster_set_contact_points(cluster.get(), contactPoints);
  if (setHostsError != CASS_OK) {
    Log::Database::critical("Failed to set hosts: " + std::string(cass_error_desc(setHostsError)));
    UNREACHABLE;
  }
}

/**
 * Remote cluster functions
 */
void Database::RemoteCluster::setupOptions() {
  // Set timeouts
  cass_cluster_set_connect_timeout(cluster.get(), connectTimeout);
  cass_cluster_set_request_timeout(cluster.get(), requestTimeout);
  cass_cluster_set_connection_idle_timeout(cluster.get(), connectionIdleTimeout);
  cass_cluster_set_constant_reconnect(cluster.get(), reconnectionAttemptTimeout);

  // Other options
  cass_cluster_set_load_balance_round_robin(cluster.get());
  cass_cluster_set_retry_policy(cluster.get(), retryPolicy.get());
  cass_cluster_set_credentials(cluster.get(), username, password);

  const CassError setPortError = cass_cluster_set_port(cluster.get(), port);
  if (setPortError != CASS_OK) {
    Log::Database::critical("Failed to set port: " + std::string(cass_error_desc(setPortError)));
    UNREACHABLE;
  }

  const CassError setHostsError = cass_cluster_set_contact_points(cluster.get(), contactPoints);
  if (setHostsError != CASS_OK) {
    Log::Database::critical("failed to set hosts: " + std::string(cass_error_desc(setHostsError)));
    UNREACHABLE;
  }
}

void Database::setupSSL() {
  const std::stringstream publicCert = openCertificate("data/database-cert.pem");
  const std::stringstream privateKey = openCertificate("data/database-key.key");
  const std::stringstream rootca = openCertificate("data/database-rootca.pem");

  const CassError addPublicCertError = cass_ssl_set_cert(Database::ssl.get(), publicCert.str().c_str());
  if (addPublicCertError != CASS_OK) {
    Log::Database::critical("Failed to set public key: " + std::string(cass_error_desc(addPublicCertError)));
    UNREACHABLE;
  }

  const CassError addPrivateKeyError = cass_ssl_set_private_key(ssl.get(), privateKey.str().c_str(), nullptr);
  if (addPrivateKeyError != CASS_OK) {
    Log::Database::critical("Failed to set private key: " + std::string(cass_error_desc(addPrivateKeyError)));
    UNREACHABLE;
  }

  const CassError addRootcaError = cass_ssl_add_trusted_cert(ssl.get(), rootca.str().c_str());
  if (addRootcaError != CASS_OK) {
    Log::Database::critical("Failed to set RootCA: " + std::string(cass_error_desc(addRootcaError)));
    UNREACHABLE;
  }

  cass_cluster_set_ssl(cluster.get(), ssl.get());
}

void Database::connect() {
  Log::Database::info("Connecting to database");

#ifdef DB_CLUSTER_LOCAL
  LocalCluster::setupOptions();
#endif

#ifdef DB_CLUSTER_REMOTE
  RemoteCluster::setupOptions();
  setupSSL();
#endif

  const std::unique_ptr<CassFuture, void(*)(CassFuture*)> connectKeyspace(cass_session_connect_keyspace(Database::connection.get(), Database::cluster.get(), "bc"), cass_future_free);
  if (cass_future_error_code(connectKeyspace.get()) != CASS_OK) {
    const char* errorMessage;
    size_t errorMessageSize;
    cass_future_error_message(connectKeyspace.get(), &errorMessage, &errorMessageSize);
    Log::Database::critical("Failed to connect to cluster: " + std::string(errorMessage, errorMessageSize));
    UNREACHABLE;
  }

  Log::Database::info("Connected to database");
}

/**
 * Disconnect functions
 */
void Database::disconnect() {
  disconnect(connection.get());
}

void Database::disconnect(CassSession* connection) {
  if (!connection) {
    return;
  }

  const std::unique_ptr<CassFuture, void(*)(CassFuture*)> closeFuture(cass_session_close(connection), cass_future_free);

  // Don't think we need a callback for this, doesn't really matter anyway
  cass_future_wait(closeFuture.get());
  if (cass_future_error_code(closeFuture.get()) != CASS_OK) {
    const char* message;
    size_t messageSize;
    cass_future_error_message(closeFuture.get(), &message, &messageSize);
  }

  cass_session_free(connection);
  connection = nullptr;
}

/**
 * Statement preparing
 */
void Database::prepareStatement(std::unique_ptr<const CassPrepared, void(*)(const CassPrepared*)>& statement, const std::string& query) {
  CassFuture* future = cass_session_prepare(connection.get(), query.c_str());

  // Define callback
  auto callback = [](CassFuture* future, void* data) {
    auto& statementCasted = *static_cast<std::unique_ptr<const CassPrepared, void(*)(const CassPrepared*)>*>(data);

    if (cass_future_error_code(future) != CASS_OK) {
      const char* errorMessage;
      size_t errorMessageSize;
      cass_future_error_message(future, &errorMessage, &errorMessageSize);
      cass_future_free(future);
      Log::Database::critical("Failed to prepare statement: " + std::string(errorMessage, errorMessageSize));
      UNREACHABLE;
    }

    statementCasted.reset(cass_future_get_prepared(future));
  };

  // Set the callback
  const CassError setCallbackError = cass_future_set_callback(future, callback, &statement);
  if (setCallbackError != CASS_OK) {
    const char* errorMessage;
    size_t errorMessageSize;
    cass_future_error_message(future, &errorMessage, &errorMessageSize);
    cass_future_free(future);
    Log::Database::critical("Failed to set callback: " + std::string(errorMessage, errorMessageSize));
    UNREACHABLE;
  }
}

  return std::unique_ptr<const CassPrepared, void(*)(const CassPrepared*)>(prepared, cass_prepared_free);
  cass_prepared_free(prepared); */
}