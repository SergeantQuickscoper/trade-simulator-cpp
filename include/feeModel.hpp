#pragma once

#include <string>
#include <unordered_map>

class FeeModel {
public:
    FeeModel();
    ~FeeModel();

    // Initialize fee structure for an exchange
    void initialize(const std::string& exchange, 
                   double makerFee, 
                   double takerFee,
                   double minFee = 0.0,
                   double maxFee = 0.0);

    // Calculate fees for a trade
    double calculateFee(double orderSize, 
                       double price, 
                       bool isMaker = false,
                       const std::string& orderType = "market");

    // Get current fee rates
    double getMakerFee() const;
    double getTakerFee() const;

    // Update fee structure
    void updateFees(double makerFee, double takerFee);

private:
    struct FeeStructure {
        double makerFee;
        double takerFee;
        double minFee;
        double maxFee;
    };

    std::unordered_map<std::string, FeeStructure> feeStructures_;
}; 