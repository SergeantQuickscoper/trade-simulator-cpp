#include "simulator.hpp"
#include <chrono>
#include <iomanip>
#include <sstream>
#include <cmath>

Simulator::Simulator()
    : slippageModel_(std::make_unique<SlippageModel>())
    , feeModel_(std::make_unique<FeeModel>())
    , marketImpactModel_(std::make_unique<MarketImpactModel>())
    , initialCapital_(0.0)
    , currentCapital_(0.0)
    , currentPosition_(0.0)
    , currentVolatility_(0.0) {}

Simulator::~Simulator() = default;

void Simulator::initialize(const std::string& exchange, const std::string& spotAsset) {
    exchange_ = exchange;
    spotAsset_ = spotAsset;
    
    feeModel_->initialize(exchange, "tier1");
    marketImpactModel_->initialize(0.02, 1000000.0);
}

TradeMetrics Simulator::simulateMarketOrder(double quantityUSD) {
    auto start = std::chrono::high_resolution_clock::now();
    
    double slippage = slippageModel_->predictSlippage(quantityUSD, 0.0);
    double fees = feeModel_->calculateFees(quantityUSD, 0.0, false);
    double impact = marketImpactModel_->calculateMarketImpact(quantityUSD, 0.0, 1.0);
    
    auto end = std::chrono::high_resolution_clock::now();
    double latency = std::chrono::duration<double, std::milli>(end - start).count();
    
    return TradeMetrics{
        slippage,
        fees,
        impact,
        slippage + fees + impact,
        calculateMakerTakerProportion(OrderBook(exchange_, spotAsset_)),
        latency
    };
}

void Simulator::updateMarketData(const OrderBook& orderbook) {
    // Use bid+ask volume as a proxy for total volume
    double totalVolume = orderbook.getBidVolume() + orderbook.getAskVolume();
    slippageModel_->update(orderbook.getMidPrice(), totalVolume, 0.0);
}

double Simulator::getCurrentVolatility() const {
    return currentVolatility_;
}

std::string Simulator::getCurrentFeeTier() const {
    return currentFeeTier_;
}

double Simulator::calculateMakerTakerProportion(const OrderBook& orderbook) {
    double bidVolume = orderbook.getBidVolume();
    double askVolume = orderbook.getAskVolume();
    return bidVolume / (bidVolume + askVolume);
}

double Simulator::measureInternalLatency() {
    auto start = std::chrono::high_resolution_clock::now();
    simulateMarketOrder(100.0);
    auto end = std::chrono::high_resolution_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

TradeResult Simulator::simulateTrade(double orderSize,
                                   double limitPrice,
                                   const std::string& orderType,
                                   double timeHorizon) {
    TradeResult result;
    
    // Get current timestamp
    auto now = std::chrono::system_clock::now();
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::stringstream ss;
    ss << std::put_time(std::localtime(&now_time_t), "%Y-%m-%d %H:%M:%S");
    result.timestamp = ss.str();

    // Calculate slippage
    result.slippage = slippageModel_->predictSlippage(orderSize, limitPrice);

    // Calculate market impact
    result.marketImpact = marketImpactModel_->calculateMarketImpact(orderSize, limitPrice, timeHorizon);

    // Calculate final execution price
    result.executedPrice = limitPrice + result.slippage + result.marketImpact;
    result.executedSize = orderSize;

    // Calculate fees
    bool isMaker = (orderType == "limit");
    result.fees = feeModel_->calculateFees(orderSize, result.executedPrice, isMaker);

    // Calculate total cost
    result.totalCost = (result.executedPrice * result.executedSize) + result.fees;

    // Update portfolio state
    if (orderSize > 0) {  // Buy
        currentCapital_ -= result.totalCost;
        currentPosition_ += orderSize;
    } else {  // Sell
        currentCapital_ += result.totalCost;
        currentPosition_ += orderSize;
    }

    return result;
}

double Simulator::getCurrentCapital() const {
    return currentCapital_;
}

double Simulator::getCurrentPosition() const {
    return currentPosition_;
}

double Simulator::getCurrentPnL() const {
    return currentCapital_ - initialCapital_;
}

void Simulator::saveState(const std::string& filename) {
    // TODO: Implement state saving
}

void Simulator::loadState(const std::string& filename) {
    // TODO: Implement state loading
}

DetailedTradeMetrics Simulator::calculateTradeMetrics(double orderSize,
                                                    double limitPrice,
                                                    const std::string& orderType,
                                                    const OrderBook& orderbook,
                                                    double timeHorizon) {
    DetailedTradeMetrics metrics;
    
    // Get current market conditions
    auto bestAsk = orderbook.getBestAsk();
    auto bestBid = orderbook.getBestBid();
    
    if (!bestAsk || !bestBid) {
        return metrics;  // Return empty metrics if no market data
    }
    
    // Calculate market conditions
    metrics.currentSpread = bestAsk->price - bestBid->price;
    metrics.midPrice = (bestAsk->price + bestBid->price) / 2.0;
    metrics.orderBookImbalance = calculateOrderBookImbalance(orderbook);
    
    // Calculate expected costs with confidence levels
    metrics.slippageConfidence = 0.95;  // 95% confidence level
    metrics.impactConfidence = 0.90;    // 90% confidence level
    
    metrics.expectedSlippage = slippageModel_->predictSlippage(
        orderSize, 
        metrics.midPrice, 
        metrics.slippageConfidence
    );
    
    metrics.expectedMarketImpact = marketImpactModel_->calculateMarketImpact(
        orderSize,
        metrics.midPrice,
        timeHorizon
    );
    
    // Calculate maker/taker probability
    metrics.makerTakerRatio = calculateMakerTakerProbability(orderbook, limitPrice);
    
    // Calculate fees based on maker/taker probability
    bool isMaker = (metrics.makerTakerRatio > 0.5);
    metrics.expectedFees = feeModel_->calculateFees(
        orderSize,
        metrics.midPrice,
        isMaker
    );
    
    // Calculate net cost
    metrics.netCost = metrics.expectedSlippage + 
                     metrics.expectedFees + 
                     metrics.expectedMarketImpact;
    
    // Estimate internal latency
    metrics.internalLatency = estimateInternalLatency();
    
    return metrics;
}

double Simulator::calculateMakerTakerProbability(const OrderBook& orderbook, double limitPrice) {
    auto bestAsk = orderbook.getBestAsk();
    auto bestBid = orderbook.getBestBid();
    
    if (!bestAsk || !bestBid) {
        return 0.5;  // Default to 50% if no market data
    }
    
    // Simple logistic regression based on price position relative to spread
    double midPrice = (bestAsk->price + bestBid->price) / 2.0;
    double spread = bestAsk->price - bestBid->price;
    
    // Calculate how far the limit price is from the mid price in terms of spread
    double normalizedDistance = (limitPrice - midPrice) / (spread / 2.0);
    
    // Apply logistic function: 1 / (1 + e^(-x))
    double probability = 1.0 / (1.0 + std::exp(-normalizedDistance));
    
    return probability;
}

double Simulator::calculateOrderBookImbalance(const OrderBook& orderbook) {
    // Get top 10 levels of the order book
    auto asks = orderbook.getAsksAtDepth(10);
    auto bids = orderbook.getBidsAtDepth(10);
    
    double totalAskVolume = 0.0;
    double totalBidVolume = 0.0;
    
    for (const auto& ask : asks) {
        totalAskVolume += ask.quantity;
    }
    
    for (const auto& bid : bids) {
        totalBidVolume += bid.quantity;
    }
    
    double totalVolume = totalAskVolume + totalBidVolume;
    if (totalVolume == 0.0) {
        return 0.0;
    }
    
    return (totalBidVolume - totalAskVolume) / totalVolume;
}

double Simulator::estimateInternalLatency() {
    // Simulate internal latency with some randomness
    // In a real system, this would be measured from actual trading infrastructure
    const double baseLatency = 0.1;  // 100 microseconds base latency
    const double jitter = 0.05;      // 50 microseconds jitter
    double randomJitter = (static_cast<double>(rand()) / RAND_MAX) * jitter;
    
    return (baseLatency + randomJitter) * 1000.0;  // Convert to milliseconds
} 