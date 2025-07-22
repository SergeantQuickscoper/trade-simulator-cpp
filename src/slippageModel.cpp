#include "slippageModel.hpp"
#include <vector>
#include <algorithm>
#include <cmath>
#include <numeric>
#include <mutex>

class SlippageModel::Impl {
public:
    struct DataPoint {
        double price;
        double volume;
        double timeStamp;
    };

    std::vector<DataPoint> historicalData_;
    std::vector<double> quantiles_;
    double currentQuantile_;
    std::mutex mutex_;  // love thread safety

    Impl() : currentQuantile_(0.95) {
        // Initialize with common quantiles
        quantiles_ = {0.1, 0.25, 0.5, 0.75, 0.9, 0.95, 0.99};
    }

    double predictQuantile(const std::vector<double>& values, double quantile) {
        if (values.empty()) return 0.0;
        if (quantile <= 0.0) return values.front();
        if (quantile >= 1.0) return values.back();
        
        std::vector<double> sortedValues = values;
        std::sort(sortedValues.begin(), sortedValues.end());
        
        double position = quantile * (sortedValues.size() - 1);
        size_t index = static_cast<size_t>(position);
        double fraction = position - index;
        
        if (index + 1 >= sortedValues.size()) {
            return sortedValues[index];
        }
        
        // Linear interpolation between adjacent values
        return sortedValues[index] + fraction * (sortedValues[index + 1] - sortedValues[index]);
    }

    double calculateSlippage(double orderSize, double currentPrice) {
        if (historicalData_.empty()) return 0.0;
        if (currentPrice <= 0.0) return 0.0;

        // price impact based on order size relative to historical volumes
        std::vector<double> volumes;
        for (const auto& data : historicalData_) {
            if (data.volume > 0.0) {  
                volumes.push_back(data.volume);
            }
        }
        
        if (volumes.empty()) return 0.0;
        
        double avgVolume = std::accumulate(volumes.begin(), volumes.end(), 0.0) / volumes.size();
        if (avgVolume <= 0.0) return 0.0;
        
        double sizeRatio = std::abs(orderSize) / avgVolume;
        
        // price volatility
        std::vector<double> returns;
        for (size_t i = 1; i < historicalData_.size(); ++i) {
            if (historicalData_[i-1].price > 0.0) { 
                double ret = (historicalData_[i].price - historicalData_[i-1].price) / historicalData_[i-1].price;
                returns.push_back(ret);
            }
        }
        
        if (returns.empty()) return 0.0;
        
        double volatility = predictQuantile(returns, currentQuantile_);
        
        // slippage as a function of size ratio and volatility
        double slippage = currentPrice * volatility * std::sqrt(sizeRatio);
        
        return slippage;
    }
};

SlippageModel::SlippageModel() : pImpl(std::make_unique<Impl>()) {}
SlippageModel::~SlippageModel() = default;

void SlippageModel::initialize(const std::vector<double>& prices,
                             const std::vector<double>& volumes,
                             const std::vector<double>& timeStamps) {
    if (prices.size() != volumes.size() || prices.size() != timeStamps.size()) {
        return;
    }

    std::lock_guard<std::mutex> lock(pImpl->mutex_);
    pImpl->historicalData_.clear();
    for (size_t i = 0; i < prices.size(); ++i) {
        pImpl->historicalData_.push_back({prices[i], volumes[i], timeStamps[i]});
    }
}

double SlippageModel::predictSlippage(double orderSize, double currentPrice, double confidenceLevel) {
    std::lock_guard<std::mutex> lock(pImpl->mutex_);
    pImpl->currentQuantile_ = confidenceLevel;
    return pImpl->calculateSlippage(orderSize, currentPrice);
}

void SlippageModel::update(double price, double volume, double timeStamp) {
    if (price <= 0.0 || volume < 0.0) return; 
    
    std::lock_guard<std::mutex> lock(pImpl->mutex_);
    pImpl->historicalData_.push_back({price, volume, timeStamp});
    
    const size_t maxDataPoints = 1000;
    if (pImpl->historicalData_.size() > maxDataPoints) {
        pImpl->historicalData_.erase(pImpl->historicalData_.begin(),
                                   pImpl->historicalData_.begin() + (pImpl->historicalData_.size() - maxDataPoints));
    }
}

// In the future can save and load model data to files here

void SlippageModel::saveModel(const std::string& filename) {
    // TODO: Implement model saving
}

void SlippageModel::loadModel(const std::string& filename) {
    // TODO: Implement model loading
} 