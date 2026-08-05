#include "Session.hpp"
#include "../utils/JsonUtils.hpp"
#include "Types.hpp"
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
            std::expected<HTTP::response<HTTP::string_body>, HTTP::response<HTTP::file_body>> res = handle_request(req);
            
            bool need_eof = false;
            if (res) {
                need_eof = res->need_eof();
                co_await send_response(*res);
            } else {
                need_eof = res.error().need_eof();
                co_await send_response_filebody(res.error());
            }

            if (need_eof) {
                errc = stream_.socket().shutdown(tcp::socket::shutdown_send, errc);
                break;
            }
        }
    }

    std::expected<HTTP::response<HTTP::string_body>, HTTP::response<HTTP::file_body>> Session::handle_request (const HTTP::request<HTTP::string_body>& req) {
        HTTP::response<HTTP::string_body> res;
        std::expected<HTTP::response<HTTP::file_body>, HTTP::response<HTTP::string_body>> file_res;

        if (req.target() == "/isAlive" && req.method() == HTTP::verb::get) {
            res = handle_is_alive(req);
        }
        else if (req.target() == "/ptt/start" && req.method() == HTTP::verb::post) {
            res = handle_ptt_start(req);
        }
        else if (req.target() == "/ptt/stop" && req.method() == HTTP::verb::post) {
            file_res = handle_ptt_stop(req);
            if (file_res) {
                return std::unexpected(std::move(*file_res));
            }
            res = file_res.error();
        }
        else {
            res.set(HTTP::field::content_type, "application/json");
            res.result(HTTP::status::not_found);
            res.body() = R"({"status": "not found"})";
        }

        res.prepare_payload();

        return std::move(res);
    }
    
    boost::asio::awaitable<void> Session::send_response(HTTP::response<HTTP::string_body>& res) {
        boost::system::error_code errc;                        
        co_await HTTP::async_write(stream_, res, boost::asio::redirect_error(boost::asio::use_awaitable, errc));
            if (errc) {
                spdlog::error("write error: {}", errc.message());
                co_return;
            }
    }

    boost::asio::awaitable<void> Session::send_response_filebody(HTTP::response<HTTP::file_body>& res) {
        boost::system::error_code errc; 
        co_await HTTP::async_write(stream_, res, boost::asio::redirect_error(boost::asio::use_awaitable, errc));
        if (errc) {
            spdlog::error("write error: {}", errc.message());
            co_return;
        }
    }

    HTTP::response<HTTP::string_body> Session::handle_is_alive(const HTTP::request<HTTP::string_body>& req) {
        HTTP::response<HTTP::string_body> res{HTTP::status::ok, req.version()};
        res.set(HTTP::field::content_type, "application/json");
        res.result(HTTP::status::ok);
        res.body() = R"({"message": "alive", "status": "ok"})";
        return res;
    }

    HTTP::response<HTTP::string_body> Session::handle_ptt_start(const HTTP::request<HTTP::string_body>& req) {
        HTTP::response<HTTP::string_body> res{HTTP::status::ok, req.version()};
        
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
        return res;
    }   
    
    std::expected<HTTP::response<HTTP::file_body>, HTTP::response<HTTP::string_body>> Session::handle_ptt_stop(const HTTP::request<HTTP::string_body>& req) {
        boost::beast::error_code errc;
        HTTP::file_body::value_type body;


        body.open("src/dummy.wav", boost::beast::file_mode::read, errc);
        if (errc) {
            spdlog::error("Failed to open file: {}", errc.message());
            HTTP::response<HTTP::string_body> res{HTTP::status::ok, req.version()};
            res.result(HTTP::status::internal_server_error);
            res.set(HTTP::field::content_type, "application/json");
            res.body() = R"({"message": "Failed to open file", "status": "error"})";
            return std::unexpected(res);
        }
        
        HTTP::response<HTTP::file_body> file_res{std::piecewise_construct, std::make_tuple(std::move(body)), std::make_tuple(HTTP::status::ok, req.version())};        
        file_res.set(HTTP::field::content_type, "audio/wav");

        return std::move(file_res);
    }   
}  // namespace WebPTT::Api