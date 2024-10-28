#include "market_data.hpp"
#include <cpprest/uri_builder.h>

namespace deribit {

MarketData::MarketData(Config& config)
    : config_(config)
    , client_(config.BASE_URL)
{}

web::json::value MarketData::get_orderbook(const std::string& instrument_name, int depth) {
    web::uri_builder builder("/public/get_order_book");
    builder.append_query("depth", depth)
           .append_query("instrument_name", instrument_name);

    try {
        auto response = client_.request(web::http::methods::GET, builder.to_string()).get();
        // std::cout << "Response: " << response.to_string() << std::endl;
        if (response.status_code() == web::http::status_codes::OK) {
            return response.extract_json().get();
        }
    } catch (const std::exception& e) {
        // Handle error
        std::cout << e.what() << std::endl; 
    }
    return web::json::value::null();
}

web::json::value MarketData::get_ticker(const std::string& instrument_name) {
    web::uri_builder builder("/public/ticker");
    builder.append_query("instrument_name", instrument_name);

    try {
        auto response = client_.request(web::http::methods::GET, builder.to_string()).get();
        if (response.status_code() == web::http::status_codes::OK) {
            return response.extract_json().get();
        }
    } catch (const std::exception& e) {
        // Handle error
        std::cout << e.what() << std::endl; 
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
            return response.extract_json().get();
        }
    } catch (const std::exception& e) {
        // Handle error
        std::cout << e.what() << std::endl; 
    }
    return web::json::value::null();
}

} // namespace deribit