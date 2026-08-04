#include "Session.hpp"
#include "../utils/JsonUtils.hpp"
#include "Types.hpp"


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
            HTTP::response<HTTP::string_body> res = handle_request(req);
            co_await send_response(res);
        }
    }

    HTTP::response<HTTP::string_body> Session::handle_request (const HTTP::request<HTTP::string_body>& req) {

        HTTP::response<HTTP::string_body> res{HTTP::status::ok, req.version()};

        if (req.target() == "/isAlive" && req.method() == HTTP::verb::get) {
            handle_is_alive(res);
        }
        else if (req.target() == "/ptt/start" && req.method() == HTTP::verb::post) {
            handle_ptt_start(req, res);
        }
        else if (req.target() == "/ptt/stop" && req.method() == HTTP::verb::post) {
            handle_ptt_stop(res);
        }
        else {
            res.set(HTTP::field::content_type, "application/json");
            res.result(HTTP::status::not_found);
            res.body() = R"({"status": "not found"})";

        }

        res.prepare_payload();

        return res;
    }
    
    boost::asio::awaitable<void> Session::send_response(const HTTP::response<HTTP::string_body>& res) {
        boost::system::error_code errc;                        
        co_await HTTP::async_write(stream_, res, boost::asio::redirect_error(boost::asio::use_awaitable, errc));
            if (errc) {
                spdlog::error("write error: {}", errc.message());
                co_return;
            }
    }

    void Session::handle_is_alive(HTTP::response<HTTP::string_body>& res) {
        res.set(HTTP::field::content_type, "application/json");
        res.result(HTTP::status::ok);
        res.body() = R"({"message": "alive", "status": "ok"})";

    }

    void Session::handle_ptt_start(const HTTP::request<HTTP::string_body>& req, HTTP::response<HTTP::string_body>& res) {
        auto result = WebPTT::Utils::parse_json<MessageBody>(req.body());
        if (result) {
            const MessageBody& body = result.value();
            spdlog::info("Received message: {}, status: {}", body.message_, body.status_);
            res.set(HTTP::field::content_type, "application/json");
            res.result(HTTP::status::ok);
            res.body() = R"({"message": "received", "status": "ok"})";
        } else {
            std::string glz_error = glz::format_error(result.error());
            spdlog::error("Failed to parse request body: {}", glz_error);
            ErrorResponse error_response{.message_ = std::move(glz_error), .status_ = "error"};
            
            res.set(HTTP::field::content_type, "application/json");
            auto result_json = WebPTT::Utils::serialize_json(error_response);
            if (result_json) {
                res.result(HTTP::status::bad_request);
                res.body() = result_json.value();
            } else {
                    res.result(HTTP::status::internal_server_error);
                    res.body() = R"({"message": "Failed to serialize error response", "status": "error"})";
            }
        }
    }    
    
    void Session::handle_ptt_stop(HTTP::response<HTTP::string_body>& res) {
        res.result(HTTP::status::ok);
        res.set(HTTP::field::content_type, "text/plain");
        res.body() = "stopped receiving information";
    }   
}  // namespace WebPTT::Api