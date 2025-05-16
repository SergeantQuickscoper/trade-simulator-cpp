#include "feeModel.hpp"
#include <algorithm>

FeeModel::FeeModel() = default;
FeeModel::~FeeModel() = default;

void FeeModel::initialize(const std::string& exchange, 
                         double makerFee, 
                         double takerFee,
                         double minFee,
                         double maxFee) {
    FeeStructure fees;
    fees.makerFee = makerFee;
    fees.takerFee = takerFee;
    fees.minFee = minFee;
    fees.maxFee = maxFee;
    
    feeStructures_[exchange] = fees;
}

double FeeModel::calculateFee(double orderSize, 
                            double price, 
                            bool isMaker,
                            const std::string& orderType) {
    // Get the fee structure for the current exchange
    auto it = feeStructures_.begin();  // For now, just use the first exchange's fees
    if (it == feeStructures_.end()) {
        return 0.0;  // No fee structure defined
    }
    
    const auto& fees = it->second;
    
    // Calculate base fee
    double feeRate = isMaker ? fees.makerFee : fees.takerFee;
    double baseFee = orderSize * price * feeRate;
    
    // Apply minimum fee if specified
    if (fees.minFee > 0.0) {
        baseFee = std::max(baseFee, fees.minFee);
    }
    
    // Apply maximum fee if specified
    if (fees.maxFee > 0.0) {
        baseFee = std::min(baseFee, fees.maxFee);
    }
    
    return baseFee;
}

double FeeModel::getMakerFee() const {
    auto it = feeStructures_.begin();
    return (it != feeStructures_.end()) ? it->second.makerFee : 0.0;
}

double FeeModel::getTakerFee() const {
    auto it = feeStructures_.begin();
    return (it != feeStructures_.end()) ? it->second.takerFee : 0.0;
}

void FeeModel::updateFees(double makerFee, double takerFee) {
    auto it = feeStructures_.begin();
    if (it != feeStructures_.end()) {
        it->second.makerFee = makerFee;
        it->second.takerFee = takerFee;
    }
} 