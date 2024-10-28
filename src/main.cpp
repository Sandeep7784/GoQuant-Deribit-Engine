#include "config.hpp"
#include "authentication.hpp"
#include "order_manager.hpp"
#include "market_data.hpp"
#include "websocket_server.hpp"
#include <iostream>
#include <fstream>
#include <json/json.h>

// Function to load configuration
deribit::Config load_config(const std::string &config_path)
{
    std::ifstream config_file(config_path);
    if (!config_file.is_open())
    {
        throw std::runtime_error("Unable to open config file");
    }

    Json::Value root;
    Json::Reader reader;
    if (!reader.parse(config_file, root))
    {
        throw std::runtime_error("Failed to parse config file");
    }

    return deribit::Config(
        root["api_credentials"]["client_id"].asString(),
        root["api_credentials"]["client_secret"].asString());
}

int main(int argc, char *argv[])
{
    try
    {
        // Load configuration
        auto config = load_config("config/config.json");

        // Initialize components
        deribit::Authentication auth(config);
        if (!auth.authenticate())
        {
            std::cerr << "Authentication failed" << std::endl;
            return 1;
        }
        std::cout << "Successfully authenticated" << std::endl;

        deribit::OrderManager order_manager(config);
        deribit::MarketData market_data(config);

        // Initialize WebSocket server
        deribit::WebsocketServer ws_server(config);
        ws_server.run(8080); // Start WebSocket server on port 8080

        // Example usage
        std::cout << "\nAvailable commands:" << std::endl;
        std::cout << "1. Place buy order" << std::endl;
        std::cout << "2. Place sell order" << std::endl;
        std::cout << "3. Cancel order" << std::endl;
        std::cout << "4. Modify order" << std::endl;
        std::cout << "5. Get positions" << std::endl;
        std::cout << "6. Get orderbook" << std::endl;
        std::cout << "7. Exit" << std::endl;

        std::string command;
        while (true)
        {
            std::cout << "\nEnter command (1-7): ";
            std::getline(std::cin, command);

            if (command == "1")
            {
                // Example: Place buy order
                deribit::OrderParams params{
                    "BTC-PERPETUAL", // instrument_name
                    20,              // amount
                    50000.0,         // price
                    "limit"          // type
                };

                std::string order_id = order_manager.place_buy_order(params);
                std::cout << "Order placed with ID: " << order_id << std::endl;
            }
            else if (command == "2")
            {
                // Example: Place sell order
                deribit::OrderParams params{
                    "BTC-PERPETUAL",
                    10,
                    70000.0,
                    "limit"};
                std::string order_id = order_manager.place_sell_order(params);
                std::cout << "Order placed with ID: " << order_id << std::endl;
            }
            else if (command == "3")
            {
                // Cancel order
                std::string order_id;
                std::cout << "Enter order ID to cancel: ";
                std::getline(std::cin, order_id);
                bool success = order_manager.cancel_order(order_id);
                std::cout << (success ? "Order cancelled" : "Failed to cancel order") << std::endl;
            }
            else if (command == "4")
            {
                // Modify order
                std::string order_id;
                double new_amount, new_price;
                std::cout << "Enter order ID: ";
                std::getline(std::cin, order_id);
                std::cout << "Enter new amount: ";
                std::cin >> new_amount;
                std::cout << "Enter new price: ";
                std::cin >> new_price;
                std::cin.ignore();

                bool success = order_manager.modify_order(order_id, new_amount, new_price);
                std::cout << (success ? "Order modified" : "Failed to modify order") << std::endl;
            }
            else if (command == "5") {
                // Get positions
                auto positions = order_manager.get_positions("BTC", "future");
                std::cout << positions.serialize() << std::endl;
            }
            else if (command == "6") {
                // Get orderbook
                auto orderbook = market_data.get_orderbook("BTC-PERPETUAL", 10);
                std::cout << orderbook.serialize() << std::endl;
            }
            else if (command == "7") {
                break;
            }
        }

        // Cleanup
        ws_server.stop();
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
