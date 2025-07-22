#pragma once

#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <functional>
#include <string>
#include <memory>
#include <thread>

namespace beast = boost::beast;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

class WebSocketClient {
public:
    using MessageHandler = std::function<void(const std::string&)>;
    using ConnectionHandler = std::function<void()>;

    WebSocketClient();
    ~WebSocketClient();

    // Connect to a WebSocket server
    void connect(const std::string& host, const std::string& port, const std::string& path = "/");
    
    // Set message handler callback
    void setMessageHandler(MessageHandler handler);
    
    // Set connection handler callback
    void setConnectionHandler(ConnectionHandler handler);
    
    // Close the connection
    void close();

private:
    net::io_context ioc_;
    ssl::context ctx_{ssl::context::tlsv12};
    std::unique_ptr<websocket::stream<ssl::stream<tcp::socket>>> ws_;
    std::thread io_thread_;
    MessageHandler messageHandler_;
    ConnectionHandler connectionHandler_;
    bool isConnected_;
    bool shouldStop_;

    void run();
    void readLoop();
    void handleError(const beast::error_code& ec, const char* what);
};
