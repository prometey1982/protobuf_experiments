#pragma once


#include <boost/beast.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <string>
#include <memory>
#include <functional>

namespace beast = boost::beast;
namespace http = beast::http;
namespace websocket = beast::websocket;
namespace net = boost::asio;
namespace ssl = boost::asio::ssl;
using tcp = boost::asio::ip::tcp;

class WebSocketClient: public std::enable_shared_from_this<WebSocketClient> {
public:
    WebSocketClient(net::io_context& ioc, ssl::context& ctx);
    ~WebSocketClient();

    void connect(const std::string& host, const std::string& port, const std::string& path);
    void send(const std::vector<uint8_t>& message);
    void close();
    
    void set_message_handler(std::function<void(const std::string&)> handler);

private:
    void on_resolve(beast::error_code ec, tcp::resolver::results_type results);
    void on_connect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);
    void on_ssl_handshake(beast::error_code ec);
    void on_handshake(beast::error_code ec);
    void on_write(beast::error_code ec, std::size_t bytes_transferred);
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void on_close(beast::error_code ec);

    net::io_context& ioc_;
    ssl::context& ctx_;
    tcp::resolver resolver_;
    websocket::stream<beast::ssl_stream<tcp::socket>> ws_;
    beast::flat_buffer buffer_;
    std::string host_;
    std::string path_;
    std::function<void(const std::string&)> message_handler_;
};