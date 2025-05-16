#include "marketImpactModel.hpp"
#include <cmath>
#include <algorithm>

MarketImpactModel::MarketImpactModel()
    : dailyVolume_(0.0)
    , volatility_(0.0)
    , permanentImpactFactor_(0.1)
    , temporaryImpactFactor_(0.1) {}

MarketImpactModel::~MarketImpactModel() = default;

void MarketImpactModel::initialize(double dailyVolume,
                                 double volatility,
                                 double permanentImpactFactor,
                                 double temporaryImpactFactor) {
    dailyVolume_ = dailyVolume;
    volatility_ = volatility;
    permanentImpactFactor_ = permanentImpactFactor;
    temporaryImpactFactor_ = temporaryImpactFactor;
}

std::vector<double> MarketImpactModel::calculateOptimalTrajectory(double totalSize,
                                                                double timeHorizon,
                                                                double riskAversion) {
    // Number of time steps (assuming 1-minute intervals)
    const int numSteps = static_cast<int>(timeHorizon * 60);
    std::vector<double> trajectory(numSteps);
    
    if (numSteps <= 0) {
        return trajectory;
    }

    // Calculate parameters
    double eta = temporaryImpactFactor_ * volatility_ / std::sqrt(dailyVolume_);
    double gamma = permanentImpactFactor_ * volatility_ / std::sqrt(dailyVolume_);
    
    // Calculate optimal trading rate
    double kappa = std::sqrt(riskAversion * volatility_ * volatility_ / (2.0 * eta));
    
    // Calculate initial trading rate
    double x0 = totalSize;
    double v0 = x0 * kappa / std::tanh(kappa * timeHorizon);
    
    // Generate trajectory
    for (int i = 0; i < numSteps; ++i) {
        double t = static_cast<double>(i) / numSteps * timeHorizon;
        trajectory[i] = x0 * std::sinh(kappa * (timeHorizon - t)) / std::sinh(kappa * timeHorizon);
    }
    
    return trajectory;
}

double MarketImpactModel::calculateImpact(double orderSize, 
                                        double currentPrice,
                                        double timeHorizon) {
    // Calculate temporary impact
    double tempImpact = temporaryImpactFactor_ * 
                       std::sqrt(std::abs(orderSize) / dailyVolume_) * 
                       currentPrice;
    
    // Calculate permanent impact
    double permImpact = permanentImpactFactor_ * 
                       (std::abs(orderSize) / dailyVolume_) * 
                       currentPrice;
    
    // Total impact is the sum of temporary and permanent impact
    return tempImpact + permImpact;
}

void MarketImpactModel::updateParameters(double dailyVolume,
                                       double volatility,
                                       double permanentImpactFactor,
                                       double temporaryImpactFactor) {
    dailyVolume_ = dailyVolume;
    volatility_ = volatility;
    permanentImpactFactor_ = permanentImpactFactor;
    temporaryImpactFactor_ = temporaryImpactFactor;
}

double MarketImpactModel::getDailyVolume() const {
    return dailyVolume_;
}

double MarketImpactModel::getVolatility() const {
    return volatility_;
}

double MarketImpactModel::getPermanentImpactFactor() const {
    return permanentImpactFactor_;
}

double MarketImpactModel::getTemporaryImpactFactor() const {
    return temporaryImpactFactor_;
} 