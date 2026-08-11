#include "UdpServer.hpp"
#include <spdlog/spdlog.h>
#include <cstdint>

#include "../../ThirdParty/RtpCpp/RtpPacket.hpp"
#include "../../ThirdParty/G711/g711.h"

namespace WebPTT::UDP {
    UdpServer::UdpServer(boost::asio::io_context& io_context)
        : socket_(io_context), is_recording_() {

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
    std::array<uint8_t, kArraySize> recv_buffer{};

    spdlog::info("UDP server is listening on port {}", kPort);


    while (socket_.is_open()) {
        std::size_t bytes_recvd = co_await socket_.async_receive_from(
            boost::asio::buffer(recv_buffer), remote_endpoint,
            boost::asio::redirect_error(boost::asio::use_awaitable, errc)
        );

        if (!errc && bytes_recvd > 0) {
            process_packet(bytes_recvd, recv_buffer);
        } else if (errc) {
            spdlog::error("Receive error: {}", errc.message());
            if (errc == boost::asio::error::operation_aborted) {
                break;
            }
        }
    }
}

    void UdpServer::process_packet(std::size_t bytes_recvd, std::array<uint8_t, kArraySize> recv_buffer) {
        std::vector<uint8_t> buffer(recv_buffer.data(), recv_buffer.data() + bytes_recvd);
        RtpCpp::RtpPacket packet(std::move(buffer));
        
        auto result = packet.parse();
        if (result == decltype(result)::kSuccess) {
            uint32_t timestamp = packet.get_header().timestamp_;
            uint16_t seq_num = packet.get_header().sequence_number_;

            spdlog::info("Timestamp: {} , Sequence: {}", timestamp, seq_num);

            std::size_t payload_size = packet.get_payload_size();
            std::size_t header_size = bytes_recvd - payload_size;
            
            std::vector<int16_t> pcm_data(payload_size);

            size_t decoded_samples = g711_alaw_decode(
                recv_buffer.data() + header_size, 
                payload_size, 
                pcm_data.data(), 
                pcm_data.size()
            );

            std::size_t pcm_byte_size = decoded_samples * sizeof(int16_t);
            spdlog::info("size of PCM: {} bytes", pcm_byte_size);
        }
        else {
            spdlog::error("Parsing error");
        }
    }
}  // namespace WebPTT::UDP