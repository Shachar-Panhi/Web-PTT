#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <glaze/glaze.hpp>
#include <rtc/rtc.hpp>
#include "boost/asio/any_io_executor.hpp"
#include <boost/beast/http.hpp>
#include <spdlog/spdlog.h>

namespace WebPTT::Api {
    using tcp = boost::asio::ip::tcp;
    
    class Listener {
        public: explicit Listener(const boost::asio::any_io_executor&);
        boost::asio::awaitable<void> listen();
    private:
        tcp::acceptor acceptor_;
    };
} // namespace WebPTT::Api