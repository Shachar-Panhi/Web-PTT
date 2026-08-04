#pragma once

#include <string>

namespace WebPTT::Api {
    struct MessageBody {
        std::string message_;
        std::string status_;
    };

    struct ErrorResponse {
        std::string message_;
        std::string status_;
    };
}  // namespace WebPTT::Api