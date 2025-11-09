#include "metrics.hpp"
#include <algorithm>
#include <numeric>
#include <cmath>

Metrics::Metrics() 
    : totalSteps_(0), totalCollisions_(0), totalCandidatesChecked_(0), N_(0) {
    runStartTime_ = std::chrono::high_resolution_clock::now();
}

void Metrics::begin_step() {
    stepStartTime_ = std::chrono::high_resolution_clock::now();
}

void Metrics::end_step(uint32_t candidates) {
    auto stepEndTime = std::chrono::high_resolution_clock::now();
    auto stepDuration = std::chrono::duration_cast<std::chrono::microseconds>(
        stepEndTime - stepStartTime_);
    double ms = stepDuration.count() / 1000.0;
    
    StepSample sample;
    sample.ms = ms;
    sample.candidates_checked = candidates;
    samples_.push_back(sample);
    
    totalSteps_++;
    totalCandidatesChecked_ += candidates;
    runEndTime_ = stepEndTime;
}

void Metrics::record_energy(double E) {
    energy_samples_.push_back(E);
}

void Metrics::finalize(double sim_time_seconds, double E0) {
    if (samples_.empty()) {
        return;
    }
    
    // Compute steps_per_sec from wall-clock time
    auto wallDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
        runEndTime_ - runStartTime_).count();
    if (wallDuration > 0) {
        steps_per_sec = (totalSteps_ * 1000.0) / wallDuration;
    }
    
    // Compute latency percentiles
    std::vector<double> stepTimes;
    stepTimes.reserve(samples_.size());
    for (const auto& sample : samples_) {
        stepTimes.push_back(sample.ms);
    }
    std::sort(stepTimes.begin(), stepTimes.end());
    
    if (!stepTimes.empty()) {
        // p50: floor(0.5 * (n-1))
        int idx50 = static_cast<int>(std::floor(0.5 * (stepTimes.size() - 1)));
        p50_ms = stepTimes[idx50];
        
        // p95: floor(0.95 * (n-1))
        int idx95 = static_cast<int>(std::floor(0.95 * (stepTimes.size() - 1)));
        p95_ms = stepTimes[idx95];
    }
    
    // Compute cand_per_particle = total candidates / (N * steps)
    // Note: N_ needs to be set, but we'll compute from samples if needed
    if (totalSteps_ > 0 && N_ > 0) {
        cand_per_particle = static_cast<double>(totalCandidatesChecked_) / (N_ * totalSteps_);
    }
    
    // Compute energy drift if energy samples available
    if (!energy_samples_.empty() && E0 > 0.0) {
        energy_drift_samples_.clear();
        for (double E : energy_samples_) {
            double drift = (E - E0) / E0;
            energy_drift_samples_.push_back(std::abs(drift));
        }
        
        std::sort(energy_drift_samples_.begin(), energy_drift_samples_.end());
        
        if (!energy_drift_samples_.empty()) {
            // Median
            int idx_median = energy_drift_samples_.size() / 2;
            energy_drift_median = energy_drift_samples_[idx_median];
            
            // Max
            energy_drift_max = energy_drift_samples_.back();
        }
    }
}