#include "market_data.hpp"
#include <cpprest/uri_builder.h>
#include <iostream>

namespace deribit {

MarketData::MarketData(Config& config)
    : config_(config)
    , client_(config.BASE_URL)
{}

web::json::value MarketData::get_orderbook(const std::string& instrument_name, int depth) {
    web::uri_builder builder("/public/get_order_book");
    builder.append_query("instrument_name", instrument_name)
           .append_query("depth", depth);

    try {
        auto response = client_.request(web::http::methods::GET, builder.to_string()).get();
        if (response.status_code() == web::http::status_codes::OK) {
            std::cout << "Retrieved orderbook for instrument: " << instrument_name << std::endl;
            return response.extract_json().get();
        } else {
            std::cout << "Failed to get orderbook for instrument: " << instrument_name << ". Status code: " << response.status_code() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting orderbook for instrument " << instrument_name << ": " << e.what() << std::endl;
    }
    return web::json::value::null();
}

web::json::value MarketData::get_ticker(const std::string& instrument_name) {
    web::uri_builder builder("/public/ticker");
    builder.append_query("instrument_name", instrument_name);

    try {
        auto response = client_.request(web::http::methods::GET, builder.to_string()).get();
        if (response.status_code() == web::http::status_codes::OK) {
            std::cout << "Retrieved ticker for instrument: " << instrument_name << std::endl;
            return response.extract_json().get();
        } else {
            std::cout << "Failed to get ticker for instrument: " << instrument_name << ". Status code: " << response.status_code() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting ticker for instrument " << instrument_name << ": " << e.what() << std::endl;
    }
    return web::json::value::null();
}

web::json::value MarketData::get_instruments(const std::string& currency, const std::string& kind) {
    web::uri_builder builder("/public/get_instruments");
    builder.append_query("currency", currency)
           .append_query("kind", kind);

    try {
        auto response = client_.request(web::http::methods::GET, builder.to_string()).get();
        if (response.status_code() == web::http::status_codes::OK) {
            std::cout << "Retrieved instruments for currency: " << currency << ", kind: " << kind << std::endl;
            return response.extract_json().get();
        } else {
            std::cout << "Failed to get instruments for currency: " << currency << ", kind: " << kind << ". Status code: " << response.status_code() << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error getting instruments for currency " << currency << ", kind " << kind << ": " << e.what() << std::endl;
    }
    return web::json::value::null();
}

} // namespace deribit