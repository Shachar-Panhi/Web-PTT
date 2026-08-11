#pragma once

#include "../../ThirdParty/RtpCpp/RtpPacket.hpp"
#include <cstdint>
#include <boost/asio.hpp>
#include "boost/asio/any_io_executor.hpp"

namespace WebPTT::UDP
{
    using udp = boost::asio::ip::udp;
    constexpr int kMaxPacketSize = 2048;
    constexpr int kPort = 8081;
    constexpr auto kIpAddress = "127.0.0.1";

    class UdpServer {
    public:
        explicit UdpServer(const boost::asio::any_io_executor& executor);
        boost::asio::awaitable<void> start();
        void process_packet(std::size_t bytes_recvd);

    private:
        boost::asio::ip::udp::socket socket_;
        RtpCpp::RtpPacket<std::vector<uint8_t>> rtp_packet_;
        std::vector<int16_t> pcm_buffer_;
    };
}  // namespace WebPTT::UDP