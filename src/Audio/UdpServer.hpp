#pragma once

#include "../../ThirdParty/RtpCpp/RtpPacket.hpp"
#include <cstdint>
#include <boost/asio.hpp>
#include "boost/asio/any_io_executor.hpp"

namespace WebPTT::Audio
{
    using udp = boost::asio::ip::udp;
    constexpr int kMaxPacketSize = 2048;
    constexpr int kPCMSize = 160;
    constexpr int kRtpSize = 172;
    constexpr int kPort = 8081;
    constexpr int kG711PayloadType = 8;
    constexpr auto kIpAddress = "127.0.0.1";

    class UdpServer {
    public:
        explicit UdpServer(const boost::asio::any_io_executor& executor);
        boost::asio::awaitable<void> start();
        void process_packet(std::size_t bytes_recvd);

    private:
        boost::asio::ip::udp::socket socket_;
        RtpCpp::RtpPacket<std::array<uint8_t, kRtpSize>> rtp_packet_;
    };
}  // namespace WebPTT::Audio