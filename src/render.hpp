#ifdef WITH_SFML
#pragma once

#include <SFML/Graphics.hpp>
#include "particle.hpp"
#include "metrics.hpp"
#include <vector>
#include <string>

class RenderWindow {
public:
    enum class State {
        SIMULATION,
        RESULTS
    };
    
    RenderWindow(float width, float height, const std::string& method);
    ~RenderWindow();
    
    bool update(const std::vector<Particle>& particles, const Metrics& metrics, int step);
    bool isNextButtonClicked() const { return nextButtonClicked_; }
    void resetNextButton() { nextButtonClicked_ = false; }
    bool isBackButtonPressed() const { return backButtonPressed_; }
    void resetBackButton() { backButtonPressed_ = false; }
    void showResults(const Metrics& metrics, int finalStep, int N, float dt, float energyDrift, 
                    uint64_t seed, float boxW, float boxH, float radius);
    void loadOtherMethodFromCSV(const std::string& summaryPath);
    State getState() const { return state_; }
    bool isWindowOpen() const { return window_.isOpen(); }
    
private:
    sf::RenderWindow window_;
    sf::Font font_;
    bool nextButtonClicked_;
    bool backButtonPressed_;
    State state_;
    std::string method_;
    float box_w_, box_h_;
    
    // Results data (current method)
    int resultsStep_;
    int resultsN_;
    float resultsDt_;
    float resultsEnergyDrift_;
    int resultsCollisions_;
    double resultsStepsPerSec_;
    double resultsAvgCandPairs_;
    double resultsP50_;
    double resultsP95_;
    double resultsEnergyDriftMedian_;
    double resultsEnergyDriftMax_;
    uint64_t resultsSeed_;
    float resultsBoxW_;
    float resultsBoxH_;
    float resultsRadius_;
    
    // Results data (other method - for comparison)
    bool hasOtherMethod_;
    std::string otherMethod_;
    int otherResultsStep_;
    int otherResultsN_;
    float otherResultsDt_;
    double otherResultsStepsPerSec_;
    double otherResultsAvgCandPairs_;
    double otherResultsP50_;
    double otherResultsP95_;
    double otherResultsEnergyDriftMedian_;
    double otherResultsEnergyDriftMax_;
    uint64_t otherResultsSeed_;
    float otherResultsBoxW_;
    float otherResultsBoxH_;
    float otherResultsRadius_;
    
    void drawParticles(const std::vector<Particle>& particles);
    void drawHUD(const Metrics& metrics, int step);
    void drawResultsScreen();
    void handleEvents();
    void drawTextValue(float x, float y, const std::string& label, const std::string& value);
    bool tryLoadSystemFont();
};

#endif // WITH_SFML

