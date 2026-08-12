#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include "boost/asio/any_io_executor.hpp"
#include <boost/beast/http.hpp>
#include <spdlog/spdlog.h>
#include <expected>

#include "../Audio/AudioData.hpp"


namespace WebPTT::Api {
    using tcp = boost::asio::ip::tcp;
    namespace HTTP = boost::beast::http;

    class Session : public std::enable_shared_from_this<Session> {
    public: 
        explicit Session(tcp::socket, std::shared_ptr<WebPTT::Audio::AudioData> audio_data);
        boost::asio::awaitable<void> start();
        boost::asio::awaitable<void> handle_request(HTTP::request<HTTP::string_body> req);
        static HTTP::response<HTTP::string_body> make_api_response(HTTP::status status, unsigned int version, const std::string& body_data);
        static HTTP::response<HTTP::string_body> make_error_response(HTTP::status status, unsigned int version, const std::string& body_data);
        static HTTP::response<HTTP::string_body> handle_is_alive(const HTTP::request<HTTP::string_body>& req);
        HTTP::response<HTTP::string_body> handle_ptt_start(const HTTP::request<HTTP::string_body>& req);
        static std::expected<HTTP::response<HTTP::file_body>, HTTP::response<HTTP::string_body>> handle_ptt_stop(const HTTP::request<HTTP::string_body>& req);

        template <typename T>
        boost::asio::awaitable<void> send_response(HTTP::response<T> res) {
            boost::system::error_code errc;                        
            res.prepare_payload();
            co_await HTTP::async_write(stream_, res, boost::asio::redirect_error(boost::asio::use_awaitable, errc));
            if (errc) {
                spdlog::error("write error: {}", errc.message());
                co_return;
            }
        }


    private:
        boost::asio::any_io_executor executor_;
        boost::beast::tcp_stream stream_;
        std::shared_ptr<WebPTT::Audio::AudioData> audio_data_;
    };
}  // namespace WebPTT::Api