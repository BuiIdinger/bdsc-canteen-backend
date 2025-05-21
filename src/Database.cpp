#include "Database.h"
#include <cassandra.h>
#include <memory>
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>
#include "Main.h"

static std::stringstream openCertificate(const std::string& path) {
  std::ifstream certificateStream(path, std::ios::in);
  if (!certificateStream.is_open()) {
    std::cerr << "Failed to open " << path << "\n";
    shutdown(EXIT_FAILURE);
  }

  std::stringstream file;
  file << certificateStream.rdbuf();
  return std::move(file);
}

void Database::setupSSL() {
  const std::stringstream publicCert = openCertificate("data/database-cert.pem");
  const std::stringstream privateKey = openCertificate("data/database-key.key");
  const std::stringstream rootca = openCertificate("data/database-rootca.pem");

  const CassError addPublicCertError = cass_ssl_set_cert(Database::ssl.get(), publicCert.str().c_str());
  if (addPublicCertError != CASS_OK) {
    std::cerr << "Failed to set public key: " << cass_error_desc(addPublicCertError) << std::endl;
    shutdown(EXIT_FAILURE);
  }

  const CassError addPrivateKeyError = cass_ssl_set_private_key(ssl.get(), privateKey.str().c_str(), nullptr);
  if (addPrivateKeyError != CASS_OK) {
    std::cerr << "Failed to set private key: " << cass_error_desc(addPrivateKeyError) << std::endl;
    shutdown(EXIT_FAILURE);
  }

  const CassError addRootcaError = cass_ssl_add_trusted_cert(ssl.get(), rootca.str().c_str());
  if (addRootcaError != CASS_OK) {
    std::cerr << "Failed to set rootca key: " << cass_error_desc(addRootcaError) << std::endl;
    shutdown(EXIT_FAILURE);
  }

  cass_cluster_set_ssl(cluster.get(), ssl.get());
}

void Database::configureOptions() {
  cass_cluster_set_credentials(cluster.get(), username, password);
  cass_cluster_set_connect_timeout(cluster.get(), 10000);
  cass_cluster_set_request_timeout(cluster.get(), 5000);
  cass_cluster_set_connection_idle_timeout(cluster.get(), 60);
  cass_cluster_set_load_balance_round_robin(cluster.get());
  cass_cluster_set_constant_reconnect(cluster.get(), 3000);
  cass_cluster_set_retry_policy(cluster.get(), retryPolicy.get());
  cass_cluster_set_port(cluster.get(), port);

  const CassError setHostsError = cass_cluster_set_contact_points(cluster.get(), contactPoints);
  if (setHostsError != CASS_OK) {
    std::cerr << "Failed to set hosts: " << cass_error_desc(setHostsError) << std::endl;
    exit(EXIT_FAILURE);
  }
}

void Database::connect() {
  setupSSL();
  configureOptions();

  const std::unique_ptr<CassFuture, void(*)(CassFuture*)> connectKeyspace(cass_session_connect_keyspace(Database::connection.get(), Database::cluster.get(), "bc"), cass_future_free);
  if (cass_future_error_code(connectKeyspace.get()) != CASS_OK) {
    const char* errorMessage;
    size_t errorMessageSize;
    cass_future_error_message(connectKeyspace.get(), &errorMessage, &errorMessageSize);
    std::cerr << "Failed to connect: " << std::string(errorMessage, errorMessageSize) << std::endl;
    exit(EXIT_FAILURE);
  }
}

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
    std::cerr << "Failed to disconnect from database: " << std::string(message, messageSize) << std::endl;
  }

  cass_session_free(connection);
  connection = nullptr;
}