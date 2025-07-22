#pragma once

#include <string>
#include <map>
#include <vector>
#include <mutex>
#include <chrono>
#include <optional>
#include <memory>

struct PriceLevel {
    double price;
    double quantity;
    
    PriceLevel(double p, double q) : price(p), quantity(q) {}
};

class OrderBook {
public:
    using PriceLevels = std::map<double, double, std::greater<double>>; // price -> quantity
    using Timestamp = std::chrono::system_clock::time_point;
    
    OrderBook(const std::string& exchange, const std::string& symbol);
    
    // Update the orderbook with new data
    void update(const std::string& timestamp, 
               const std::vector<std::pair<std::string, std::string>>& asks,
               const std::vector<std::pair<std::string, std::string>>& bids);
    
    // Get current top of book
    std::optional<PriceLevel> getBestAsk() const;
    std::optional<PriceLevel> getBestBid() const;
    
    // Get price levels at a specific depth
    std::vector<PriceLevel> getAsksAtDepth(size_t depth) const;
    std::vector<PriceLevel> getBidsAtDepth(size_t depth) const;
    
    // Get full orderbook state
    const PriceLevels& getAsks() const { return asks_; }
    const PriceLevels& getBids() const { return bids_; }
    
    // Get last update timestamp
    Timestamp getLastUpdateTime() const { return lastUpdateTime_; }
    
    // Get exchange and symbol
    const std::string& getExchange() const { return exchange_; }
    const std::string& getSymbol() const { return symbol_; }
    
    // Get mid price
    double getMidPrice() const;
    
    // Get spread
    double getSpread() const;
    
    // Get total volume at a specific price level
    double getVolumeAtPrice(double price) const;
    
    // Get total volume between two price levels
    double getVolumeBetweenPrices(double lowerPrice, double upperPrice) const;

    // Get total bid and ask volume
    double getBidVolume() const;
    double getAskVolume() const;

private:
    std::string exchange_;
    std::string symbol_;
    PriceLevels asks_;  // Sorted by price (ascending)
    PriceLevels bids_;  // Sorted by price (descending)
    Timestamp lastUpdateTime_;
    mutable std::mutex mutex_;  // For thread safety
    
    // Helper functions
    void updateSide(PriceLevels& side, const std::vector<std::pair<std::string, std::string>>& levels);
    static double parsePrice(const std::string& price);
    static double parseQuantity(const std::string& quantity);
    static Timestamp parseTimestamp(const std::string& timestamp);
}; 