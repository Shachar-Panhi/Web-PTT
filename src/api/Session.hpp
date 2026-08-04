#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <glaze/glaze.hpp>
#include <rtc/rtc.hpp>
#include "boost/asio/any_io_executor.hpp"
#include <boost/beast/http.hpp>
#include <spdlog/spdlog.h>

namespace PTT {
    using tcp = boost::asio::ip::tcp;
    namespace HTTP = boost::beast::http;

    class Session : public std::enable_shared_from_this<Session> {
    public: 
        explicit Session(tcp::socket);
        boost::asio::awaitable<void> start();
        HTTP::response<HTTP::string_body> handle_request(const HTTP::request<HTTP::string_body>& req);
        boost::asio::awaitable<void> send_response(const HTTP::response<HTTP::string_body>& res);
        void handle_is_alive(HTTP::response<HTTP::string_body>& res);
        void handle_ptt_start(HTTP::response<HTTP::string_body>& res);
        void handle_ptt_stop(HTTP::response<HTTP::string_body>& res);


    private:
        boost::asio::any_io_executor executor_;
        boost::beast::tcp_stream stream_;
    };
}  // namespace PTT