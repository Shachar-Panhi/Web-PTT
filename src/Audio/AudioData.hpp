#pragma once

#include <vector>
#include <cstdint>

namespace WebPTT::Audio {
    class AudioData {
    public:
        void set_is_recording(bool value) {
            is_recording_ = value;
            if(is_recording_) {
                pcm_buffer_.clear();
            }
        }
        bool get_is_recording() const {
            return is_recording_;
        }
        std::vector<int16_t>& get_audio_vector(){
            return pcm_buffer_;
        } 
    private:
        std::vector<int16_t> pcm_buffer_;
        bool is_recording_{false};
    };
}  // namespace WebPTT::Audio