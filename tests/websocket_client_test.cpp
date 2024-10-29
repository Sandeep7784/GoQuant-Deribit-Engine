#include <websocketpp/client.hpp>
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <iostream>
#include <json/json.h>

using client = websocketpp::client<websocketpp::config::asio_client>;
using message_ptr = websocketpp::config::asio_client::message_type::ptr;

class WebsocketClient {
public:
    WebsocketClient() {
        client_.clear_access_channels(websocketpp::log::alevel::all);
        client_.set_access_channels(websocketpp::log::alevel::connect);
        client_.set_access_channels(websocketpp::log::alevel::disconnect);
        client_.set_access_channels(websocketpp::log::alevel::app);

        client_.init_asio();

        client_.set_message_handler([this](websocketpp::connection_hdl hdl, message_ptr msg) {
            on_message(hdl, msg);
        });

        client_.set_open_handler([this](websocketpp::connection_hdl hdl) {
            on_open(hdl);
        });
    }

    void connect(const std::string& uri) {
        websocketpp::lib::error_code ec;
        connection_ = client_.get_connection(uri, ec);
        
        if (ec) {
            std::cout << "Could not create connection: " << ec.message() << std::endl;
            return;
        }

        client_.connect(connection_);
        client_.run();
    }

    void subscribe_to_symbol(const std::string& symbol) {
        Json::Value request;
        request["action"] = "subscribe";
        request["symbol"] = symbol;

        std::string message = Json::FastWriter().write(request);
        
        try {
            client_.send(connection_, message, websocketpp::frame::opcode::text);
            std::cout << "Sent subscription request for " << symbol << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error sending subscription: " << e.what() << std::endl;
        }
    }

private:
    void on_message(websocketpp::connection_hdl hdl, message_ptr msg) {
        std::cout << "Received message: " << msg->get_payload() << std::endl;
    }

    void on_open(websocketpp::connection_hdl hdl) {
        std::cout << "Connection established" << std::endl;
        subscribe_to_symbol("BTC-PERPETUAL");
    }

    client client_;
    client::connection_ptr connection_;
};

int main() {
    try {
        WebsocketClient client;
        std::cout << "Connecting to WebSocket server..." << std::endl;
        client.connect("ws://localhost:8080");
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    return 0;
}