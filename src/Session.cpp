#include "Session.hpp"


namespace PTT{ 
    Session::Session(tcp::socket socket)
    : executor_(socket.get_executor())
    , stream_(std::move(socket)) {}
    
    boost::asio::awaitable<void> Session::handle_session() {
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

            HTTP::response<HTTP::string_body> res{HTTP::status::ok, req.version()};
            res.set(HTTP::field::access_control_allow_origin, "*");
            res.set(HTTP::field::access_control_allow_headers, "*");
            res.set(HTTP::field::access_control_allow_methods, "GET, POST, OPTIONS");

            if (req.method() == HTTP::verb::get) {
                handle_get_request(&res, req.target());
            }
            else {
                res.result(HTTP::status::method_not_allowed);
                res.body() = "Method Not Allowed";
            }

            res.prepare_payload();

            co_await HTTP::async_write(stream_, res, boost::asio::redirect_error(boost::asio::use_awaitable, errc));
            if (errc) {
                spdlog::error("write error: {}", errc.message());
                break;
            }

            if (req.need_eof()) {
                boost::system::error_code shutdown_errc;
                errc = stream_.socket().shutdown(tcp::socket::shutdown_send, shutdown_errc);
                break;
            }
        }
    }

    void Session::handle_get_request(HTTP::response<HTTP::string_body>* res, std::string target) {
        res->result(HTTP::status::ok);
        res->set(HTTP::field::content_type, "text/plain");
        if (target == "/isAlive") {
            res->body() = "server is alive";
        } 
        else if (target == "/ptt/start") {
            res->body() = "started receiving information";
        } 
        else if (target == "/ptt/stop") {
            res->body() = "stopped receiving information";
        } 
        else {
            res->result(HTTP::status::not_found);
            res->body() = "404 Not Found";
        }
    }
}  // namespace PTT