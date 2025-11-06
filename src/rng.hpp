#pragma once

#include <cstdint>
#include <random>

class RNG {
public:
    explicit RNG(uint64_t seed) : gen_(seed), seed_(seed) {}
    
    uint64_t seed() const { return seed_; }
    
    float uniform(float min = 0.0f, float max = 1.0f) {
        std::uniform_real_distribution<float> dist(min, max);
        return dist(gen_);
    }
    
    int uniform_int(int min, int max) {
        std::uniform_int_distribution<int> dist(min, max);
        return dist(gen_);
    }
    
private:
    std::mt19937_64 gen_;
    uint64_t seed_;
};
