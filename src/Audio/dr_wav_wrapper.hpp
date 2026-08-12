#include "../../ThirdParty/dr_wav.h"
#include <string>
#include <span>
#include <memory>


namespace DrWav::Wrapper {

    enum class WavError : uint8_t{
        kSuccess,
        kFileOpenFailed,
        kEmptyData
    };

    class DrWavWrapper {
    public:
        DrWavWrapper();
        
        ~DrWavWrapper();
        
        DrWavWrapper(const DrWavWrapper&) = delete;
        DrWavWrapper& operator=(const DrWavWrapper&) = delete;
        
        DrWavWrapper(DrWavWrapper&& other) noexcept;
        DrWavWrapper& operator=(DrWavWrapper&& other) noexcept;

        WavError save_packet_vector(const std::string& file_path, std::span<int16_t> pcm_data);

    private:
        void close();

        std::unique_ptr<drwav> wav_;
        bool is_initialized_{false};
    };
    


}  // namespace DrWav::Wrapper