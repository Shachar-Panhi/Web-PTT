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
        [[nodiscard]] bool get_is_recording() const;
        std::span<int16_t> get_audio_span();
        void decode_store_packets(std::span<std::uint8_t> payload);
    private:
        std::vector<int16_t> pcm_buffer_;
        bool is_recording_{false};
    };
}  // namespace WebPTT::Audio