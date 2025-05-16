#pragma once

#include <vector>
#include <string>

class MarketImpactModel {
public:
    MarketImpactModel();
    ~MarketImpactModel();

    // Initialize the model with market parameters
    void initialize(double dailyVolume,
                   double volatility,
                   double permanentImpactFactor = 0.1,
                   double temporaryImpactFactor = 0.1);

    // Calculate optimal execution trajectory
    std::vector<double> calculateOptimalTrajectory(double totalSize,
                                                 double timeHorizon,
                                                 double riskAversion = 0.0);

    // Calculate market impact for a single trade
    double calculateImpact(double orderSize, 
                          double currentPrice,
                          double timeHorizon);

    // Update market parameters
    void updateParameters(double dailyVolume,
                         double volatility,
                         double permanentImpactFactor,
                         double temporaryImpactFactor);

    // Get current parameters
    double getDailyVolume() const;
    double getVolatility() const;
    double getPermanentImpactFactor() const;
    double getTemporaryImpactFactor() const;

private:
    double dailyVolume_;
    double volatility_;
    double permanentImpactFactor_;
    double temporaryImpactFactor_;
}; 