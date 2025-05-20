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

private:
    std::unique_ptr<SlippageModel> slippageModel_;
    std::unique_ptr<FeeModel> feeModel_;
    std::unique_ptr<MarketImpactModel> marketImpactModel_;
    
    std::string exchange_;
    std::string spotAsset_;
    double currentVolatility_;
    std::string currentFeeTier_;

    // Helper methods
    double calculateMakerTakerProportion(const OrderBook& orderbook);
    double measureInternalLatency();
}; 