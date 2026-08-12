#include "Session.hpp"
#include <glaze/glaze.hpp>
#include <rtc/rtc.hpp>
#include "boost/beast/http/message_fwd.hpp"


namespace WebPTT::Api { 
    Session::Session(tcp::socket socket, std::shared_ptr<WebPTT::Audio::AudioData> audio_data)
    : executor_(socket.get_executor())
    , stream_(std::move(socket))
    , audio_data_(std::move(audio_data)) {}
    
    boost::asio::awaitable<void> Session::start() {
        boost::system::error_code errc;                    
        boost::beast::flat_buffer buffer;

        auto self = shared_from_this();
        
        spdlog::info("client connected successfully");

        while (stream_.socket().is_open()) {
            HTTP::request<HTTP::string_body> req;   
            buffer.clear();

            co_await HTTP::async_read(stream_, buffer, req, boost::asio::redirect_error(boost::asio::use_awaitable, errc));
            
            if (errc) {
                if (errc == HTTP::error::end_of_stream || errc == boost::asio::error::connection_reset) {
                    spdlog::info("client disconnected");
                } else {
                    spdlog::error("read error: {}", errc.message());
                }
                break;
            }
            co_await handle_request(std::move(req));
        }
    }

    boost::asio::awaitable<void> Session::handle_request(HTTP::request<HTTP::string_body> req) {

        if (req.target() == "/isAlive" && req.method() == HTTP::verb::get) {
            co_await send_response(handle_is_alive(req));
        }
        else if (req.target() == "/ptt/start" && req.method() == HTTP::verb::post) {
            co_await send_response(handle_ptt_start(req));
        }
        else if (req.target() == "/ptt/stop" && req.method() == HTTP::verb::post) {
            auto file_res = handle_ptt_stop(req);
            if (file_res) {
                co_await send_response(std::move(*file_res));
            } else {
                co_await send_response(std::move(file_res.error()));
            }
        }
        else {
            co_await send_response(make_error_response(HTTP::status::not_found, req.version(), R"({"status": "not found"})"));
        }
    }

    HTTP::response<HTTP::string_body> Session::make_api_response(HTTP::status status, unsigned int version, const std::string& body_data) {
        HTTP::response<HTTP::string_body> res{status, version};
        res.set(HTTP::field::content_type, "application/json");
        res.body() = body_data;
        return res;
    }

    HTTP::response<HTTP::string_body> Session::make_error_response(HTTP::status status, unsigned int version, const std::string& body_data) {
        HTTP::response<HTTP::string_body> res{status, version};
        res.set(HTTP::field::content_type, "application/json");
        res.body() = body_data;
        return res;
    }

    HTTP::response<HTTP::string_body> Session::handle_is_alive(const HTTP::request<HTTP::string_body>& req) {
        return make_api_response(HTTP::status::ok, req.version(), R"({"message": "alive", "status": "ok"})");
    }

    HTTP::response<HTTP::string_body> Session::handle_ptt_start(const HTTP::request<HTTP::string_body>& req) {
        
        audio_data_->set_is_recording(true);
        audio_data_->clear_audio_vector();
        return make_api_response(HTTP::status::ok, req.version(), R"({"message": "recording started", "status": "ok"})");
    }   
    
    std::expected<HTTP::response<HTTP::file_body>, HTTP::response<HTTP::string_body>> Session::handle_ptt_stop(const HTTP::request<HTTP::string_body>& req) {
        boost::beast::error_code errc;
        HTTP::file_body::value_type body;

        body.open("src/dummy.wav", boost::beast::file_mode::read, errc);
        
        if (errc) {
            spdlog::error("Failed to open file: {}", errc.message());
            return std::unexpected(make_error_response(HTTP::status::internal_server_error, req.version(), R"({"message": "Failed to open file", "status": "error"})"));
        }
        
        HTTP::response<HTTP::file_body> file_res{std::piecewise_construct, std::make_tuple(std::move(body)), std::make_tuple(HTTP::status::ok, req.version())};        
        file_res.set(HTTP::field::content_type, "audio/wav");

        return file_res;
    }   
}  // namespace WebPTT::Api