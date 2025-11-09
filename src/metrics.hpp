#pragma once

#include <vector>
#include <chrono>
#include <cstdint>

struct StepSample {
    double ms;
    uint32_t candidates_checked;
};

struct Metrics {
    Metrics();
    
    void begin_step();                    // Start timer for current step
    void end_step(uint32_t candidates);  // Stop timer, record candidates checked this step
    void record_energy(double E);        // Optional energy log (per simulated second)
    
    void finalize(double sim_time_seconds, double E0);  // Compute percentiles, averages, drift
    
    // Computed at finalize():
    double steps_per_sec = 0.0;
    double p50_ms = 0.0;
    double p95_ms = 0.0;
    double cand_per_particle = 0.0;
    double energy_drift_median = 0.0;
    double energy_drift_max = 0.0;
    
    // Accessors for compatibility
    int getTotalCollisions() const { return totalCollisions_; }
    void recordCollisions(int collisions) { totalCollisions_ += collisions; }
    void setN(int N) { N_ = N; }
    
private:
    std::vector<StepSample> samples_;
    std::vector<double> energy_samples_;
    std::vector<double> energy_drift_samples_;
    
    int totalSteps_;
    int totalCollisions_;
    uint64_t totalCandidatesChecked_;
    int N_;  // Number of particles (set during finalize)
    
    std::chrono::high_resolution_clock::time_point stepStartTime_;
    std::chrono::high_resolution_clock::time_point runStartTime_;
    std::chrono::high_resolution_clock::time_point runEndTime_;
    
    double percentile(const std::vector<double>& sorted, double p) const;
};