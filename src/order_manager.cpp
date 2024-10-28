#include "order_manager.hpp"
#include <cpprest/json.h>

namespace deribit {

OrderManager::OrderManager(Config& config)
    : config_(config)
    , client_(config.BASE_URL)
{}

web::http::http_request OrderManager::create_authenticated_request(
    web::http::method method,
    const std::string& path
) {
    web::http::http_request request(method);
    request.headers().add("Authorization", "Bearer " + config_.access_token);
    request.set_request_uri(path);
    // std::cout << "access token: " << config_.access_token << std::endl;
    return request;
}

std::string OrderManager::place_buy_order(const OrderParams& params) {
    web::uri_builder builder("/private/buy");
    builder.append_query("amount", params.amount)
           .append_query("instrument_name", params.instrument_name)
           .append_query("type", params.type);
    
    if (params.type == "limit") {
        builder.append_query("price", params.price);
    }

    auto request = create_authenticated_request(web::http::methods::GET, builder.to_string());
    // std::cout << "Request: " << request.to_string() << std::endl;
    try {
        auto response = client_.request(request).get();
        // std::cout << response.to_string() << std::endl;
        if (response.status_code() == web::http::status_codes::OK) {
            auto json = response.extract_json().get();
            return json["result"]["order"]["order_id"].as_string();
        }
    } catch (const std::exception& e) {
        // Handle error
        std::cout << e.what() << std::endl; 
    }
    return "";
}

// https://test.deribit.com/api/v2/private/sell?advanced=usd&amount=10&instrument_name=BTC-PERPETUAL&otoco_config[]=%5Bobject%20Object%5D&price=66610&type=limit

std::string OrderManager::place_sell_order(const OrderParams& params) {
    web::uri_builder builder("/private/sell");
    builder.append_query("advanced", "usd")
        .append_query("amount", params.amount)
           .append_query("instrument_name", params.instrument_name);
               
    if (params.type == "limit") {
        builder.append_query("price", params.price);
    }

    builder.append_query("type", params.type);

    auto request = create_authenticated_request(web::http::methods::GET, builder.to_string());
    // std::cout << "Request: " << request.to_string() << std::endl;

    try {
        auto response = client_.request(request).get();
        // std::cout << response.to_string() << std::endl;
        if (response.status_code() == web::http::status_codes::OK) {
            auto json = response.extract_json().get();
            return json["result"]["order"]["order_id"].as_string();
        }
    } catch (const std::exception& e) {
        // Handle error
        std::cout << e.what() << std::endl; 
    }
    return "";
}

bool OrderManager::cancel_order(const std::string& order_id) {
    web::uri_builder builder("/private/cancel");
    builder.append_query("order_id", order_id);

    auto request = create_authenticated_request(web::http::methods::GET, builder.to_string());
    
    try {
        auto response = client_.request(request).get();
        return response.status_code() == web::http::status_codes::OK;
    } catch (const std::exception& e) {
        // Handle error
        std::cout << e.what() << std::endl; 
    }
    return false;
}

bool OrderManager::modify_order(const std::string& order_id, double new_amount, double new_price) {
    web::uri_builder builder("/private/edit");
    builder.append_query("order_id", order_id)
           .append_query("amount", new_amount)
           .append_query("price", new_price);

    auto request = create_authenticated_request(web::http::methods::GET, builder.to_string());
    
    try {
        auto response = client_.request(request).get();
        return response.status_code() == web::http::status_codes::OK;
    } catch (const std::exception& e) {
        // Handle error
        std::cout << e.what() << std::endl; 
    }
    return false;
}

web::json::value OrderManager::get_positions(const std::string& currency, const std::string& kind) {
    web::uri_builder builder("/private/get_positions");
    builder.append_query("currency", currency)
           .append_query("kind", kind);

    auto request = create_authenticated_request(web::http::methods::GET, builder.to_string());
    
    try {
        auto response = client_.request(request).get();
        if (response.status_code() == web::http::status_codes::OK) {
            return response.extract_json().get();
        }
    } catch (const std::exception& e) {
        // Handle error
        std::cout << e.what() << std::endl; 
    }
    return web::json::value::null();
}

} // namespace deribit