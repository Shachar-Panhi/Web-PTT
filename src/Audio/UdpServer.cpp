#include "UdpServer.hpp"
#include <spdlog/spdlog.h>
#include <cstdint>

#include "AudioData.hpp"
#include "boost/asio/any_io_executor.hpp"

namespace WebPTT::Audio {
    UdpServer::UdpServer(const boost::asio::any_io_executor& executor, std::shared_ptr<WebPTT::Audio::AudioData> audio_data)
    : socket_(executor)
    , audio_data_(std::move(audio_data)) {

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

            if (errc) {
                spdlog::error("Receive error: {}", errc.message());
                if (errc == boost::asio::error::operation_aborted) {
                    break;
                }
            }
            
            if (audio_data_->get_is_recording() && bytes_recvd > 0) {
                process_packet(bytes_recvd);
            }
        }
    }

    void UdpServer::process_packet(std::size_t bytes_recvd) {
        auto result = rtp_packet_.parse(bytes_recvd);

        if (result != RtpCpp::Result::kSuccess)
        {
            spdlog::error("Parsing error");
            return;
        }

        if (rtp_packet_.get_header().payload_type_ != kG711PayloadType)
        {
            spdlog::error("Wrong payload type");
            return;
        }

        uint32_t timestamp = rtp_packet_.get_header().timestamp_;
        uint16_t seq_num = rtp_packet_.get_header().sequence_number_;

        spdlog::info("Timestamp: {} , Sequence: {}", timestamp, seq_num);

        auto payload = rtp_packet_.payload();
        spdlog::info("payload: {}, byte size: {}, header size: {}", payload.size(), bytes_recvd, bytes_recvd - payload.size());
        
        
        audio_data_->store_packets(payload);
        
    }
}  // namespace WebPTT::Audio