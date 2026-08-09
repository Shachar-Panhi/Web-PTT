#include "Session.hpp"
#include "../utils/JsonUtils.hpp"
#include "Types.hpp"
#include <glaze/glaze.hpp>
#include <rtc/rtc.hpp>
#include "boost/beast/http/message_fwd.hpp"

namespace WebPTT::Api { 
    Session::Session(tcp::socket socket)
    : executor_(socket.get_executor())
    , stream_(std::move(socket)) {}
    
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
        auto result = WebPTT::Utils::parse_json<MessageBody>(req.body());
        
        if (result) {
            const MessageBody& body = result.value();
            spdlog::info("Received message: {}, status: {}", body.message_, body.status_);
            return make_api_response(HTTP::status::ok, req.version(), R"({"message": "received", "status": "ok"})");
        }  
        std::string glz_error = glz::format_error(result.error());
        spdlog::error("Failed to parse request body: {}", glz_error);
        
        ErrorResponse error_response{.message_ = std::move(glz_error), .status_ = "error"};
        auto result_json = WebPTT::Utils::serialize_json(error_response);
            
        if (result_json) {
            return make_error_response(HTTP::status::bad_request, req.version(), result_json.value());
        }  
        return make_error_response(HTTP::status::internal_server_error, req.version(), R"({"message": "Failed to serialize error response", "status": "error"})");
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