#include "WebSocketClient.hpp"
#include <iostream>
#include <thread>
#include <boost/asio/connect.hpp>

WebSocketClient::WebSocketClient(net::io_context& ioc, ssl::context& ctx)
    : _ioc(ioc)
    , _ctx(ctx)
    , _resolver(net::make_strand(ioc))
    , _ws(net::make_strand(ioc), _ctx)
{
}

WebSocketClient::~WebSocketClient()
{
    close();
}

void WebSocketClient::connect(const std::string& host, const std::string& port, const std::string& path)
{
    _host = host;
    _path = path;
    
    auto self = shared_from_this();
    
    _resolver.async_resolve(
        host,
        port,
        beast::bind_front_handler(
            &WebSocketClient::onResolve,
            self
        )
    );
}

void WebSocketClient::send(const std::vector<uint8_t>& message)
{
    auto self = shared_from_this();
    
    _ws.async_write(
        net::buffer(message),
        beast::bind_front_handler(
            &WebSocketClient::onWrite,
            self
        )
    );
}

void WebSocketClient::close()
{
    if (_ws.is_open()) {
        auto self = shared_from_this();
        
        _ws.async_close(
            websocket::close_code::normal,
            beast::bind_front_handler(
                &WebSocketClient::onClose,
                self
            )
        );
    }
}

void WebSocketClient::setMessageHandler(std::function<void(const std::string&)> handler)
{
    _messageHandler = handler;
}

void WebSocketClient::onResolve(beast::error_code ec, tcp::resolver::results_type results)
{
    if (ec) {
        std::cerr << "Resolve error: " << ec.message() << std::endl;
        return;
    }
    
    auto self = shared_from_this();
    
    // Используем правильный метод для подключения к результатам резолвера
    net::async_connect(
        beast::get_lowest_layer(_ws),
        results,
        beast::bind_front_handler(
            &WebSocketClient::onConnect,
            self
        )
    );
}

void WebSocketClient::onConnect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep)
{
    if (ec) {
        std::cerr << "Connect error: " << ec.message() << std::endl;
        return;
    }
    
    if (!SSL_set_tlsext_host_name(_ws.next_layer().native_handle(), _host.c_str())) {
        ec = beast::error_code(static_cast<int>(::ERR_get_error()), net::error::get_ssl_category());
        std::cerr << "SSL set host error: " << ec.message() << std::endl;
        return;
    }
    
    auto self = shared_from_this();
    
    _ws.next_layer().async_handshake(
        ssl::stream_base::client,
        beast::bind_front_handler(
            &WebSocketClient::onSslHandshake,
            self
        )
    );
}

void WebSocketClient::onSslHandshake(beast::error_code ec)
{
    if (ec) {
        std::cerr << "SSL handshake error: " << ec.message() << std::endl;
        return;
    }
    
    auto self = shared_from_this();
    
    _ws.async_handshake(_host, _path,
        beast::bind_front_handler(
            &WebSocketClient::onHandshake,
            self
        )
    );
}

void WebSocketClient::onHandshake(beast::error_code ec)
{
    if (ec) {
        std::cerr << "WebSocket handshake error: " << ec.message() << std::endl;
        return;
    }
    
    std::cout << "WebSocket connected successfully!" << std::endl;
    
    auto self = shared_from_this();
    
    _ws.async_read(
        _buffer,
        beast::bind_front_handler(
            &WebSocketClient::onRead,
            self
        )
    );
}

void WebSocketClient::onWrite(beast::error_code ec, std::size_t bytesTransferred)
{
    if (ec) {
        std::cerr << "Write error: " << ec.message() << std::endl;
        return;
    }
}

void WebSocketClient::onRead(beast::error_code ec, std::size_t bytesTransferred)
{
    if (ec) {
        std::cerr << "Read error: " << ec.message() << std::endl;
        return;
    }
    
    if (_messageHandler) {
        std::string message = beast::buffers_to_string(_buffer.data());
        _messageHandler(message);
    }
    
    _buffer.consume(_buffer.size());
    
    auto self = shared_from_this();
    
    _ws.async_read(
        _buffer,
        beast::bind_front_handler(
            &WebSocketClient::onRead,
            self
        )
    );
}

void WebSocketClient::onClose(beast::error_code ec)
{
    if (ec) {
        std::cerr << "Close error: " << ec.message() << std::endl;
    }
}