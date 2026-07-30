#include "backend.hpp"
#include <boost/beast.hpp>
#include <spdlog/spdlog.h>

#define DR_WAV_IMPLEMENTATION
#include "ThirdParty/dr_wav.h"

#include "ThirdParty/G711/g711_ref.h"

#include <string>
#include <memory>


namespace PTT {
    constexpr int kPort = 8080;
    const std::string kIPAddress = "127.0.0.1";

    using tcp = boost::asio::ip::tcp;

    boost::asio::awaitable<void> handle_session(tcp::socket socket) {
        boost::system::error_code errc;
        auto web_socket = std::make_shared<boost::beast::websocket::stream<tcp::socket>>(std::move(socket));

        co_await web_socket->async_accept(redirect_error(boost::asio::use_awaitable, errc));
        if (errc) {
            spdlog::error("WebSocket accept failed: {}", errc.message());
            co_return;
        }

        spdlog::info("client connected successfully");

        boost::beast::flat_buffer buffer;

        while (web_socket->is_open()) {
            buffer.clear();
            co_await web_socket->async_read(buffer, redirect_error(boost::asio::use_awaitable, errc));
            
            if (errc) {
                if (errc == boost::beast::websocket::error::closed) {
                    spdlog::info("client disconnected");
                } else {
                    spdlog::error("read error: {}", errc.message());
                }
                break;
            }

            std::string incoming = boost::beast::buffers_to_string(buffer.data());
            spdlog::info("received message: {}", incoming);

            std::string response = "server received: " + incoming;
            
            co_await web_socket->async_write(boost::asio::buffer(response), redirect_error(boost::asio::use_awaitable, errc));
            if (errc) {
                spdlog::error("write error: {}", errc.message());
                break;
            }
        }
    }

    boost::asio::awaitable<void> listener() {
        auto io_context = co_await boost::asio::this_coro::executor;
        boost::system::error_code errc;

        tcp::acceptor acceptor(io_context);

        tcp::endpoint endpoint(boost::asio::ip::make_address(kIPAddress, errc), kPort);
        if (errc){
            spdlog::error("invalid ip address {}", errc.message());
            co_return;
        }

        errc = acceptor.open(endpoint.protocol(), errc);
        errc = acceptor.bind(endpoint, errc);
        errc = acceptor.listen(boost::asio::socket_base::max_listen_connections, errc);

        if (errc) {
            spdlog::error("failed to start server: {}", errc.message());
            co_return;
        }
        
        
        spdlog::info("server is listening on port 8080");
        
        while (true) {
            tcp::socket socket = co_await acceptor.async_accept(redirect_error(boost::asio::use_awaitable, errc));
            if (!errc) {
                co_spawn(io_context, handle_session(std::move(socket)), boost::asio::detached);
            }
        }
    }
}  // namespace PTT

int main() {
    boost::asio::io_context io_context;

    co_spawn(io_context, PTT::listener(), boost::asio::detached);
    io_context.run();
    return 0;
}