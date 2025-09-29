#include "tcp_server.h"
#include "core/logger.h"
#include <boost/endian/conversion.hpp>
#include <random>
#include <sstream>

namespace ccsim::net {

  // TcpServer implementation
  TcpServer::TcpServer(unsigned short port) 
    : port_(port), acceptor_(io_context_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)) {
    
    auto& logger = core::Logger::instance();
    logger.info("TCP Server created on port: " + std::to_string(port));
  }

  TcpServer::~TcpServer() {
    stop();
  }

  void TcpServer::start() {
    if (running_.exchange(true)) {
      return; // Already running
    }
    
    auto& logger = core::Logger::instance();
    logger.info("Starting TCP Server on port: " + std::to_string(port_));
    
    // Start IO thread
    io_thread_ = std::make_unique<std::thread>([this]() {
      io_context_.run();
    });
    
    // Start accepting connections
    accept_connections();
  }

  void TcpServer::stop() {
    if (!running_.exchange(false)) {
      return; // Already stopped
    }
    
    auto& logger = core::Logger::instance();
    logger.info("Stopping TCP Server");
    
    // Close all client connections
    {
      std::lock_guard<std::mutex> lock(clients_mutex_);
      for (auto& pair : clients_) {
        pair.second->close();
      }
      clients_.clear();
    }
    
    // Stop IO context
    io_context_.stop();
    
    // Wait for IO thread
    if (io_thread_ && io_thread_->joinable()) {
      io_thread_->join();
    }
  }

  bool TcpServer::is_running() const {
    return running_;
  }

  void TcpServer::set_message_handler(MessageHandler handler) {
    message_handler_ = std::move(handler);
  }

  void TcpServer::set_connection_handler(ConnectionHandler handler) {
    connection_handler_ = std::move(handler);
  }

  void TcpServer::send_message(const std::string& client_id, const std::string& message) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    auto it = clients_.find(client_id);
    if (it != clients_.end()) {
      it->second->send_message(message);
    }
  }

  void TcpServer::broadcast_message(const std::string& message) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    for (auto& pair : clients_) {
      pair.second->send_message(message);
    }
  }

  std::vector<std::string> TcpServer::get_connected_clients() const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    std::vector<std::string> clients;
    for (const auto& pair : clients_) {
      clients.push_back(pair.first);
    }
    return clients;
  }

  bool TcpServer::is_client_connected(const std::string& client_id) const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    return clients_.find(client_id) != clients_.end();
  }

  void TcpServer::disconnect_client(const std::string& client_id) {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    auto it = clients_.find(client_id);
    if (it != clients_.end()) {
      it->second->close();
      clients_.erase(it);
    }
  }

  size_t TcpServer::connected_clients() const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    return clients_.size();
  }

  void TcpServer::accept_connections() {
    auto session = std::make_shared<ClientSession>(
      boost::asio::ip::tcp::socket(io_context_), this);
    
    acceptor_.async_accept(session->socket(),
      [this, session](const boost::system::error_code& error) {
        handle_accept(session, error);
      });
  }

  void TcpServer::handle_accept(ClientSessionPtr session, const boost::system::error_code& error) {
    if (!error) {
      // Generate unique client ID
      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<> dis(1000, 9999);
      std::string client_id = "client_" + std::to_string(dis(gen));
      
      session->set_client_id(client_id);
      
      // Store client session
      {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        clients_[client_id] = session;
      }
      
      // Notify connection handler
      if (connection_handler_) {
        connection_handler_(client_id);
      }
      
      // Start session
      session->start();
      
      auto& logger = core::Logger::instance();
      logger.info("Client connected: " + client_id);
    }
    
    // Continue accepting connections
    if (running_) {
      accept_connections();
    }
  }

  // ClientSession implementation
  TcpServer::ClientSession::ClientSession(boost::asio::ip::tcp::socket socket, TcpServer* server)
    : socket_(std::move(socket)), server_(server) {}

  void TcpServer::ClientSession::start() {
    connected_ = true;
    read_header();
  }

  void TcpServer::ClientSession::send_message(const std::string& message) {
    if (!connected_) return;
    
    std::lock_guard<std::mutex> lock(send_mutex_);
    
    // Prepare message with length header
    uint32_t length = static_cast<uint32_t>(message.length());
    uint32_t network_length = boost::endian::native_to_big(length);
    
    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(boost::asio::buffer(&network_length, 4));
    buffers.push_back(boost::asio::buffer(message));
    
    boost::asio::async_write(socket_, buffers,
      [this](const boost::system::error_code& error, size_t bytes_transferred) {
        handle_write(error, bytes_transferred);
      });
  }

  void TcpServer::ClientSession::close() {
    if (connected_.exchange(false)) {
      boost::system::error_code ec;
      socket_.close(ec);
    }
  }

  void TcpServer::ClientSession::read_header() {
    boost::asio::async_read(socket_, boost::asio::buffer(header_buffer_),
      [this](const boost::system::error_code& error, size_t bytes_transferred) {
        if (!error) {
          uint32_t network_length;
          std::memcpy(&network_length, header_buffer_.data(), 4);
          uint32_t body_length = boost::endian::big_to_native(network_length);
          
          if (body_length > 0 && body_length < 1024 * 1024) { // 1MB limit
            read_body(body_length);
          } else {
            close();
          }
        } else {
          close();
        }
      });
  }

  void TcpServer::ClientSession::read_body(size_t body_length) {
    body_buffer_.resize(body_length);
    boost::asio::async_read(socket_, boost::asio::buffer(body_buffer_),
      [this](const boost::system::error_code& error, size_t bytes_transferred) {
        handle_read(error, bytes_transferred);
      });
  }

  void TcpServer::ClientSession::handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error && connected_) {
      std::string message(body_buffer_.data(), bytes_transferred);
      
      // Notify server of received message
      if (server_->message_handler_) {
        server_->message_handler_(client_id_, message);
      }
      
      // Continue reading
      read_header();
    } else {
      close();
    }
  }

  void TcpServer::ClientSession::handle_write(const boost::system::error_code& error, size_t bytes_transferred) {
    if (error) {
      close();
    }
  }

  void TcpServer::ClientSession::set_client_id(const std::string& id) {
    client_id_ = id;
  }

} // namespace ccsim::net
