#pragma once

#include "particle.hpp"
#include "quadtree.hpp"
#include "physics.hpp"
#include <vector>

class EngineQuadtree {
public:
    EngineQuadtree(float box_w, float box_h, float r);
    
    void step(std::vector<Particle>& particles, float dt);
    
    // metricss
    int getCandidatePairsChecked() const { return candidatePairsChecked_; }
    int getCollisionsThisStep() const { return collisionsThisStep_; }
    void resetMetrics() { candidatePairsChecked_ = 0; collisionsThisStep_ = 0; }
    
private:
    Quadtree quadtree_;
    float box_w_, box_h_, r_;
    int candidatePairsChecked_;
    int collisionsThisStep_;
    
    void buildBroadPhase(const std::vector<Particle>& particles);
    void narrowPhase(std::vector<Particle>& particles);
};

