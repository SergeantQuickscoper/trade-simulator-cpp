#include "websocketClient.hpp"
#include "orderbook.hpp"
#include "simulator.hpp"
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <nlohmann/json.hpp>
#include <iomanip>

using json = nlohmann::json;

void printMetrics(const DetailedTradeMetrics& metrics) {
    std::cout << "\n=== Trade Metrics ===\n";
    std::cout << std::fixed << std::setprecision(8);
    
    // Expected costs
    std::cout << "Expected Slippage: " << metrics.expectedSlippage << "\n";
    std::cout << "Expected Fees: " << metrics.expectedFees << "\n";
    std::cout << "Expected Market Impact: " << metrics.expectedMarketImpact << "\n";
    std::cout << "Net Cost: " << metrics.netCost << "\n\n";
    
    // Execution metrics
    std::cout << "Maker/Taker Ratio: " << (metrics.makerTakerRatio * 100) << "%\n";
    std::cout << "Internal Latency: " << metrics.internalLatency << " ms\n\n";
    
    // Market conditions
    std::cout << "Current Spread: " << metrics.currentSpread << "\n";
    std::cout << "Mid Price: " << metrics.midPrice << "\n";
    std::cout << "Order Book Imbalance: " << (metrics.orderBookImbalance * 100) << "%\n\n";
    
}

int main() {
    WebSocketClient client;
    OrderBook orderbook("OKX", "BTC-USDT-SWAP");
    Simulator simulator;
    
    // Initialize simulator with $100,000 capital
    simulator.initialize("OKX", "BTC-USDT-SWAP", 100000.0);

    // Set up message handler
    client.setMessageHandler([&orderbook, &simulator](const std::string& message) {
        try {
            // Parse the JSON message
            auto data = json::parse(message);
            
            // Extract orderbook data
            std::string timestamp = data["timestamp"];
            auto asks = data["asks"].get<std::vector<std::vector<std::string>>>();
            auto bids = data["bids"].get<std::vector<std::vector<std::string>>>();
            
            // Convert to the format expected by OrderBook
            std::vector<std::pair<std::string, std::string>> askLevels;
            std::vector<std::pair<std::string, std::string>> bidLevels;
            
            for (const auto& ask : asks) {
                askLevels.emplace_back(ask[0], ask[1]);
            }
            
            for (const auto& bid : bids) {
                bidLevels.emplace_back(bid[0], bid[1]);
            }
            
            // Update the orderbook
            orderbook.update(timestamp, askLevels, bidLevels);
            
            auto bestBid = orderbook.getBestBid();
            auto bestAsk = orderbook.getBestAsk();
            // 
            std::cout << "----- Orderbook Bests----- " << std::endl;
            std::cout << "Best Bid: " << bestBid->price << std::endl;
            std::cout << "Best Ask: " << bestAsk->price << std::endl;
            std::cout << "-------------------------- " << std::endl;

            // Update simulator with new market data
            simulator.updateMarketData(orderbook);
            
            // Simulate a sample trade and get metrics
            // Example: Buy 0.1 BTC at market
            auto metrics = simulator.calculateTradeMetrics(
                0.0000096,                // Order size
                0.0,               // Market order (no limit price)
                "market",          // Order type
                orderbook,         // Current orderbook
                60.0              // 1-minute time horizon
            );
            
            // Print the metrics
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
    std::string host = "ws.gomarket-cpp.goquant.io";
    std::string port = "443";
    std::string path = "/ws/l2-orderbook/okx/BTC-USDT-SWAP";
    
    std::cout << "Connecting to " << host << ":" << port << path << std::endl;
    client.connect(host, port, path);

    // Keep the main thread alive for a while
    std::this_thread::sleep_for(std::chrono::seconds(30));

    // Clean up
    client.close();

    return 0;
}
