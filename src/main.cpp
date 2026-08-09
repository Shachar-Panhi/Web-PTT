#include "api/Listener.hpp"
#include "api/UdpServer.hpp"

int main() {
    boost::asio::io_context io_context; 
    
    co_spawn(io_context, WebPTT::Api::Listener(io_context.get_executor()).listen(), boost::asio::detached);
    co_spawn(io_context, WebPTT::UDP::UdpServer(io_context).start(), boost::asio::detached);
    
    io_context.run();
    return 0;
}
