#pragma once

#include <string>
#include <map>

class FeeModel {
public:
    FeeModel();
    ~FeeModel();

    // Initialize fee model for a specific exchange
    void initialize(const std::string& exchange, const std::string& feeTier);

    // Calculate fees for a market order
    double calculateFees(double orderSize, double price, bool isMaker);

    // Update fee tier
    void updateFeeTier(const std::string& feeTier);

    // Get current fee rates
    double getMakerFeeRate() const;
    double getTakerFeeRate() const;

private:
    std::string exchange_;
    std::string feeTier_;
    double makerFeeRate_;
    double takerFeeRate_;

    // Fee tier definitions for different exchanges
    struct FeeTier {
        double makerRate;
        double takerRate;
    };
    
    std::map<std::string, FeeTier> okxFeeTiers_;
    void initializeOKXFeeTiers();
}; 