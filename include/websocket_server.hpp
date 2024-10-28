#pragma once

#define ASIO_STANDALONE
#include "config.hpp"
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <map>
#include <set>
#include <memory>
#include <functional>

namespace deribit {

class WebsocketServer {
public:
    using server = websocketpp::server<websocketpp::config::asio>;
    using connection_hdl = websocketpp::connection_hdl;
    using message_ptr = server::message_ptr;
    using context_ptr = std::shared_ptr<void>;

    explicit WebsocketServer(Config& config);
    void run(uint16_t port);
    void stop();

private:
    void on_message(connection_hdl hdl, message_ptr msg);
    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void subscribe_to_orderbook(const std::string& symbol);
    void handle_orderbook_update(const std::string& symbol, const std::string& data);

    Config& config_;
    server ws_server_;
    std::map<connection_hdl, std::set<std::string>, std::owner_less<connection_hdl>> subscriptions_;
    std::shared_ptr<std::thread> server_thread_;
    bool is_running_;
};

} // namespace deribit