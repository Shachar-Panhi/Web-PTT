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
        boost::asio::awaitable<void> handle_session();
        void handle_get_request(HTTP::response<HTTP::string_body>* res, std::string target);
    private:
        boost::asio::any_io_executor executor_;
        boost::beast::tcp_stream stream_;
    };
}  // namespace PTT