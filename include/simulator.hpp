#pragma once

#include <string>
#include <memory>
#include <chrono>
#include "slippageModel.hpp"
#include "feeModel.hpp"
#include "marketImpactModel.hpp"
#include "orderbook.hpp"

struct TradeMetrics {
    double expectedSlippage;
    double expectedFees;
    double expectedMarketImpact;
    double netCost;
    double makerTakerProportion;
    double internalLatency;
};

struct TradeResult {
    std::string timestamp;
    double slippage = 0.0;
    double marketImpact = 0.0;
    double executedPrice = 0.0;
    double executedSize = 0.0;
    double fees = 0.0;
    double totalCost = 0.0;
};

struct DetailedTradeMetrics {
    double currentSpread = 0.0;
    double midPrice = 0.0;
    double orderBookImbalance = 0.0;
    double slippageConfidence = 0.0;
    double impactConfidence = 0.0;
    double expectedSlippage = 0.0;
    double expectedMarketImpact = 0.0;
    double makerTakerRatio = 0.0;
    double expectedFees = 0.0;
    double netCost = 0.0;
    double internalLatency = 0.0;
};

class Simulator {
public:
    Simulator();
    ~Simulator();

    // Initialize the simulator with exchange and asset
    void initialize(const std::string& exchange, const std::string& spotAsset);
    TradeMetrics simulateMarketOrder(double quantityUSD);
    void updateMarketData(const OrderBook& orderbook);
    double getCurrentVolatility() const;
    std::string getCurrentFeeTier() const;

    TradeResult simulateTrade(double orderSize, double limitPrice, const std::string& orderType, double timeHorizon);
    DetailedTradeMetrics calculateTradeMetrics(double orderSize, double limitPrice, const std::string& orderType, const OrderBook& orderbook, double timeHorizon);
    double getCurrentCapital() const;
    double getCurrentPosition() const;
    double getCurrentPnL() const;
    void saveState(const std::string& filename);
    void loadState(const std::string& filename);

private:
    std::unique_ptr<SlippageModel> slippageModel_;
    std::unique_ptr<FeeModel> feeModel_;
    std::unique_ptr<MarketImpactModel> marketImpactModel_;
    
    std::string exchange_;
    std::string spotAsset_;
    double initialCapital_ = 0.0;
    double currentCapital_ = 0.0;
    double currentPosition_ = 0.0;
    double currentVolatility_ = 0.0;
    std::string currentFeeTier_;

    // Helper methods
    double calculateMakerTakerProportion(const OrderBook& orderbook);
    double measureInternalLatency();
    double calculateMakerTakerProbability(const OrderBook& orderbook, double limitPrice);
    double calculateOrderBookImbalance(const OrderBook& orderbook);
    double estimateInternalLatency();
}; 