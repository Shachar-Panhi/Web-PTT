#define DR_WAV_IMPLEMENTATION
#include "dr_wav_wrapper.hpp"

namespace DrWav::Wrapper {
    constexpr int kSampleRate = 8000;
    constexpr int kChannels = 1;
    constexpr int kBitsPerSample = 16;


    DrWavWrapper::DrWavWrapper() = default;

    DrWavWrapper::~DrWavWrapper() {
        close();
    }

    DrWavWrapper::DrWavWrapper(DrWavWrapper&& other) noexcept 
        : wav_(std::move(other.wav_)), is_initialized_(other.is_initialized_) {
        other.is_initialized_ = false;
    }

    DrWavWrapper& DrWavWrapper::operator=(DrWavWrapper&& other) noexcept {
        if (this != &other) {
            wav_ = std::move(other.wav_);
            is_initialized_ = other.is_initialized_;
            other.is_initialized_ = false;
            close();
        }
        return *this;
    }

    WavError DrWavWrapper::save_packet_vector(const std::string& file_path, std::span<int16_t> pcm_data) {
        if (pcm_data.empty()) {
            return WavError::kEmptyData;
        }

        close();

        drwav_data_format format;
        format.container = drwav_container_riff;
        format.format = DR_WAVE_FORMAT_PCM;
        format.channels = kChannels;
        format.sampleRate = kSampleRate;
        format.bitsPerSample = kBitsPerSample;

        wav_ = std::make_unique<drwav>();

        if (drwav_init_file_write(wav_.get(), file_path.c_str(), &format, nullptr) == 0U) {
            return WavError::kFileOpenFailed;
        }
        
        is_initialized_ = true;

        drwav_write_pcm_frames(wav_.get(), pcm_data.size() / kChannels, pcm_data.data());
        
        close();
        
        return WavError::kSuccess;
    }

    void DrWavWrapper::close() {
        if (is_initialized_ && wav_) {
            drwav_uninit(wav_.get());
            is_initialized_ = false;
        }
    }
}  // namespace DrWav::Wrapper