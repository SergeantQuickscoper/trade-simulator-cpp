#pragma once

#include <vector>
#include <memory>
#include <string>

class SlippageModel {
public:
    SlippageModel();
    ~SlippageModel();

    // Initialize the model with historical data
    void initialize(const std::vector<double>& prices, 
                   const std::vector<double>& volumes,
                   const std::vector<double>& timeStamps);

    // Predict slippage using quantile regression
    double predictSlippage(double orderSize, 
                          double currentPrice, 
                          double quantile = 0.95);

    // Update the model with new data point
    void update(double price, double volume, double timeStamp);

    // Get model statistics
    double getMeanSlippage() const;
    double getSlippageStdDev() const;
    double getSlippageQuantile(double quantile) const;

    // Save/load model parameters
    void saveModel(const std::string& filename);
    void loadModel(const std::string& filename);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;  // PIMPL idiom for implementation details
}; 