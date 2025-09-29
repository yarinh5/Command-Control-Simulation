#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <thread>
#include <string>
#include <functional>
#include <mutex>
#include <atomic>
#include "protocol_handler.h"

namespace ccsim::net {

  class TcpClient {
  public:
    using MessageHandler = std::function<void(const std::string&)>;
    using ConnectionHandler = std::function<void(bool)>;
    
    explicit TcpClient(const std::string& server_address, unsigned short port);
    ~TcpClient();
    
    // Connection control
    bool connect();
    void disconnect();
    bool is_connected() const;
    
    // Message handling
    void set_message_handler(MessageHandler handler);
    void set_connection_handler(ConnectionHandler handler);
    
    // Send messages
    bool send_message(const std::string& message);
    
  private:
    class ClientImpl;
    using ClientImplPtr = std::unique_ptr<ClientImpl>;
    
    ClientImplPtr impl_;
    std::string server_address_;
    unsigned short port_;
    std::string client_id_;
  };

  // Internal client implementation
  class TcpClient::ClientImpl {
  public:
    ClientImpl(const std::string& server_address, unsigned short port, const std::string& client_id);
    ~ClientImpl();
    
    bool connect();
    void disconnect();
    bool is_connected() const;
    
    void set_message_handler(MessageHandler handler);
    void set_connection_handler(ConnectionHandler handler);
    
    bool send_message(const std::string& message);
    
  private:
    void read_header();
    void read_body(size_t body_length);
    void handle_connect(const boost::system::error_code& error);
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_write(const boost::system::error_code& error, size_t bytes_transferred);
    
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::socket socket_;
    boost::asio::ip::tcp::resolver resolver_;
    
    std::string server_address_;
    unsigned short port_;
    std::string client_id_;
    
    std::atomic<bool> connected_{false};
    std::unique_ptr<std::thread> io_thread_;
    
    MessageHandler message_handler_;
    ConnectionHandler connection_handler_;
    
    // Message buffer
    std::array<char, 4> header_buffer_;
    std::vector<char> body_buffer_;
    
    std::mutex send_mutex_;
  };

} // namespace ccsim::net
