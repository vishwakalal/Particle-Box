#pragma once

#include "particle.hpp"
#include <vector>

namespace physics {
    void integrate(std::vector<Particle>& particles, float dt);
    void handle_walls(std::vector<Particle>& particles, float box_w, float box_h, float r);
    bool circle_overlap(const Particle& a, const Particle& b);
    void resolve_collision(Particle& a, Particle& b);
    void positional_correction(Particle& a, Particle& b, float epsilon = 0.01f);
    float total_energy(const std::vector<Particle>& particles);
}


