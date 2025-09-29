#include "tcp_client.h"
#include "core/logger.h"
#include <boost/endian/conversion.hpp>
#include <random>

namespace ccsim::net {

  // TcpClient implementation
  TcpClient::TcpClient(const std::string& server_address, unsigned short port)
    : server_address_(server_address), port_(port) {
    
    // Generate unique client ID
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1000, 9999);
    client_id_ = "agent_" + std::to_string(dis(gen));
    
    impl_ = std::make_unique<ClientImpl>(server_address, port, client_id_);
  }

  TcpClient::~TcpClient() {
    disconnect();
  }

  bool TcpClient::connect() {
    return impl_->connect();
  }

  void TcpClient::disconnect() {
    impl_->disconnect();
  }

  bool TcpClient::is_connected() const {
    return impl_->is_connected();
  }

  void TcpClient::set_message_handler(MessageHandler handler) {
    impl_->set_message_handler(std::move(handler));
  }

  void TcpClient::set_connection_handler(ConnectionHandler handler) {
    impl_->set_connection_handler(std::move(handler));
  }

  bool TcpClient::send_message(const std::string& message) {
    return impl_->send_message(message);
  }

  // ClientImpl implementation
  TcpClient::ClientImpl::ClientImpl(const std::string& server_address, unsigned short port, const std::string& client_id)
    : socket_(io_context_), resolver_(io_context_), server_address_(server_address), port_(port), client_id_(client_id) {}

  TcpClient::ClientImpl::~ClientImpl() {
    disconnect();
  }

  bool TcpClient::ClientImpl::connect() {
    if (connected_) {
      return true;
    }
    
    auto& logger = core::Logger::instance();
    logger.info("Connecting to server: " + server_address_ + ":" + std::to_string(port_));
    
    try {
      auto endpoints = resolver_.resolve(server_address_, std::to_string(port_));
      
      boost::asio::async_connect(socket_, endpoints,
        [this](const boost::system::error_code& error, const boost::asio::ip::tcp::endpoint&) {
          handle_connect(error);
        });
      
      // Start IO thread
      io_thread_ = std::make_unique<std::thread>([this]() {
        io_context_.run();
      });
      
      // Wait for connection (with timeout)
      auto start = std::chrono::steady_clock::now();
      while (!connected_ && (std::chrono::steady_clock::now() - start) < std::chrono::seconds(5)) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
      }
      
      if (connected_) {
        logger.info("Connected to server successfully");
        return true;
      } else {
        logger.error("Failed to connect to server");
        return false;
      }
    } catch (const std::exception& e) {
      logger.error("Connection error: " + std::string(e.what()));
      return false;
    }
  }

  void TcpClient::ClientImpl::disconnect() {
    if (connected_.exchange(false)) {
      auto& logger = core::Logger::instance();
      logger.info("Disconnecting from server");
      
      boost::system::error_code ec;
      socket_.close(ec);
      
      io_context_.stop();
      
      if (io_thread_ && io_thread_->joinable()) {
        io_thread_->join();
      }
      
      if (connection_handler_) {
        connection_handler_(false);
      }
    }
  }

  bool TcpClient::ClientImpl::is_connected() const {
    return connected_;
  }

  void TcpClient::ClientImpl::set_message_handler(MessageHandler handler) {
    message_handler_ = std::move(handler);
  }

  void TcpClient::ClientImpl::set_connection_handler(ConnectionHandler handler) {
    connection_handler_ = std::move(handler);
  }

  bool TcpClient::ClientImpl::send_message(const std::string& message) {
    if (!connected_) {
      return false;
    }
    
    std::lock_guard<std::mutex> lock(send_mutex_);
    
    // Prepare message with length header
    uint32_t length = static_cast<uint32_t>(message.length());
    uint32_t network_length = boost::endian::native_to_big(length);
    
    std::vector<boost::asio::const_buffer> buffers;
    buffers.push_back(boost::asio::buffer(&network_length, 4));
    buffers.push_back(boost::asio::buffer(message));
    
    try {
      boost::asio::write(socket_, buffers);
      return true;
    } catch (const std::exception&) {
      disconnect();
      return false;
    }
  }

  void TcpClient::ClientImpl::read_header() {
    boost::asio::async_read(socket_, boost::asio::buffer(header_buffer_),
      [this](const boost::system::error_code& error, size_t bytes_transferred) {
        if (!error) {
          uint32_t network_length;
          std::memcpy(&network_length, header_buffer_.data(), 4);
          uint32_t body_length = boost::endian::big_to_native(network_length);
          
          if (body_length > 0 && body_length < 1024 * 1024) { // 1MB limit
            read_body(body_length);
          } else {
            disconnect();
          }
        } else {
          disconnect();
        }
      });
  }

  void TcpClient::ClientImpl::read_body(size_t body_length) {
    body_buffer_.resize(body_length);
    boost::asio::async_read(socket_, boost::asio::buffer(body_buffer_),
      [this](const boost::system::error_code& error, size_t bytes_transferred) {
        handle_read(error, bytes_transferred);
      });
  }

  void TcpClient::ClientImpl::handle_connect(const boost::system::error_code& error) {
    if (!error) {
      connected_ = true;
      
      if (connection_handler_) {
        connection_handler_(true);
      }
      
      // Start reading messages
      read_header();
    } else {
      auto& logger = core::Logger::instance();
      logger.error("Connection failed: " + error.message());
    }
  }

  void TcpClient::ClientImpl::handle_read(const boost::system::error_code& error, size_t bytes_transferred) {
    if (!error && connected_) {
      std::string message(body_buffer_.data(), bytes_transferred);
      
      // Notify message handler
      if (message_handler_) {
        message_handler_(message);
      }
      
      // Continue reading
      read_header();
    } else {
      disconnect();
    }
  }

  void TcpClient::ClientImpl::handle_write(const boost::system::error_code& error, size_t bytes_transferred) {
    if (error) {
      disconnect();
    }
  }

} // namespace ccsim::net
