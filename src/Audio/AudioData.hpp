#pragma once

#include <vector>
#include <cstdint>

namespace WebPTT::Audio {
    class AudioData {
    public:
        AudioData();
        void set_is_recording(bool value);
        bool get_is_recording() const;
        std::vector<int16_t>& get_audio_vector();
    private:
        std::vector<int16_t> pcm_buffer_;
        bool is_recording_{false};
    };
}  // namespace WebPTT::Audio