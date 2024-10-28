#pragma once

#include "config.hpp"
#include <string>
#include <cpprest/http_client.h>

namespace deribit {

class Authentication {
public:
    explicit Authentication(Config& config);
    
    bool authenticate();
    std::string get_access_token() const;
    bool is_authenticated() const;
    void refresh_token();

private:
    Config& config_;
    bool is_authenticated_;
    web::http::client::http_client client_;
};

} // namespace deribit