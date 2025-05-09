#include "Database.h"
#include <cassandra.h>
#include <memory>
#include <fstream>
#include <ios>
#include <iostream>
#include <sstream>

static std::stringstream openCertificate(const std::string path) {
  std::ifstream certificateStream(path, std::ios::in);
  if (!certificateStream.is_open() || !certificateStream.good()) {
    std::cerr << "Failed to open " << path << "\n";
    exit(EXIT_FAILURE);
  }

  std::stringstream file;
  file << certificateStream.rdbuf();
  return std::move(file);
}

void Database::connect() {
  // Use SSL
  std::stringstream publicCert = openCertificate("data/database-cert.pem");
  std::stringstream privateKey = openCertificate("data/database-key.key");
  std::stringstream rootca = openCertificate("data/database-rootca.pem");

  CassError addPublicCertError = cass_ssl_set_cert(Database::ssl.get(), publicCert.str().c_str());
  if (addPublicCertError != CASS_OK) {
    std::cerr << "Failed to set public key: " << cass_error_desc(addPublicCertError) << std::endl;
    exit(EXIT_FAILURE);
  }

  CassError addPrivateKeyError = cass_ssl_set_private_key(Database::ssl.get(), privateKey.str().c_str(), nullptr);
  if (addPrivateKeyError != CASS_OK) {
    std::cerr << "Failed to set private key: " << cass_error_desc(addPrivateKeyError) << std::endl;
    exit(EXIT_FAILURE);
  }

  CassError addRootcaError = cass_ssl_add_trusted_cert(Database::ssl.get(), rootca.str().c_str());
  if (addRootcaError != CASS_OK) {
    std::cerr << "Failed to set rootca key: " << cass_error_desc(addRootcaError) << std::endl;
    exit(EXIT_FAILURE);
  }

  // Set options now
  cass_cluster_set_ssl(Database::cluster.get(), Database::ssl.get());
  cass_cluster_set_credentials(Database::cluster.get(), "username", "password");
  cass_cluster_set_connect_timeout(Database::cluster.get(), 10000);
  cass_cluster_set_request_timeout(Database::cluster.get(), 5000);
  cass_cluster_set_connection_idle_timeout(Database::cluster.get(), 60);
  cass_cluster_set_load_balance_round_robin(Database::cluster.get());
  cass_cluster_set_constant_reconnect(Database::cluster.get(), 3000);
  cass_cluster_set_retry_policy(Database::cluster.get(), retryPolicy.get());
  cass_cluster_set_port(Database::cluster.get(), 6969);

  CassError setHostsError = cass_cluster_set_contact_points(Database::cluster.get(), "db.buildinger.org");
  if (setHostsError != CASS_OK) {
    std::cerr << "Failed to set hosts: " << cass_error_desc(setHostsError) << std::endl;
    exit(EXIT_FAILURE);
  }
  
  // Connect
  std::unique_ptr<CassFuture, void(*)(CassFuture*)> connectKeyspace(cass_session_connect_keyspace(Database::connection.get(), Database::cluster.get(), "bc"), cass_future_free);
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

  std::unique_ptr<CassFuture, void(*)(CassFuture*)> closeFuture(cass_session_close(connection), cass_future_free);

  // Don't think we need a callback for this, doesn't really matter anyways
  cass_future_wait(closeFuture.get());
  if (cass_future_error_code(closeFuture.get()) != CASS_OK) {
    const char* message;
    size_t messageSize;
    cass_future_error_message(closeFuture.get(), &message, &messageSize);
    std::cerr << "Failed to disconnect from database: " << std::string(message, messageSize) << std::endl << "\n";
  }

  cass_session_free(connection);
}