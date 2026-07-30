#ifndef backend_hpp
#define backend_hpp

#include <boost/asio.hpp>
#include <glaze/glaze.hpp>
#include <rtc/rtc.hpp>

namespace PTT {
    using boost::asio::ip::tcp;

    boost::asio::awaitable<void> handle_session(tcp::socket socket);
    boost::asio::awaitable<void> listener();

}  // namespace PTT

#endif