#pragma once

#include <span>
#include <vector>
#include <cstdint>

namespace WebPTT::Audio {
    constexpr int kPCMSize = 160;

    class AudioData {
    public:
        AudioData();
        void set_is_recording(bool value);
        bool get_is_recording() const;
        std::vector<int16_t>& get_audio_vector();
        static void store_packets(std::span<std::uint8_t> payload, std::vector<int16_t>& audio_vector);
        static size_t decode_into_g711(std::span<std::uint8_t> payload, std::array<int16_t, kPCMSize>& pcm_buffer);
    private:
        std::vector<int16_t> pcm_buffer_;
        bool is_recording_{false};
    };
}  // namespace WebPTT::Audio