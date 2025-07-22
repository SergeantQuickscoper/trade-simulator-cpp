#include "marketImpactModel.hpp"
#include <cmath>

MarketImpactModel::MarketImpactModel()
    : volatility_(0.0)
    , dailyVolume_(0.0)
    , permanentImpactFactor_(0.1)
    , temporaryImpactFactor_(0.1) {}

MarketImpactModel::~MarketImpactModel() = default;

void MarketImpactModel::initialize(double volatility,
                                 double dailyVolume,
                                 double permanentImpactFactor,
                                 double temporaryImpactFactor) {
    volatility_ = volatility;
    dailyVolume_ = dailyVolume;
    permanentImpactFactor_ = permanentImpactFactor;
    temporaryImpactFactor_ = temporaryImpactFactor;
}

double MarketImpactModel::calculateMarketImpact(double orderSize,
                                              double currentPrice,
                                              double timeHorizon) {
    double tempImpact = temporaryImpactFactor_ * 
                       std::sqrt(std::abs(orderSize) / dailyVolume_) * 
                       currentPrice;
    
    double permImpact = permanentImpactFactor_ * 
                       (std::abs(orderSize) / dailyVolume_) * 
                       currentPrice;
    
    return tempImpact + permImpact;
}

std::vector<double> MarketImpactModel::calculateOptimalTrajectory(double totalSize,
                                                                double timeHorizon,
                                                                double riskAversion) {
    std::vector<double> trajectory;
    int steps = 10;
    double stepSize = totalSize / steps;
    
    for (int i = 0; i < steps; ++i) {
        trajectory.push_back(stepSize);
    }
    
    return trajectory;
}

void MarketImpactModel::updateParameters(double volatility,
                                       double dailyVolume,
                                       double permanentImpactFactor,
                                       double temporaryImpactFactor) {
    volatility_ = volatility;
    dailyVolume_ = dailyVolume;
    permanentImpactFactor_ = permanentImpactFactor;
    temporaryImpactFactor_ = temporaryImpactFactor;
}

double MarketImpactModel::getVolatility() const {
    return volatility_;
}

double MarketImpactModel::getDailyVolume() const {
    return dailyVolume_;
}

double MarketImpactModel::getPermanentImpactFactor() const {
    return permanentImpactFactor_;
}

double MarketImpactModel::getTemporaryImpactFactor() const {
    return temporaryImpactFactor_;
} 