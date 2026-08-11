#include "api/Listener.hpp"
#include "api/UdpServer.hpp"

int main() {
    boost::asio::io_context io_context; 
    
    WebPTT::Api::Listener listener(io_context.get_executor());
    co_spawn(io_context, listener.listen(), boost::asio::detached);
    
    WebPTT::UDP::UdpServer server(io_context.get_executor());
    co_spawn(io_context, server.start(), boost::asio::detached);
    
    io_context.run();
    return 0;
}
