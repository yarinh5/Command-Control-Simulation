#pragma once

#include <boost/asio.hpp>
#include <memory>
#include <thread>
#include <vector>
#include <unordered_map>
#include <functional>
#include <mutex>
#include "protocol_handler.h"

namespace ccsim::net {

  class TcpServer {
  public:
    using MessageHandler = std::function<void(const std::string&, const std::string&)>;
    using ConnectionHandler = std::function<void(const std::string&)>;
    
    explicit TcpServer(unsigned short port);
    ~TcpServer();
    
    // Server control
    void start();
    void stop();
    bool is_running() const;
    
    // Message handling
    void set_message_handler(MessageHandler handler);
    void set_connection_handler(ConnectionHandler handler);
    
    // Send messages
    void send_message(const std::string& client_id, const std::string& message);
    void broadcast_message(const std::string& message);
    
    // Client management
    std::vector<std::string> get_connected_clients() const;
    bool is_client_connected(const std::string& client_id) const;
    void disconnect_client(const std::string& client_id);
    
    // Statistics
    size_t connected_clients() const;
    
  private:
    class ClientSession;
    using ClientSessionPtr = std::shared_ptr<ClientSession>;
    
    void accept_connections();
    void handle_accept(ClientSessionPtr session, const boost::system::error_code& error);
    
    mutable std::mutex clients_mutex_;
    std::unordered_map<std::string, ClientSessionPtr> clients_;
    
    boost::asio::io_context io_context_;
    boost::asio::ip::tcp::acceptor acceptor_;
    std::unique_ptr<std::thread> io_thread_;
    
    MessageHandler message_handler_;
    ConnectionHandler connection_handler_;
    
    std::atomic<bool> running_{false};
    unsigned short port_;
  };

  // Client session for handling individual connections
  class TcpServer::ClientSession : public std::enable_shared_from_this<ClientSession> {
  public:
    ClientSession(boost::asio::ip::tcp::socket socket, TcpServer* server);
    
    void start();
    void send_message(const std::string& message);
    void close();
    
    const std::string& client_id() const { return client_id_; }
    bool is_connected() const { return connected_; }
    
    void set_client_id(const std::string& id) {
      client_id_ = id;
    }
    
    boost::asio::ip::tcp::socket& socket() {
      return socket_;
    }
    
  private:
    void read_header();
    void read_body(size_t body_length);
    void handle_read(const boost::system::error_code& error, size_t bytes_transferred);
    void handle_write(const boost::system::error_code& error, size_t bytes_transferred);
    
    boost::asio::ip::tcp::socket socket_;
    TcpServer* server_;
    std::string client_id_;
    std::atomic<bool> connected_{false};
    
    // Message buffer
    std::array<char, 4> header_buffer_;
    std::vector<char> body_buffer_;
    
    std::mutex send_mutex_;
  };

} // namespace ccsim::net
