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
    
    void setMessageHandler(std::function<void(const std::string&)> handler);

private:
    void onResolve(beast::error_code ec, tcp::resolver::results_type results);
    void onConnect(beast::error_code ec, tcp::resolver::results_type::endpoint_type ep);
    void onSslHandshake(beast::error_code ec);
    void onHandshake(beast::error_code ec);
    void onWrite(beast::error_code ec, std::size_t bytesTransferred);
    void onRead(beast::error_code ec, std::size_t bytesTransferred);
    void onClose(beast::error_code ec);

    net::io_context& _ioc;
    ssl::context& _ctx;
    tcp::resolver _resolver;
    websocket::stream<beast::ssl_stream<tcp::socket>> _ws;
    beast::flat_buffer _buffer;
    std::string _host;
    std::string _path;
    std::function<void(const std::string&)> _messageHandler;
};
