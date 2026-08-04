#include "api/Listener.hpp"

int main() {
    boost::asio::io_context io_context;
    
    co_spawn(io_context, WebPTT::Api::Listener(io_context.get_executor()).listen(), boost::asio::detached);
    io_context.run();
    return 0;
}
