#include "websocketClient.hpp"
#include "orderbook.hpp"
#include "simulator.hpp"
#include <iostream>
#include <fstream>
#include <map>
#include <string>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>
#include <iomanip>

using json = nlohmann::json;

// parse env
std::map<std::string, std::string> load_env(const std::string& filepath = ".env") {
    std::map<std::string, std::string> env;
    std::ifstream file(filepath);
    std::string line;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        std::string key, value;
        if (std::getline(iss, key, '=') && std::getline(iss, value)) {
            env[key] = value;
        }
    }

    return env;
}

void printMetrics(const DetailedTradeMetrics& metrics) {
    std::cout << "\n=== Trade Metrics ===\n";
    std::cout << std::fixed << std::setprecision(8);
    
    // Output metrics
    std::cout << "Expected Slippage: " << metrics.expectedSlippage << "\n";
    std::cout << "Expected Fees: " << metrics.expectedFees << "\n";
    std::cout << "Expected Market Impact: " << metrics.expectedMarketImpact << "\n";
    std::cout << "Net Cost: " << metrics.netCost << "\n\n";
    std::cout << "Maker/Taker Ratio: " << (metrics.makerTakerRatio * 100) << "%\n";
    std::cout << "Internal Latency: " << metrics.internalLatency << " ms\n\n";
    std::cout << "Current Spread: " << metrics.currentSpread << "\n";
    std::cout << "Mid Price: " << metrics.midPrice << "\n";
    std::cout << "Order Book Imbalance: " << (metrics.orderBookImbalance * 100) << "%\n\n";
    
}

int main() {
    auto env = load_env();
    WebSocketClient client;
    
    std::string exchange = env["EXCHANGE"];
    std::string symbol   = env["SYMBOL"];
    double initial_capital = std::stod(env["INITIAL_CAPITAL"]);

    OrderBook orderbook(exchange, symbol);
    Simulator simulator;

    simulator.initialize(exchange, symbol, initial_capital);

    // Set up message handler
    client.setMessageHandler([&orderbook, &simulator](const std::string& message) {
        try {
            auto data = json::parse(message);
            
            // Extracting orderbook data
            std::string timestamp = data["timestamp"];
            auto asks = data["asks"].get<std::vector<std::vector<std::string>>>();
            auto bids = data["bids"].get<std::vector<std::vector<std::string>>>();
            std::vector<std::pair<std::string, std::string>> askLevels;
            std::vector<std::pair<std::string, std::string>> bidLevels;
            
            for (const auto& ask : asks){
                askLevels.emplace_back(ask[0], ask[1]);
            }
            
            for (const auto& bid : bids){
                bidLevels.emplace_back(bid[0], bid[1]);
            }
            
            // Update the orderbook
            orderbook.update(timestamp, askLevels, bidLevels);
            auto bestBid = orderbook.getBestBid();
            auto bestAsk = orderbook.getBestAsk();
            std::cout << "----- Orderbook Bests----- " << std::endl;
            std::cout << "Best Bid: " << bestBid->price << std::endl;
            std::cout << "Best Ask: " << bestAsk->price << std::endl;
            std::cout << "-------------------------- " << std::endl;

            simulator.updateMarketData(orderbook);
            
            // Example: Buy 0.1 BTC at market
            auto metrics = simulator.calculateTradeMetrics(
                0.0000096,                // Order size
                0.0,               // Market order (no limit price)
                "market",          // Order type
                orderbook,         // Current orderbook
                60.0              // 1-minute time horizon
            );
            printMetrics(metrics);
            
        } catch (const std::exception& e) {
            std::cerr << "Error processing message: " << e.what() << std::endl;
        }
    });

    // Set up connection handler
    client.setConnectionHandler([]() {
        std::cout << "Connected to WebSocket server" << std::endl;
    });

    // Connect to a WebSocket server 
    std::string host = env["HOST"];
    std::string port = env["PORT"];
    std::string path = env["PATH"];
    
    std::cout << "Connecting to " << host << ":" << port << path << std::endl;
    client.connect(host, port, path);

    // Keep the main thread alive for a while
    std::this_thread::sleep_for(std::chrono::seconds(30));

    // Clean up
    client.close();

    return 0;
}
