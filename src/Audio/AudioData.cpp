#include "AudioData.hpp"

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

}  // namespace WebPTT::Audio