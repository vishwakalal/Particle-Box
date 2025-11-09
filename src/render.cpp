#ifdef WITH_SFML
#include "render.hpp"
#include <sstream>
#include <iomanip>
#include <cmath>
#include <fstream>
using namespace std;
RenderWindow::RenderWindow(float width, float height, const std::string& method)
    : window_(sf::VideoMode(sf::Vector2u(static_cast<unsigned int>(width), static_cast<unsigned int>(height))),
              "Particle Box Simulation"),
      nextButtonClicked_(false), backButtonPressed_(false), state_(State::SIMULATION),
      method_(method), box_w_(width), box_h_(height), hasOtherMethod_(false) {
    window_.setFramerateLimit(60);
    
    // Try to load a system font
    tryLoadSystemFont();
}

RenderWindow::~RenderWindow() {
    if (window_.isOpen()) {
        window_.close();
    }
}

bool RenderWindow::update(const vector<Particle>& particles, const Metrics& metrics, int step) {
    handleEvents();
    
    if (!window_.isOpen()) {
        return false;
    }
    
    window_.clear(sf::Color::Black);
    
    if (state_ == State::SIMULATION) {
        drawParticles(particles);
        drawHUD(metrics, step);
    } else if (state_ == State::RESULTS) {
        drawResultsScreen();
    }
    
    window_.display();
    return true;
}

void RenderWindow::showResults(const Metrics& metrics, int finalStep, int N, float dt, float energyDrift,
                               uint64_t seed, float boxW, float boxH, float radius) {
    state_ = State::RESULTS;
    resultsStep_ = finalStep;
    resultsN_ = N;
    resultsDt_ = dt;
    resultsEnergyDrift_ = energyDrift;
    resultsCollisions_ = metrics.getTotalCollisions();
    resultsStepsPerSec_ = metrics.steps_per_sec;
    resultsAvgCandPairs_ = metrics.cand_per_particle;
    resultsP50_ = metrics.p50_ms;
    resultsP95_ = metrics.p95_ms;
    resultsEnergyDriftMedian_ = metrics.energy_drift_median;
    resultsEnergyDriftMax_ = metrics.energy_drift_max;
    resultsSeed_ = seed;
    resultsBoxW_ = boxW;
    resultsBoxH_ = boxH;
    resultsRadius_ = radius;
    window_.setTitle("Particle Box Simulation - Results");
}

bool RenderWindow::tryLoadSystemFont() {
    // Try common macOS system font paths
    const char* fontPaths[] = {
        "/System/Library/Fonts/Supplemental/Arial.ttf",
        "/System/Library/Fonts/Helvetica.ttc",
        "/Library/Fonts/Arial.ttf",
        "/System/Library/Fonts/HelveticaNeue.ttc",
        nullptr
    };
    
    for (int i = 0; fontPaths[i] != nullptr; ++i) {
        if (font_.openFromFile(fontPaths[i])) {
            return true;
        }
    }
    
    return false;
}

void RenderWindow::drawParticles(const vector<Particle>& particles) {
    for (const auto& p : particles) {
        sf::CircleShape circle(p.r);
        circle.setPosition(sf::Vector2f(p.x - p.r, p.y - p.r));
        circle.setFillColor(p.collided ? sf::Color::Red : sf::Color::White);
        circle.setOutlineColor(sf::Color::Cyan);
        circle.setOutlineThickness(1.0f);
        window_.draw(circle);
    }
    
    // Draw box border
    sf::RectangleShape border;
    border.setSize(sf::Vector2f(box_w_, box_h_));
    border.setPosition(sf::Vector2f(0, 0));
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color::Green);
    border.setOutlineThickness(2.0f);
    window_.draw(border);
}

void RenderWindow::drawHUD(const Metrics& metrics, int step) {
    // Simple HUD - just a background bar
    sf::RectangleShape bg(sf::Vector2f(box_w_, 30));
    bg.setPosition(sf::Vector2f(0, 0));
    bg.setFillColor(sf::Color(0, 0, 0, 200));
    window_.draw(bg);
    
    // No button - use N key to go to results
}

void RenderWindow::drawResultsScreen() {
    // Background
    sf::RectangleShape bg(sf::Vector2f(box_w_, box_h_));
    bg.setPosition(sf::Vector2f(0, 0));
    bg.setFillColor(sf::Color(20, 20, 40));
    window_.draw(bg);
    
    // Title bar
    sf::RectangleShape titleBar(sf::Vector2f(box_w_, 60));
    titleBar.setPosition(sf::Vector2f(0, 0));
    titleBar.setFillColor(sf::Color(30, 30, 60));
    titleBar.setOutlineColor(sf::Color::Cyan);
    titleBar.setOutlineThickness(2.0f);
    window_.draw(titleBar);
    
    // Try to render text if font is available
    bool fontAvailable = font_.getInfo().family != "";
    
    float startY = 80.0f;
    float spacing = 35.0f;
    float x = 20.0f;
    
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6);
    
    if (fontAvailable) {
        // Render text using font - side by side if both methods available
        float y = startY;
        float leftX = x;
        float rightX = hasOtherMethod_ ? (box_w_ / 2.0f + 20.0f) : x;
        float columnWidth = hasOtherMethod_ ? (box_w_ / 2.0f - 40.0f) : (box_w_ - 40.0f);
        
        // Draw column headers
        if (hasOtherMethod_) {
            drawTextValue(leftX, y - 20.0f, method_ + ":", "");
            drawTextValue(rightX, y - 20.0f, otherMethod_ + ":", "");
            
            // Draw separator line
            sf::RectangleShape separator(sf::Vector2f(2.0f, box_h_ - startY));
            separator.setPosition(sf::Vector2f(box_w_ / 2.0f - 1.0f, startY - 20.0f));
            separator.setFillColor(sf::Color::Cyan);
            window_.draw(separator);
        }
        
        // Left column (current method)
        float currentY = y;
        
        // Method
        drawTextValue(leftX, currentY, "Method:", method_);
        currentY += spacing;
        
        // N
        drawTextValue(leftX, currentY, "N:", std::to_string(resultsN_));
        currentY += spacing;
        
        // dt
        oss.str("");
        oss << std::setprecision(3) << resultsDt_;
        drawTextValue(leftX, currentY, "dt:", oss.str());
        currentY += spacing;
        
        // steps
        drawTextValue(leftX, currentY, "steps:", std::to_string(resultsStep_));
        currentY += spacing;
        
        // steps_per_sec
        oss.str("");
        oss << std::setprecision(1) << resultsStepsPerSec_;
        drawTextValue(leftX, currentY, "steps_per_sec:", oss.str());
        currentY += spacing;
        
        // cand_per_particle
        oss.str("");
        oss << std::setprecision(2) << resultsAvgCandPairs_;
        drawTextValue(leftX, currentY, "cand_per_particle:", oss.str());
        currentY += spacing;
        
        // p50_ms
        oss.str("");
        oss << std::setprecision(2) << resultsP50_;
        drawTextValue(leftX, currentY, "p50_ms:", oss.str());
        currentY += spacing;
        
        // p95_ms
        oss.str("");
        oss << std::setprecision(2) << resultsP95_;
        drawTextValue(leftX, currentY, "p95_ms:", oss.str());
        currentY += spacing;
        
        // energy_drift_median
        oss.str("");
        oss << std::scientific << std::setprecision(1) << resultsEnergyDriftMedian_;
        drawTextValue(leftX, currentY, "energy_drift_median:", oss.str());
        currentY += spacing;
        
        // energy_drift_max
        oss.str("");
        oss << std::scientific << std::setprecision(1) << resultsEnergyDriftMax_;
        drawTextValue(leftX, currentY, "energy_drift_max:", oss.str());
        currentY += spacing;
        
        // seed
        drawTextValue(leftX, currentY, "seed:", std::to_string(resultsSeed_));
        currentY += spacing;
        
        
        // radius
        oss.str("");
        oss << std::setprecision(1) << resultsRadius_;
        drawTextValue(leftX, currentY, "radius:", oss.str());
        
        // Right column (other method) if available
        if (hasOtherMethod_) {
            currentY = y;
            
            // Method
            drawTextValue(rightX, currentY, "Method:", otherMethod_);
            currentY += spacing;
            
            // N
            drawTextValue(rightX, currentY, "N:", std::to_string(otherResultsN_));
            currentY += spacing;
            
            // dt
            oss.str("");
            oss << std::setprecision(3) << otherResultsDt_;
            drawTextValue(rightX, currentY, "dt:", oss.str());
            currentY += spacing;
            
            // steps
            drawTextValue(rightX, currentY, "steps:", std::to_string(otherResultsStep_));
            currentY += spacing;
            
            // steps_per_sec
            oss.str("");
            oss << std::setprecision(1) << otherResultsStepsPerSec_;
            drawTextValue(rightX, currentY, "steps_per_sec:", oss.str());
            currentY += spacing;
            
            // cand_per_particle
            oss.str("");
            oss << std::setprecision(2) << otherResultsAvgCandPairs_;
            drawTextValue(rightX, currentY, "cand_per_particle:", oss.str());
            currentY += spacing;
            
            // p50_ms
            oss.str("");
            oss << std::setprecision(2) << otherResultsP50_;
            drawTextValue(rightX, currentY, "p50_ms:", oss.str());
            currentY += spacing;
            
            // p95_ms
            oss.str("");
            oss << std::setprecision(2) << otherResultsP95_;
            drawTextValue(rightX, currentY, "p95_ms:", oss.str());
            currentY += spacing;
            
            // energy_drift_median
            oss.str("");
            oss << std::scientific << std::setprecision(1) << otherResultsEnergyDriftMedian_;
            drawTextValue(rightX, currentY, "energy_drift_median:", oss.str());
            currentY += spacing;
            
            // energy_drift_max
            oss.str("");
            oss << std::scientific << std::setprecision(1) << otherResultsEnergyDriftMax_;
            drawTextValue(rightX, currentY, "energy_drift_max:", oss.str());
            currentY += spacing;
            
            // seed
            drawTextValue(rightX, currentY, "seed:", std::to_string(otherResultsSeed_));
            currentY += spacing;
            
            
            // radius
            oss.str("");
            oss << std::setprecision(1) << otherResultsRadius_;
            drawTextValue(rightX, currentY, "radius:", oss.str());
        }
    } else {
        // Fallback: display values as simple text representation
        // Create a simple text display using formatted strings drawn as shapes
        // For now, we'll just show a message that font is not available
        // In a real implementation, you'd use a text rendering library or shapes
        
        sf::RectangleShape msgBg(sf::Vector2f(box_w_ - 40, 100));
        msgBg.setPosition(sf::Vector2f(20, box_h_ / 2 - 50));
        msgBg.setFillColor(sf::Color(50, 50, 50));
        msgBg.setOutlineColor(sf::Color::White);
        msgBg.setOutlineThickness(2.0f);
        window_.draw(msgBg);
        
        // Display summary.csv path
        std::ostringstream pathMsg;
        pathMsg << "Results saved to summary.csv\n";
        pathMsg << "Check console or CSV file for values";
        
        // Since we can't render text easily, just show a simple indicator
        // The actual values are in the console and CSV
    }
}

void RenderWindow::drawTextValue(float x, float y, const std::string& label, const std::string& value) {
    if (font_.getInfo().family == "") {
        return; // No font available
    }
    
    // SFML 3 requires font in constructor
    std::string fullText = label + " " + value;
    sf::Text text(font_, fullText, 18);
    text.setFillColor(sf::Color::White);
    text.setPosition(sf::Vector2f(x, y));
    
    window_.draw(text);
}

void RenderWindow::loadOtherMethodFromCSV(const std::string& summaryPath) {
    hasOtherMethod_ = false;
    
    // Determine which method to look for
    std::string targetMethod = (method_ == "quadtree") ? "hash" : "quadtree";
    
    std::ifstream file(summaryPath);
    if (!file.is_open()) {
        return; // File doesn't exist yet
    }
    
    // Read header
    std::string header;
    std::getline(file, header);
    
    // Find column indices
    std::vector<std::string> headers;
    std::istringstream headerStream(header);
    std::string token;
    while (std::getline(headerStream, token, ',')) {
        headers.push_back(token);
    }
    
    int methodIdx = -1, nIdx = -1, dtIdx = -1, stepsIdx = -1, stepsPerSecIdx = -1;
    int candPerParticleIdx = -1, p50Idx = -1, p95Idx = -1;
    int energyMedianIdx = -1, energyMaxIdx = -1;
    int seedIdx = -1, boxWIdx = -1, boxHIdx = -1, radiusIdx = -1;
    
    for (size_t i = 0; i < headers.size(); ++i) {
        if (headers[i] == "method") methodIdx = i;
        else if (headers[i] == "N") nIdx = i;
        else if (headers[i] == "dt") dtIdx = i;
        else if (headers[i] == "steps") stepsIdx = i;
        else if (headers[i] == "steps_per_sec") stepsPerSecIdx = i;
        else if (headers[i] == "cand_per_particle") candPerParticleIdx = i;
        else if (headers[i] == "p50_ms") p50Idx = i;
        else if (headers[i] == "p95_ms") p95Idx = i;
        else if (headers[i] == "energy_drift_median") energyMedianIdx = i;
        else if (headers[i] == "energy_drift_max") energyMaxIdx = i;
        else if (headers[i] == "seed") seedIdx = i;
        else if (headers[i] == "box_w") boxWIdx = i;
        else if (headers[i] == "box_h") boxHIdx = i;
        else if (headers[i] == "radius") radiusIdx = i;
    }
    
    if (methodIdx < 0) return; // Invalid header
    
    std::string line;
    std::string lastLine;
    
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        std::vector<std::string> values;
        std::istringstream lineStream(line);
        while (std::getline(lineStream, token, ',')) {
            values.push_back(token);
        }
        
        if (values.size() > static_cast<size_t>(methodIdx) && values[methodIdx] == targetMethod) {
            lastLine = line;
        }
    }
    
    if (lastLine.empty()) return; // No entry found for other method
    // pasrsing the last line for the target method
    std::vector<std::string> values;
    std::istringstream lineStream(lastLine);
    while (std::getline(lineStream, token, ',')) {
        values.push_back(token);
    }
    
    if (values.size() < headers.size()) return;
    
    // get values from columns
    try {
        otherMethod_ = targetMethod;
        if (nIdx >= 0) otherResultsN_ = std::stoi(values[nIdx]);
        if (dtIdx >= 0) otherResultsDt_ = std::stof(values[dtIdx]);
        if (stepsIdx >= 0) otherResultsStep_ = std::stoi(values[stepsIdx]);
        if (stepsPerSecIdx >= 0) otherResultsStepsPerSec_ = std::stod(values[stepsPerSecIdx]);
        if (candPerParticleIdx >= 0) otherResultsAvgCandPairs_ = std::stod(values[candPerParticleIdx]);
        if (p50Idx >= 0) otherResultsP50_ = std::stod(values[p50Idx]);
        if (p95Idx >= 0) otherResultsP95_ = std::stod(values[p95Idx]);
        if (energyMedianIdx >= 0) otherResultsEnergyDriftMedian_ = std::stod(values[energyMedianIdx]);
        if (energyMaxIdx >= 0) otherResultsEnergyDriftMax_ = std::stod(values[energyMaxIdx]);
        if (seedIdx >= 0) otherResultsSeed_ = std::stoull(values[seedIdx]);
        if (boxWIdx >= 0) otherResultsBoxW_ = std::stof(values[boxWIdx]);
        if (boxHIdx >= 0) otherResultsBoxH_ = std::stof(values[boxHIdx]);
        if (radiusIdx >= 0) otherResultsRadius_ = std::stof(values[radiusIdx]);
        
        hasOtherMethod_ = true;
    } catch (...) {
        // Parsing error, ignore
        hasOtherMethod_ = false;
    }
}


// general event manager that checks for window close and key presses

void RenderWindow::handleEvents() {
    while (auto event = window_.pollEvent()) {
        if (event->is<sf::Event::Closed>()) {
            window_.close();
        } else if (event->is<sf::Event::KeyPressed>()) {
            auto keyEvent = event->getIf<sf::Event::KeyPressed>();
            if (keyEvent) {
                if (state_ == State::SIMULATION) {
                    if (keyEvent->code == sf::Keyboard::Key::Space) {
                        // Pause/resume (would need state management)
                    } else if (keyEvent->code == sf::Keyboard::Key::N) {
                        // N key to go to results page
                        nextButtonClicked_ = true;
                    }
                } else if (state_ == State::RESULTS) {
                    if (keyEvent->code == sf::Keyboard::Key::B) {
                        // B key to go back to simulation
                        backButtonPressed_ = true;
                        state_ = State::SIMULATION;
                    }
                }
            }
        }
    }
}

#endif // WITH_SFML

