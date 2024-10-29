#include "websocket_server.hpp"
#include <json/json.h>
#include <iostream>
#include <boost/beast/websocket.hpp>
#include <boost/asio/io_context.hpp>

namespace deribit {

WebsocketServer::WebsocketServer(Config& config)
    : config_(config)
{
    ws_server_.clear_access_channels(websocketpp::log::alevel::all);
    ws_server_.set_access_channels(websocketpp::log::alevel::connect);
    ws_server_.set_access_channels(websocketpp::log::alevel::disconnect);
    ws_server_.set_access_channels(websocketpp::log::alevel::app);

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

    init_deribit_connection();
}

WebsocketServer::~WebsocketServer() {
    stop();
}

void WebsocketServer::stop() {
    try {
        std::cout << "Stopping WebSocket server..." << std::endl;
        ws_server_.stop();
        if (server_thread_ && server_thread_->joinable()) {
            server_thread_->join();
        }

        std::cout << "Stopping Deribit WebSocket client..." << std::endl;
        deribit_client_.stop();
        if (deribit_thread_ && deribit_thread_->joinable()) {
            deribit_thread_->join();
        }
        std::cout << "WebSocket server and Deribit client stopped." << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error stopping WebSocket server or Deribit client: " << e.what() << std::endl;
    }
}

void WebsocketServer::on_message(connection_hdl hdl, message_ptr msg) {
    try {
        std::cout << "Received message from client: " << msg->get_payload() << std::endl;
        
        Json::Value json;
        Json::Reader reader;
        reader.parse(msg->get_payload(), json);
        
        std::string action = json["action"].asString();
        std::string symbol = json["symbol"].asString();
        
        if (action == "subscribe") {
            std::cout << "Client subscribing to symbol: " << symbol << std::endl;
            subscriptions_[hdl].insert(symbol);
            subscribe_to_orderbook(symbol);
        } else if (action == "unsubscribe") {
            std::cout << "Client unsubscribing from symbol: " << symbol << std::endl;
            subscriptions_[hdl].erase(symbol);
        }
    } catch (const std::exception& e) {
        std::cout << "Error processing message: " << e.what() << std::endl;
    }
}

void WebsocketServer::on_open(connection_hdl hdl) {
    std::cout << "New client connected" << std::endl;
    subscriptions_[hdl] = std::set<std::string>();
}

void WebsocketServer::on_close(connection_hdl hdl) {
    std::cout << "Client disconnected" << std::endl;
    subscriptions_.erase(hdl);
}

void WebsocketServer::subscribe_to_orderbook(const std::string& symbol) {
    if (!deribit_client_connection_) {
        std::cout << "No connection to Deribit" << std::endl;
        return;
    }

    Json::Value subscription;
    subscription["jsonrpc"] = "2.0";
    subscription["id"] = 42;
    subscription["method"] = "public/subscribe";
    subscription["params"]["channels"] = Json::arrayValue;
    subscription["params"]["channels"].append("book." + symbol + ".100ms");

    std::string message = Json::FastWriter().write(subscription);
    
    try {
        deribit_client_.send(deribit_client_connection_, message, websocketpp::frame::opcode::text);
        std::cout << "Subscribed to orderbook for " << symbol << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error subscribing to orderbook: " << e.what() << std::endl;
    }
}

void WebsocketServer::handle_orderbook_update(const std::string& symbol, const std::string& data) {
    // Broadcast the update to all clients subscribed to this symbol
    for (const auto& subscription : subscriptions_) {
        if (subscription.second.find(symbol) != subscription.second.end()) {
            try {
                ws_server_.send(subscription.first, data, websocketpp::frame::opcode::text);
            } catch (const std::exception& e) {
                std::cerr << "Error sending orderbook update to client: " << e.what() << std::endl;
            }
        }
    }
}

void WebsocketServer::init_deribit_connection() {
    try {
        // Create WebSocket connection to Deribit
        deribit_client_.init_asio();
        deribit_client_.set_access_channels(websocketpp::log::alevel::all);
        
        // Set up TLS connection
        deribit_client_.set_tcp_init_handler([](websocketpp::connection_hdl) {
            return websocketpp::lib::make_shared<boost::asio::ssl::context>(
                boost::asio::ssl::context::tlsv12_client
            );
        });

        // Connect to Deribit WebSocket
        websocketpp::lib::error_code ec;
        deribit_client_connection_ = deribit_client_.get_connection(
            config_.WS_URL,
            ec
        );

        if (ec) {
            std::cout << "Could not create connection: " << ec.message() << std::endl;
            return;
        }

        deribit_client_.connect(deribit_client_connection_);

        // Start the ASIO io_service run loop
        deribit_thread_ = websocketpp::lib::make_shared<websocketpp::lib::thread>([this]() {
            try {
                deribit_client_.run();
            } catch (const std::exception& e) {
                std::cout << "Deribit WebSocket error: " << e.what() << std::endl;
            }
        });
        
        std::cout << "Successfully connected to Deribit WebSocket" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Error initializing Deribit connection: " << e.what() << std::endl;
    }
}

void WebsocketServer::run(uint16_t port) {
    try {
        std::cout << "WebSocket server starting on port " << port << std::endl;
        ws_server_.listen(port);
        ws_server_.start_accept();
        
        server_thread_ = websocketpp::lib::make_shared<websocketpp::lib::thread>([this]() {
            try {
                ws_server_.run();
            } catch (const std::exception& e) {
                std::cout << "WebSocket server error: " << e.what() << std::endl;
            }
        });
        
        std::cout << "WebSocket server running" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Failed to start WebSocket server: " << e.what() << std::endl;
        throw;
    }
}

} // namespace deribit