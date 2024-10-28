#pragma once

#include "config.hpp"
#include <string>
#include <cpprest/http_client.h>

namespace deribit {

struct OrderParams {
    std::string instrument_name;
    double amount;
    double price;
    std::string type;  // "market" or "limit"
};

class OrderManager {
public:
    explicit OrderManager(Config& config);
    
    // Core functionality
    std::string place_buy_order(const OrderParams& params);
    std::string place_sell_order(const OrderParams& params);
    bool cancel_order(const std::string& order_id);
    bool modify_order(const std::string& order_id, double new_amount, double new_price);
    web::json::value get_positions(const std::string& currency, const std::string& kind);

private:
    Config& config_;
    web::http::client::http_client client_;
    
    web::http::http_request create_authenticated_request(
        web::http::method method,
        const std::string& path
    );
};

} // namespace deribit