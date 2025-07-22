#include "feeModel.hpp"

FeeModel::FeeModel()
    : makerFeeRate_(0.001)
    , takerFeeRate_(0.002) {
    initializeOKXFeeTiers();
}

FeeModel::~FeeModel() = default;

void FeeModel::initialize(const std::string& exchange, const std::string& feeTier) {
    exchange_ = exchange;
    feeTier_ = feeTier;
    
    if (exchange == "OKX") {
        auto it = okxFeeTiers_.find(feeTier);
        if (it != okxFeeTiers_.end()) {
            makerFeeRate_ = it->second.makerRate;
            takerFeeRate_ = it->second.takerRate;
        }
    }
}

double FeeModel::calculateFees(double orderSize, double price, bool isMaker) {
    double feeRate = isMaker ? makerFeeRate_ : takerFeeRate_;
    return orderSize * price * feeRate;
}

void FeeModel::updateFeeTier(const std::string& feeTier) {
    feeTier_ = feeTier;
    initialize(exchange_, feeTier);
}

double FeeModel::getMakerFeeRate() const {
    return makerFeeRate_;
}

double FeeModel::getTakerFeeRate() const {
    return takerFeeRate_;
}

void FeeModel::initializeOKXFeeTiers() {
    okxFeeTiers_["tier1"] = {0.0008, 0.001};  // 0.08% maker, 0.1% taker
    okxFeeTiers_["tier2"] = {0.0007, 0.0009}; // 0.07% maker, 0.09% taker
    okxFeeTiers_["tier3"] = {0.0006, 0.0008}; // 0.06% maker, 0.08% taker
    okxFeeTiers_["tier4"] = {0.0005, 0.0007}; // 0.05% maker, 0.07% taker
    okxFeeTiers_["tier5"] = {0.0004, 0.0006}; // 0.04% maker, 0.06% taker
} 