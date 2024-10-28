#pragma once

#include "config.hpp"
#include <cpprest/http_client.h>
#include <string>

namespace deribit {

class MarketData {
public:
    explicit MarketData(Config& config);
    
    web::json::value get_orderbook(const std::string& instrument_name, int depth);
    web::json::value get_ticker(const std::string& instrument_name);
    web::json::value get_instruments(const std::string& currency, const std::string& kind);

private:
    Config& config_;
    web::http::client::http_client client_;
};

} // namespace deribit