#include "client.hpp"
#include <iostream>
#include <thread>
#include <boost/asio/connect.hpp>

WebSocketClient::WebSocketClient(net::io_context& ioc, ssl::context& ctx)
    : ioc_(ioc)
    , ctx_(ctx)
    , resolver_(net::make_strand(ioc))
    , ws_(net::make_strand(ioc), ctx_)
{
}

WebSocketClient::~WebSocketClient() {
    close();
}

void WebSocketClient::connect(const std::string& host, const std::string& port, const std::string& path) {
    host_ = host;
    path_ = path;
    
    auto self = shared_from_this();
    
    resolver_.async_resolve(
        host,
        port,
        beast::bind_front_handler(
            &WebSocketClient::on_resolve,
            self
        )
    );
}

void WebSocketClient::send(const std::vector<uint8_t>& message) {
    auto self = shared_from_this();
    
    ws_.async_write(
        net::buffer(message),
        beast::bind_front_handler(
            &WebSocketClient::on_write,
            self
        )
    );
}

void WebSocketClient::close() {
    if (ws_.is_open()) {
        auto self = shared_from_this();
        
        ws_.async_close(
            websocket::close_code::normal,
            beast::bind_front_handler(
                &WebSocketClient::on_close,
                self
            )
        );
    }
}

void WebSocketClient::set_message_handler(std::function<void(const std::string&)> handler) {
    message_handler_ = handler;
}

void WebSocketClient::on_resolve(beast::error_code ec, tcp::resolver::results_type results) {
    if (ec) {
        std::cerr << "Resolve error: " << ec.message() << std::endl;
        return;
    }
    
    auto self = shared_from_this();
    
    // Используем правильный метод для подключения к результатам резолвера
    net::async_connect(
        beast::get_lowest_layer(ws_),
        results,
        beast::bind_front_handler(
            &WebSocketClient::on_connect,
            self
        )
    );
}

void WebSocketClient::on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep) {
    if (ec) {
        std::cerr << "Connect error: " << ec.message() << std::endl;
        return;
    }
    
    if (!SSL_set_tlsext_host_name(ws_.next_layer().native_handle(), host_.c_str())) {
        ec = beast::error_code(static_cast<int>(::ERR_get_error()), net::error::get_ssl_category());
        std::cerr << "SSL set host error: " << ec.message() << std::endl;
        return;
    }
    
    auto self = shared_from_this();
    
    ws_.next_layer().async_handshake(
        ssl::stream_base::client,
        beast::bind_front_handler(
            &WebSocketClient::on_ssl_handshake,
            self
        )
    );
}

void WebSocketClient::on_ssl_handshake(beast::error_code ec) {
    if (ec) {
        std::cerr << "SSL handshake error: " << ec.message() << std::endl;
        return;
    }
    
    auto self = shared_from_this();
    
    ws_.async_handshake(host_, path_,
        beast::bind_front_handler(
            &WebSocketClient::on_handshake,
            self
        )
    );
}

void WebSocketClient::on_handshake(beast::error_code ec) {
    if (ec) {
        std::cerr << "WebSocket handshake error: " << ec.message() << std::endl;
        return;
    }
    
    std::cout << "WebSocket connected successfully!" << std::endl;
    
    auto self = shared_from_this();
    
    ws_.async_read(
        buffer_,
        beast::bind_front_handler(
            &WebSocketClient::on_read,
            self
        )
    );
}

void WebSocketClient::on_write(beast::error_code ec, std::size_t bytes_transferred) {
    if (ec) {
        std::cerr << "Write error: " << ec.message() << std::endl;
        return;
    }
}

void WebSocketClient::on_read(beast::error_code ec, std::size_t bytes_transferred) {
    if (ec) {
        std::cerr << "Read error: " << ec.message() << std::endl;
        return;
    }
    
    if (message_handler_) {
        std::string message = beast::buffers_to_string(buffer_.data());
        message_handler_(message);
    }
    
    buffer_.consume(buffer_.size());
    
    auto self = shared_from_this();
    
    ws_.async_read(
        buffer_,
        beast::bind_front_handler(
            &WebSocketClient::on_read,
            self
        )
    );
}

void WebSocketClient::on_close(beast::error_code ec) {
    if (ec) {
        std::cerr << "Close error: " << ec.message() << std::endl;
    }
}