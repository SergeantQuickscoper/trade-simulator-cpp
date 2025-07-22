#pragma once

#include <vector>
#include <string>

class MarketImpactModel {
public:
    MarketImpactModel();
    ~MarketImpactModel();

    // Initialize the model with market parameters
    void initialize(double volatility,
                   double dailyVolume,
                   double permanentImpactFactor = 0.1,
                   double temporaryImpactFactor = 0.1);

    // Calculate market impact using Almgren-Chriss model
    double calculateMarketImpact(double orderSize,
                               double currentPrice,
                               double timeHorizon);

    // Calculate optimal execution trajectory
    std::vector<double> calculateOptimalTrajectory(double totalSize,
                                                 double timeHorizon,
                                                 double riskAversion = 0.0);

    // Update market parameters
    void updateParameters(double volatility,
                         double dailyVolume,
                         double permanentImpactFactor,
                         double temporaryImpactFactor);

    // Get current parameters
    double getVolatility() const;
    double getDailyVolume() const;
    double getPermanentImpactFactor() const;
    double getTemporaryImpactFactor() const;

private:
    double volatility_;
    double dailyVolume_;
    double permanentImpactFactor_;
    double temporaryImpactFactor_;
}; 