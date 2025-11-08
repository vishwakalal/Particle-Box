#include "engine_hash.hpp"
#include <algorithm>

EngineHash::EngineHash(float box_w, float box_h, float r)
    : spatialHash_(std::max(2.0f * r, 1.0f)),
      box_w_(box_w), box_h_(box_h), r_(r), candidatePairsChecked_(0), collisionsThisStep_(0) {
}

void EngineHash::step(std::vector<Particle>& particles, float dt) {
    candidatePairsChecked_ = 0;
    collisionsThisStep_ = 0;
    
    // Reset collision flags
    for (auto& p : particles) {
        p.collided = false;
    }
    
    // Integrate
    physics::integrate(particles, dt);
    
    // Handle walls
    physics::handle_walls(particles, box_w_, box_h_, r_);
    
    // Build broad-phase
    buildBroadPhase(particles);
    
    // Narrow-phase collision detection and resolution
    narrowPhase(particles);
}

void EngineHash::buildBroadPhase(const std::vector<Particle>& particles) {
    spatialHash_.clear();
    for (const auto& p : particles) {
        BodyRef ref(p.id, p.x, p.y, p.r);
        spatialHash_.insert(ref);
    }
}

void EngineHash::narrowPhase(std::vector<Particle>& particles) {
    // Track pairs we've already processed this step
    std::vector<std::pair<int, int>> processedPairs;
    
    // Create ID to index map
    std::vector<int> idToIndex(particles.size());
    for (size_t i = 0; i < particles.size(); ++i) {
        idToIndex[particles[i].id] = i;
    }
    
    for (size_t i = 0; i < particles.size(); ++i) {
        auto& p = particles[i];
        
        // Query neighbors within 2*r radius
        std::vector<int> candidates;
        spatialHash_.query(p.x, p.y, 2.0f * r_, candidates);
        
        candidatePairsChecked_ += candidates.size();
        
        for (int j_id : candidates) {
            if (j_id <= static_cast<int>(p.id)) continue; // Avoid duplicate pairs
            
            // Check if already processed
            bool alreadyProcessed = false;
            for (const auto& pair : processedPairs) {
                if ((pair.first == p.id && pair.second == j_id) ||
                    (pair.first == j_id && pair.second == p.id)) {
                    alreadyProcessed = true;
                    break;
                }
            }
            if (alreadyProcessed) continue;
            
            if (j_id < 0 || j_id >= static_cast<int>(particles.size())) continue;
            int j_idx = idToIndex[j_id];
            auto& other = particles[j_idx];
            
            // Narrow-phase test
            if (physics::circle_overlap(p, other)) {
                physics::resolve_collision(p, other);
                physics::positional_correction(p, other);
                processedPairs.push_back({p.id, other.id});
                collisionsThisStep_++;
            }
        }
    }
}
