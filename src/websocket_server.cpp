#include "websocket_server.hpp"
#include <json/json.h>

namespace deribit {

WebsocketServer::WebsocketServer(Config& config)
    : config_(config)
{
    ws_server_.init_asio();
    
    ws_server_.set_message_handler(
        std::bind(&WebsocketServer::on_message, this, std::placeholders::_1, std::placeholders::_2)
    );
    
    ws_server_.set_open_handler(
        std::bind(&WebsocketServer::on_open, this, std::placeholders::_1)
    );
    
    ws_server_.set_close_handler(
        std::bind(&WebsocketServer::on_close, this, std::placeholders::_1)
    );
}

void WebsocketServer::run(uint16_t port) {
    ws_server_.listen(port);
    ws_server_.start_accept();
    
    server_thread_ = websocketpp::lib::make_shared<websocketpp::lib::thread>([this]() {
        try {
            ws_server_.run();
        } catch (const std::exception& e) {
            // Handle error
        }
    });
}

void WebsocketServer::stop() {
    ws_server_.stop();
    if (server_thread_ && server_thread_->joinable()) {
        server_thread_->join();
    }
}

void WebsocketServer::on_message(connection_hdl hdl, message_ptr msg) {
    try {
        Json::Value json;
        Json::Reader reader;
        reader.parse(msg->get_payload(), json);
        
        std::string action = json["action"].asString();
        std::string symbol = json["symbol"].asString();
        
        if (action == "subscribe") {
            subscriptions_[hdl].insert(symbol);
            subscribe_to_orderbook(symbol);
        } else if (action == "unsubscribe") {
            subscriptions_[hdl].erase(symbol);
        }
    } catch (const std::exception& e) {
        // Handle error
    }
}

void WebsocketServer::on_open(connection_hdl hdl) {
    subscriptions_[hdl] = std::set<std::string>();
}

void WebsocketServer::on_close(connection_hdl hdl) {
    subscriptions_.erase(hdl);
}

void WebsocketServer::subscribe_to_orderbook(const std::string& symbol) {
    // Connect to Deribit WebSocket and subscribe to orderbook updates
    // This is a simplified version - you'll need to implement the actual
    // Deribit WebSocket client connection and subscription logic
}

void WebsocketServer::handle_orderbook_update(const std::string& symbol, const std::string& data) {
    // Broadcast the update to all clients subscribed to this symbol
    for (const auto& subscription : subscriptions_) {
        if (subscription.second.find(symbol) != subscription.second.end()) {
            try {
                ws_server_.send(subscription.first, data, websocketpp::frame::opcode::text);
            } catch (const std::exception& e) {
                // Handle error
            }
        }
    }
}

} // namespace deribit