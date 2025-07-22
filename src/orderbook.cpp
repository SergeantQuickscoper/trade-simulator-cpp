#include "orderbook.hpp"
#include <sstream>
#include <iomanip>
#include <ctime>
#include <algorithm>

OrderBook::OrderBook(const std::string& exchange, const std::string& symbol)
    : exchange_(exchange), symbol_(symbol) {}

void OrderBook::update(const std::string& timestamp,
                      const std::vector<std::pair<std::string, std::string>>& asks,
                      const std::vector<std::pair<std::string, std::string>>& bids) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    lastUpdateTime_ = parseTimestamp(timestamp);
    
    updateSide(asks_, asks);
    updateSide(bids_, bids);
}

void OrderBook::updateSide(PriceLevels& side, const std::vector<std::pair<std::string, std::string>>& levels) {
    side.clear();
    
    for(const auto& level : levels) {
        double price = parsePrice(level.first);
        double quantity = parseQuantity(level.second);
        if(quantity > 0){ 
             // Only insert non-zero quantities
            side[price] = quantity;
        }
    }
}

std::optional<PriceLevel> OrderBook::getBestAsk() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (asks_.empty()) return std::nullopt;
    auto it = asks_.begin();
    return PriceLevel(it->first, it->second);
}

std::optional<PriceLevel> OrderBook::getBestBid() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (bids_.empty()) return std::nullopt;
    auto it = bids_.begin();
    return PriceLevel(it->first, it->second);
}

std::vector<PriceLevel> OrderBook::getAsksAtDepth(size_t depth) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<PriceLevel> result;
    result.reserve(std::min(depth, asks_.size()));
    
    auto it = asks_.begin();
    for (size_t i = 0; i < depth && it != asks_.end(); ++i, ++it) {
        result.emplace_back(it->first, it->second);
    }
    return result;
}

std::vector<PriceLevel> OrderBook::getBidsAtDepth(size_t depth) const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<PriceLevel> result;
    result.reserve(std::min(depth, bids_.size()));
    
    auto it = bids_.begin();
    for (size_t i = 0; i < depth && it != bids_.end(); ++i, ++it) {
        result.emplace_back(it->first, it->second);
    }
    return result;
}

double OrderBook::getMidPrice() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (asks_.empty() || bids_.empty()) return 0.0;
    
    auto bestAsk = asks_.begin();
    auto bestBid = bids_.begin();
    
    return (bestAsk->first + bestBid->first) / 2.0;
}

double OrderBook::getSpread() const {
    std::lock_guard<std::mutex> lock(mutex_);
    if (asks_.empty() || bids_.empty()) return 0.0;
    
    auto bestAsk = asks_.begin();
    auto bestBid = bids_.begin();
    
    return bestAsk->first - bestBid->first;
}

double OrderBook::getVolumeAtPrice(double price) const {
    std::lock_guard<std::mutex> lock(mutex_);
    
    // Check asks
    auto askIt = asks_.find(price);
    if (askIt != asks_.end()) {
        return askIt->second;
    }
    
    // Check bids
    auto bidIt = bids_.find(price);
    if (bidIt != bids_.end()) {
        return bidIt->second;
    }
    
    return 0.0;
}

double OrderBook::getVolumeBetweenPrices(double lowerPrice, double upperPrice) const {
    std::lock_guard<std::mutex> lock(mutex_);
    double totalVolume = 0.0;
    
    //Sum ask volume
    for (const auto& [price, quantity] : asks_) {
        if (price >= lowerPrice && price <= upperPrice) {
            totalVolume += quantity;
        }
    }
    
    //Sum bid volume
    for (const auto& [price, quantity] : bids_) {
        if (price >= lowerPrice && price <= upperPrice) {
            totalVolume += quantity;
        }
    }
    
    return totalVolume;
}

double OrderBook::getBidVolume() const {
    std::lock_guard<std::mutex> lock(mutex_);
    double total = 0.0;
    for (const auto& [price, quantity] : bids_) {
        total += quantity;
    }
    return total;
}

double OrderBook::getAskVolume() const {
    std::lock_guard<std::mutex> lock(mutex_);
    double total = 0.0;
    for (const auto& [price, quantity] : asks_) {
        total += quantity;
    }
    return total;
}

double OrderBook::parsePrice(const std::string& price) {
    return std::stod(price);
}

double OrderBook::parseQuantity(const std::string& quantity) {
    return std::stod(quantity);
}

OrderBook::Timestamp OrderBook::parseTimestamp(const std::string& timestamp) {
    std::tm tm = {};
    std::stringstream ss(timestamp);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%SZ");
    
    auto time = std::chrono::system_clock::from_time_t(std::mktime(&tm));
    return time;
} 