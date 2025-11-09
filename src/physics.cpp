#include "physics.hpp"
#include <algorithm>
#include <cmath>

namespace physics {

void integrate(std::vector<Particle>& particles, float dt) {
    for (auto& p : particles) {
        p.x += p.vx * dt;
        p.y += p.vy * dt;
    }
}

void handle_walls(std::vector<Particle>& particles, float box_w, float box_h, float r) {
    for (auto& p : particles) {
        //left wall
        if (p.x - r < 0.0f) {
            p.x = r;
            p.vx = -p.vx;
            p.collided = true;
        }
        //right wall
        if (p.x + r > box_w) {
            p.x = box_w - r;
            p.vx = -p.vx;
            p.collided = true;
        }
        //bottom wall
        if (p.y - r < 0.0f) {
            p.y = r;
            p.vy = -p.vy;
            p.collided = true;
        }
        //top wall
        if (p.y + r > box_h) {
            p.y = box_h - r;
            p.vy = -p.vy;
            p.collided = true;
        }
    }
}

bool circle_overlap(const Particle& a, const Particle& b) {
    float dx = a.x - b.x;
    float dy = a.y - b.y;
    float dist_sq = dx * dx + dy * dy;
    float r_sum = a.r + b.r;
    return dist_sq < r_sum * r_sum;
}

void resolve_collision(Particle& a, Particle& b) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float dist_sq = dx * dx + dy * dy;
    
    if (dist_sq < 1e-10f) {
        dx = 1.0f;
        dy = 0.0f;
        dist_sq = 1.0f;
    }
    
    float dist = std::sqrt(dist_sq);
    float nx = dx / dist;
    float ny = dy / dist;
    
    float dvx = b.vx - a.vx;
    float dvy = b.vy - a.vy;
    float dvn = dvx * nx + dvy * ny;
    
    float impulse = dvn;
    
    a.vx += impulse * nx;
    a.vy += impulse * ny;
    b.vx -= impulse * nx;
    b.vy -= impulse * ny;
    
    a.collided = true;
    b.collided = true;
}

void positional_correction(Particle& a, Particle& b, float epsilon) {
    float dx = b.x - a.x;
    float dy = b.y - a.y;
    float dist_sq = dx * dx + dy * dy;
    float r_sum = a.r + b.r;
    
    if (dist_sq < r_sum * r_sum && dist_sq > 1e-10f) {
        float dist = std::sqrt(dist_sq);
        float overlap = r_sum - dist;
        
        if (overlap > epsilon) {
            float nx = dx / dist;
            float ny = dy / dist;
            
            float correction = overlap * 0.5f * epsilon;
            a.x -= correction * nx;
            a.y -= correction * ny;
            b.x += correction * nx;
            b.y += correction * ny;
        }
    }
}

float total_energy(const std::vector<Particle>& particles) {
    float energy = 0.0f;
    for (const auto& p : particles) {
        energy += 0.5f * (p.vx * p.vx + p.vy * p.vy);
    }
    return energy;
}

}

