#include "api/Listener.hpp"
#include "Audio/UdpServer.hpp"

int main() {
    boost::asio::io_context io_context; 

    auto shared_audio = std::make_shared<WebPTT::Audio::AudioData>();
    
    WebPTT::Api::Listener listener(io_context.get_executor(), shared_audio);
    co_spawn(io_context, listener.listen(), boost::asio::detached);
    
    WebPTT::Audio::UdpServer server(io_context.get_executor(), shared_audio);
    co_spawn(io_context, server.start(), boost::asio::detached);
    
    io_context.run();
    return 0;
}
