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

    // Read the supported instruments into a vector
    std::vector<std::string> supported_instruments;
    for (const auto &instrument : root["trading"]["supported_instruments"])
    {
        supported_instruments.push_back(instrument.asString());
    }

    return deribit::Config(
        root["api_credentials"]["client_id"].asString(),
        root["api_credentials"]["client_secret"].asString(),
        root["server"]["websocket_port"].asInt(),
        root["trading"]["default_currency"].asString(),
        root["trading"]["default_instrument"].asString(), // Add this line
        supported_instruments);
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
        ws_server.run(config.server.websocket_port); // Start WebSocket server on configured port
        std::cout << "WebSocket server started on port " << config.server.websocket_port << std::endl;

        // Interactive console
        std::string command;
        while (true)
        {
            std::cout << "\nAvailable commands:" << std::endl;
            std::cout << "1. Place buy order" << std::endl;
            std::cout << "2. Place sell order" << std::endl;
            std::cout << "3. Cancel order" << std::endl;
            std::cout << "4. Modify order" << std::endl;
            std::cout << "5. Get positions" << std::endl;
            std::cout << "6. Get orderbook" << std::endl;
            std::cout << "7. Get ticker" << std::endl;
            std::cout << "8. Get instruments" << std::endl;
            std::cout << "9. Exit" << std::endl;

            std::cout << "\nEnter command (1-9): ";
            std::getline(std::cin, command);

            if (command == "1")
            {
                deribit::OrderParams params{
                    config.trading.default_instrument,
                    10,
                    60000.0,
                    "limit"};
                std::string order_id = order_manager.place_buy_order(params);
                std::cout << "Buy order placed successfully. Order ID: " << order_id << std::endl;
            }
            else if (command == "2")
            {
                deribit::OrderParams params{
                    config.trading.default_instrument,
                    10,
                    75000.0,
                    "limit"};
                std::string order_id = order_manager.place_sell_order(params);
                std::cout << "Sell order placed successfully. Order ID: " << order_id << std::endl;
            }
            else if (command == "3")
            {
                std::string order_id;
                std::cout << "Enter order ID to cancel: ";
                std::getline(std::cin, order_id);
                bool success = order_manager.cancel_order(order_id);
                if (success)
                {
                    std::cout << "Order ID " << order_id << " canceled successfully." << std::endl;
                }
                else
                {
                    std::cout << "Failed to cancel order ID " << order_id << "." << std::endl;
                }
            }
            else if (command == "4")
            {
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
                if (success)
                {
                    std::cout << "Order ID " << order_id << " modified successfully." << std::endl;
                }
                else
                {
                    std::cout << "Failed to modify order ID " << order_id << "." << std::endl;
                }
            }
            else if (command == "5")
            {
                auto positions = order_manager.get_positions(config.trading.default_currency, "future");
                std::cout << "Retrieved positions:" << std::endl;
                std::cout << positions.serialize() << std::endl;
            }
            else if (command == "6")
            {
                std::cout << "Enter instrument name: ";
                std::string instrument_name;
                std::getline(std::cin, instrument_name);
                auto orderbook = market_data.get_orderbook(instrument_name, 10);
                std::cout << "Retrieved orderbook for instrument: " << instrument_name << std::endl;
                std::cout << orderbook.serialize() << std::endl;
            }
            else if (command == "7")
            {
                std::cout << "Enter instrument name: ";
                std::string instrument_name;
                std::getline(std::cin, instrument_name);
                auto ticker = market_data.get_ticker(instrument_name);
                std::cout << "Retrieved ticker for instrument: " << instrument_name << std::endl;
                std::cout << ticker.serialize() << std::endl;
            }
            else if (command == "8")
            {
                auto instruments = market_data.get_instruments(config.trading.default_currency, "future");
                std::cout << "Retrieved instruments for currency: " << config.trading.default_currency << std::endl;
                std::cout << instruments.serialize() << std::endl;
            }
            else if (command == "9")
            {
                break;
            }
        }

        // Cleanup
        ws_server.stop();
        return 0;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}