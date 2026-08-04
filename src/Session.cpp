#include "Session.hpp"


namespace PTT{ 
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
            handle_ptt_start(res);
        }
        else if (req.target() == "/ptt/stop" && req.method() == HTTP::verb::post) {
            handle_ptt_stop(res);
        }
        else {
            res.result(HTTP::status::not_found);
            res.body() = "404 Not Found";
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
        res.result(HTTP::status::ok);
        res.set(HTTP::field::content_type, "text/plain");
        res.body() = "server is alive";
    }

    void Session::handle_ptt_start(HTTP::response<HTTP::string_body>& res) {
        res.result(HTTP::status::ok);
        res.set(HTTP::field::content_type, "text/plain");
        res.body() = "started receiving information";
    }    
    
    void Session::handle_ptt_stop(HTTP::response<HTTP::string_body>& res) {
        res.result(HTTP::status::ok);
        res.set(HTTP::field::content_type, "text/plain");
        res.body() = "stopped receiving information";
    }   
}  // namespace PTT