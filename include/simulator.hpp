#pragma once

#include "slippageModel.hpp"
#include "feeModel.hpp"
#include "marketImpactModel.hpp"
#include "orderbook.hpp"
#include <memory>
#include <string>
#include <vector>

struct TradeResult {
    double executedPrice;
    double executedSize;
    double slippage;
    double fees;
    double marketImpact;
    double totalCost;
    std::string timestamp;
};

struct DetailedTradeMetrics {
    // Expected costs
    double expectedSlippage;
    double expectedFees;
    double expectedMarketImpact;
    double netCost;
    
    // Execution metrics
    double makerTakerRatio;  // Probability of being a maker
    double internalLatency;  // in milliseconds
    
    // Market conditions
    double currentSpread;
    double midPrice;
    double orderBookImbalance;  // (bid volume - ask volume) / (bid volume + ask volume)
    
    // Confidence metrics
    double slippageConfidence;
    double impactConfidence;
};

class Simulator {
public:
    Simulator();
    ~Simulator();

    // Initialize the simulator with models and parameters
    void initialize(const std::string& exchange,
                   const std::string& symbol,
                   double initialCapital);

    // Simulate a trade
    TradeResult simulateTrade(double orderSize,
                            double limitPrice,
                            const std::string& orderType,
                            double timeHorizon = 0.0);

    // Calculate detailed trade metrics
    DetailedTradeMetrics calculateTradeMetrics(double orderSize,
                                             double limitPrice,
                                             const std::string& orderType,
                                             const OrderBook& orderbook,
                                             double timeHorizon = 0.0);

    // Update market data
    void updateMarketData(const OrderBook& orderbook);

    // Get current portfolio state
    double getCurrentCapital() const;
    double getCurrentPosition() const;
    double getCurrentPnL() const;

    // Save/load simulation state
    void saveState(const std::string& filename);
    void loadState(const std::string& filename);

private:
    std::unique_ptr<SlippageModel> slippageModel_;
    std::unique_ptr<FeeModel> feeModel_;
    std::unique_ptr<MarketImpactModel> marketImpactModel_;
    
    double initialCapital_;
    double currentCapital_;
    double currentPosition_;
    std::string exchange_;
    std::string symbol_;

    // Helper methods
    double calculateMakerTakerProbability(const OrderBook& orderbook, double limitPrice);
    double calculateOrderBookImbalance(const OrderBook& orderbook);
    double estimateInternalLatency();
}; 