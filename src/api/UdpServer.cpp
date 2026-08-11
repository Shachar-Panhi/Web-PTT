#include "UdpServer.hpp"
#include <spdlog/spdlog.h>
#include <cstdint>

#include "../../ThirdParty/G711/g711.h"
#include "boost/asio/any_io_executor.hpp"

namespace WebPTT::UDP {
    UdpServer::UdpServer(const boost::asio::any_io_executor& executor)
        : socket_(executor), 
        rtp_packet_(kMaxPacketSize), 
        pcm_buffer_(kMaxPacketSize) {

        boost::system::error_code errc;
        auto endpoint = udp::endpoint(boost::asio::ip::make_address(kIpAddress, errc), kPort);
        if (errc) {
            spdlog::error("invalid IP address: {}", errc.message());
            return;
        }

        errc = socket_.open(endpoint.protocol(), errc);
        errc = socket_.bind(endpoint, errc);
        if (errc) {
            spdlog::error("failed to start server: {}", errc.message());
            return;
        }

        spdlog::info("Socket Address = {}, Socket Port = {}", socket_.local_endpoint().address().to_string(), socket_.local_endpoint().port());
    }

boost::asio::awaitable<void> UdpServer::start() {
    boost::system::error_code errc;
    boost::asio::ip::udp::endpoint remote_endpoint;    

    spdlog::info("UDP server is listening on port {}", kPort);


    while (socket_.is_open()) {
        auto& raw_buffer = rtp_packet_.buffer();

        std::size_t bytes_recvd = co_await socket_.async_receive_from(
            boost::asio::buffer(raw_buffer), remote_endpoint,
            boost::asio::redirect_error(boost::asio::use_awaitable, errc)
        );

        if (!errc && bytes_recvd > 0) {
            process_packet(bytes_recvd);
        } else if (errc) {
            spdlog::error("Receive error: {}", errc.message());
            if (errc == boost::asio::error::operation_aborted) {
                break;
            }
        }
    }
}

    void UdpServer::process_packet(std::size_t bytes_recvd) {
                
        auto result = rtp_packet_.parse(bytes_recvd);

        if (!(result == RtpCpp::Result::kSuccess))
        {
            spdlog::error("Parsing error");
            return;
        }

        uint32_t timestamp = rtp_packet_.get_header().timestamp_;
        uint16_t seq_num = rtp_packet_.get_header().sequence_number_;

        spdlog::info("Timestamp: {} , Sequence: {}", timestamp, seq_num);

        auto payload = rtp_packet_.payload();
        std::size_t payload_size = payload.size();
        std::size_t header_size = bytes_recvd - payload_size;
        
        spdlog::info("payload: {}, byte size: {}, header size: {}", payload_size, bytes_recvd, header_size);

        size_t decoded_samples = g711_alaw_decode(
            payload.data() + header_size, 
            payload_size, 
            pcm_buffer_.data(), 
            pcm_buffer_.size()
        );

        spdlog::info("size of PCM: {} bytes", decoded_samples);
    }
}  // namespace WebPTT::UDP