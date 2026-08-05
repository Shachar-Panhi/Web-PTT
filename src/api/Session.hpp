#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <glaze/glaze.hpp>
#include <rtc/rtc.hpp>
#include "boost/asio/any_io_executor.hpp"
#include <boost/beast/http.hpp>
#include <spdlog/spdlog.h>
#include <expected>

namespace WebPTT::Api {
    using tcp = boost::asio::ip::tcp;
    namespace HTTP = boost::beast::http;

    class Session : public std::enable_shared_from_this<Session> {
    public: 
        explicit Session(tcp::socket);
        boost::asio::awaitable<void> start();
        static std::expected<HTTP::response<HTTP::string_body>, HTTP::response<HTTP::file_body>> handle_request(const HTTP::request<HTTP::string_body>& req);
        boost::asio::awaitable<void> send_response(HTTP::response<HTTP::string_body>& res);
        boost::asio::awaitable<void> send_response_filebody(HTTP::response<HTTP::file_body>& res);
        static HTTP::response<HTTP::string_body> handle_is_alive(const HTTP::request<HTTP::string_body>& req);
        static HTTP::response<HTTP::string_body> handle_ptt_start(const HTTP::request<HTTP::string_body>& req);
        static std::expected<HTTP::response<HTTP::file_body>, HTTP::response<HTTP::string_body>> handle_ptt_stop(const HTTP::request<HTTP::string_body>& req);


    private:
        boost::asio::any_io_executor executor_;
        boost::beast::tcp_stream stream_;
    };
}  // namespace WebPTT::Api