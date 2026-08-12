#include "AudioData.hpp"
#include "../../ThirdParty/G711/g711.h"
#include <array>
#include <spdlog/spdlog.h>

namespace WebPTT::Audio {
    AudioData::AudioData() {}

    void AudioData::set_is_recording(bool value) {
        is_recording_ = value;
        if(is_recording_) {
            pcm_buffer_.clear();
        }
    }
    bool AudioData::get_is_recording() const {
        return is_recording_;
    }

    std::vector<int16_t>& AudioData::get_audio_vector(){
        return pcm_buffer_;
    } 

    void AudioData::store_packets(std::span<std::uint8_t> payload) {
        std::array<int16_t, kPCMSize> pcm_buffer{};
        auto decoded_samples = decode_into_g711(payload, pcm_buffer);

        pcm_buffer_.insert(
            pcm_buffer_.end(),
            pcm_buffer.begin(),
            pcm_buffer.begin() + decoded_samples
        );

        spdlog::info("size of PCM: {} bytes", decoded_samples);

    }
    size_t AudioData::decode_into_g711(std::span<const uint8_t> payload, std::array<int16_t, kPCMSize>& pcm_buffer) {

        size_t decoded_samples = g711_alaw_decode(
            payload.data(), 
            payload.size(), 
            pcm_buffer.data(), 
            pcm_buffer.size()
        );
        return decoded_samples;
    }
}  // namespace WebPTT::Audio