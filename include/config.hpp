#pragma once

#include <string>

namespace deribit {

struct Config {
    static constexpr const char* BASE_URL = "https://test.deribit.com/api/v2";
    static constexpr const char* WS_URL = "wss://test.deribit.com/ws/api/v2";
    
    std::string client_id;
    std::string client_secret;
    std::string access_token;
    
    Config(const std::string& id, const std::string& secret)
        : client_id(id), client_secret(secret) {}
};

} // namespace deribit