#pragma once

#include <vector>
#include <cstdint>

namespace WebPTT::Audio {
    class AudioData {
    public:
        void set_is_recording(bool value) {
            is_recording_ = value;
        }
        bool get_is_recording() const {
            return is_recording_;
        }
        std::vector<uint16_t>& get_audio_vector(){
            return audio_vector_;
        } 
        void clear_audio_vector(){
            audio_vector_.clear();
        }
    private:
        std::vector<uint16_t> audio_vector_;
        bool is_recording_{false};
    };
}  // namespace WebPTT::Audio