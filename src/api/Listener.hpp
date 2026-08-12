#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <glaze/glaze.hpp>
#include <rtc/rtc.hpp>
#include "boost/asio/any_io_executor.hpp"
#include <boost/beast/http.hpp>
#include <spdlog/spdlog.h>

#include "../Audio/AudioData.hpp"

namespace WebPTT::Api {
    using tcp = boost::asio::ip::tcp;
    
    class Listener {
        public: explicit Listener(const boost::asio::any_io_executor&, std::shared_ptr<WebPTT::Audio::AudioData> audio_data);
        boost::asio::awaitable<void> listen();
    private:
        tcp::acceptor acceptor_;
        std::shared_ptr<WebPTT::Audio::AudioData> audio_data_;
    };
} // namespace WebPTT::Api