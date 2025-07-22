#include "websocketClient.hpp"
#include <iostream>

WebSocketClient::WebSocketClient() : isConnected_(false), shouldStop_(false) {
    // Set up SSL context
    ctx_.set_verify_mode(ssl::verify_none);
}

WebSocketClient::~WebSocketClient() {
    close();
}

void WebSocketClient::connect(const std::string& host, const std::string& port, const std::string& path) {
    try {
        std::cout << "Initializing WS" << std::endl;
        ws_ = std::make_unique<websocket::stream<ssl::stream<tcp::socket>>>(ioc_, ctx_);
        std::cout << "Resolving hostname..." << std::endl;
        tcp::resolver resolver(ioc_);
        auto const results = resolver.resolve(host, port);
        std::cout << "Connecting to server..." << std::endl;
        auto ep = net::connect(beast::get_lowest_layer(*ws_), results);
        std::cout << "Setting up SSL..." << std::endl;
        if (!SSL_set_tlsext_host_name(ws_->next_layer().native_handle(), host.c_str())) {
            beast::error_code ec{static_cast<int>(::ERR_get_error()), net::error::get_ssl_category()};
            throw beast::system_error{ec};
        }
        std::cout << "Performing SSL handshake..." << std::endl;
        ws_->next_layer().handshake(ssl::stream_base::client);
        ws_->set_option(websocket::stream_base::decorator(
            [](websocket::request_type& req) {
                req.set(beast::http::field::user_agent,
                    std::string(BOOST_BEAST_VERSION_STRING) +
                    " websocket-client-coro");
            }));

        std::cout << "Performing WebSocket handshake..." << std::endl;
        ws_->handshake(host, path);

        isConnected_ = true;
        if (connectionHandler_) {
            connectionHandler_();
        }

        // Start the read loop in a separate thread
        shouldStop_ = false;
        io_thread_ = std::thread([this]() { readLoop(); });

    } catch (const beast::system_error& se) {
        std::cerr << "Beast error: " << se.what() << " (code: " << se.code() << ")" << std::endl;
        isConnected_ = false;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        isConnected_ = false;
    }
}

void WebSocketClient::setMessageHandler(MessageHandler handler) {
    messageHandler_ = std::move(handler);
}

void WebSocketClient::setConnectionHandler(ConnectionHandler handler) {
    connectionHandler_ = std::move(handler);
}

void WebSocketClient::close() {
    if (isConnected_) {
        shouldStop_ = true;
        if (ws_) {
            try {
                ws_->close(websocket::close_code::normal);
            } catch (const std::exception& e) {
                std::cerr << "Error closing connection: " << e.what() << std::endl;
            }
        }
        if (io_thread_.joinable()) {
            io_thread_.join();
        }
        isConnected_ = false;
    }
}

void WebSocketClient::readLoop() {
    try {
        while (!shouldStop_ && isConnected_) {
            beast::flat_buffer buffer;
            ws_->read(buffer);
            
            if (messageHandler_) {
                std::string message = beast::buffers_to_string(buffer.data());
                messageHandler_(message);
            }
        }
    } catch (const beast::system_error& se) {
        if (se.code() != websocket::error::closed) {
            handleError(se.code(), "read");
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in read loop: " << e.what() << std::endl;
    }
    isConnected_ = false;
}

void WebSocketClient::handleError(const beast::error_code& ec, const char* what) {
    std::cerr << what << ": " << ec.message() << " (code: " << ec << ")" << std::endl;
    isConnected_ = false;
}
