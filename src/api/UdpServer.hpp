#pragma once

#include <array>
#include <cstdint>
#include <boost/asio.hpp>

namespace WebPTT::UDP
{
    using udp = boost::asio::ip::udp;
    constexpr int kArraySize = 2048;
    constexpr int kPort = 8081;
    constexpr auto kIpAddress = "127.0.0.1";

    class UdpServer {
    public:
        explicit UdpServer(boost::asio::io_context& io_context);
        boost::asio::awaitable<void> start();
        static void process_packet(std::size_t bytes_recvd, std::array<uint8_t, kArraySize> recv_buffer);

    private:
        boost::asio::ip::udp::socket socket_;
        bool is_recording_;
    };
}  // namespace WebPTT::UDP