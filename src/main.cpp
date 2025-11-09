#include "cli.hpp"
#include "sim_config.hpp"
#include "particle.hpp"
#include "physics.hpp"
#include "engine_quadtree.hpp"
#include "engine_hash.hpp"
#include "rng.hpp"
#include "metrics.hpp"
#include "csv.hpp"
#include <iostream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <memory>
#include <cstdlib>

#ifdef WITH_SFML
#include "render.hpp"
#endif

// Initialize particles with random non-overlapping positions
std::vector<Particle> initializeParticles(const SimConfig& config, RNG& rng) {
    std::vector<Particle> particles;
    particles.reserve(config.N);
    
    int attempts = 0;
    const int maxAttempts = 1000;
    
    for (int i = 0; i < config.N; ++i) {
        float x, y;
        bool valid = false;
        
        attempts = 0;
        while (!valid && attempts < maxAttempts) {
            x = rng.uniform(config.radius, config.box_w - config.radius);
            y = rng.uniform(config.radius, config.box_h - config.radius);
            
            valid = true;
            for (const auto& existing : particles) {
                float dx = x - existing.x;
                float dy = y - existing.y;
                float dist_sq = dx * dx + dy * dy;
                float r_sum = 2 * config.radius;
                if (dist_sq < r_sum * r_sum) {
                    valid = false;
                    break;
                }
            }
            attempts++;
        }
        
        if (!valid) {
            std::cerr << "Warning: Could not place particle " << i << " after " << maxAttempts << " attempts" << std::endl;
        }
        
        // Random velocity in bounded range (increased for more collisions)
        float speed = rng.uniform(400.0f, 600.0f);
        float angle = rng.uniform(0.0f, 2.0f * 3.14159265359f);
        float vx = speed * std::cos(angle);
        float vy = speed * std::sin(angle);
        
        particles.emplace_back(x, y, vx, vy, config.radius, i);
    }
    
    return particles;
}

void writeMetadata(const SimConfig& config, const std::string& outdir) {
    std::ostringstream oss;
    oss << outdir << "/run_meta.json";
    std::ofstream file(oss.str());
    
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    
    file << "{\n"
         << "  \"seed\": " << config.seed << ",\n"
         << "  \"N\": " << config.N << ",\n"
         << "  \"radius\": " << config.radius << ",\n"
         << "  \"box\": [" << config.box_w << ", " << config.box_h << "],\n"
         << "  \"dt\": " << config.dt << ",\n"
         << "  \"steps\": " << config.steps << ",\n"
         << "  \"method\": \"" << config.method << "\",\n"
         << "  \"start_time\": \"" << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S") << "\"\n"
         << "}\n";
}

int main(int argc, char* argv[]) {
    SimConfig config = CLI::parse(argc, argv);
    
    // Create output directory
    std::string cmd = "mkdir -p " + config.outdir;
    system(cmd.c_str());
    
    // Initialize RNG
    RNG rng(config.seed);
    
    // Initialize particles
    std::vector<Particle> particles = initializeParticles(config, rng);
    
    // Write metadata
    writeMetadata(config, config.outdir);
    
    // Create engine based on method
    std::unique_ptr<EngineQuadtree> engine_quadtree;
    std::unique_ptr<EngineHash> engine_hash;
    
    if (config.method == "quadtree") {
        engine_quadtree = std::make_unique<EngineQuadtree>(config.box_w, config.box_h, config.radius);
    } else if (config.method == "hash") {
        engine_hash = std::make_unique<EngineHash>(config.box_w, config.box_h, config.radius);
    } else {
        std::cerr << "Error: Unknown method: " << config.method << std::endl;
        return 1;
    }
    
    // Metrics
    Metrics metrics;
    metrics.setN(config.N);
    
    // CSV writers
    CSVWriter* stepsWriter = nullptr;
    CSVWriter* pairsWriter = nullptr;
    
    if (!config.summary_only) {
        std::ostringstream stepsFile;
        stepsFile << config.outdir << "/steps.csv";
        stepsWriter = new CSVWriter(stepsFile.str());
        stepsWriter->writeRow({"step", "id", "x", "y", "vx", "vy", "collided"});
        
        if (config.log_pairs) {
            std::ostringstream pairsFile;
            pairsFile << config.outdir << "/pairs.csv";
            pairsWriter = new CSVWriter(pairsFile.str());
            pairsWriter->writeRow({"step", "i", "j", "tested", "collided"});
        }
    }
    
    // Compute initial energy
    double initialEnergy = 0.0;
    if (!config.no_energy) {
        initialEnergy = static_cast<double>(physics::total_energy(particles));
    }
    
    // Track simulated time for energy recording (once per simulated second)
    double simulatedTime = 0.0;
    double lastEnergyRecordTime = 0.0;
    
    // Rendering setup
#ifdef WITH_SFML
    RenderWindow* renderWindow = nullptr;
    if (!config.headless) {
        renderWindow = new RenderWindow(config.box_w, config.box_h, config.method);
    }
#endif
    
    // Determine number of steps
    int totalSteps = config.steps;
    if (config.time_limit > 0.0f) {
        totalSteps = static_cast<int>(config.time_limit / config.dt);
    }
    
    // Simulation loop
    for (int step = 0; step < totalSteps; ++step) {
        // Begin step timing
        metrics.begin_step();
        
        // Step simulation
        if (config.method == "quadtree") {
            engine_quadtree->step(particles, config.dt);
        } else {
            engine_hash->step(particles, config.dt);
        }
        
        // Get candidate pairs checked this step
        uint32_t candidatePairs = 0;
        if (config.method == "quadtree") {
            candidatePairs = static_cast<uint32_t>(engine_quadtree->getCandidatePairsChecked());
        } else {
            candidatePairs = static_cast<uint32_t>(engine_hash->getCandidatePairsChecked());
        }
        
        // End step and record candidates
        metrics.end_step(candidatePairs);
        
        // Record collisions
        int collisions = 0;
        if (config.method == "quadtree") {
            collisions = engine_quadtree->getCollisionsThisStep();
        } else {
            collisions = engine_hash->getCollisionsThisStep();
        }
        metrics.recordCollisions(collisions);
        
        // Record energy once per simulated second
        if (!config.no_energy) {
            simulatedTime += config.dt;
            if (simulatedTime - lastEnergyRecordTime >= 1.0) {
                double currentEnergy = static_cast<double>(physics::total_energy(particles));
                metrics.record_energy(currentEnergy);
                lastEnergyRecordTime = simulatedTime;
            }
        }
        
        // Log step data (if not summary_only)
        if (!config.summary_only && stepsWriter) {
            for (const auto& p : particles) {
                std::vector<std::string> row = {
                    std::to_string(step),
                    std::to_string(p.id),
                    std::to_string(p.x),
                    std::to_string(p.y),
                    std::to_string(p.vx),
                    std::to_string(p.vy),
                    std::to_string(p.collided ? 1 : 0)
                };
                stepsWriter->writeRow(row);
            }
        }
        
        // Render
#ifdef WITH_SFML
        if (renderWindow && !config.headless) {
            if (!renderWindow->update(particles, metrics, step)) {
                break; // Window closed
            }
            
            // Check if Next button was clicked
            if (renderWindow->isNextButtonClicked() && renderWindow->getState() == RenderWindow::State::SIMULATION) {
                // Reset button state
                renderWindow->resetNextButton();
                
                // Finalize metrics before showing results
                double currentSimTime = step * config.dt;
                metrics.finalize(currentSimTime, initialEnergy);
                
                // Compute energy drift for display
                double finalEnergy = 0.0;
                double energyDrift = 0.0;
                if (!config.no_energy) {
                    finalEnergy = static_cast<double>(physics::total_energy(particles));
                    energyDrift = (finalEnergy - initialEnergy) / initialEnergy;
                }
                
                // Write summary CSV before loading other method
                std::ostringstream summaryFile;
                summaryFile << config.outdir << "/summary.csv";
                bool summaryExists = std::ifstream(summaryFile.str()).good();
                CSVWriter summaryWriter(summaryFile.str(), true);  // Append mode
                
                if (!summaryExists) {
                    summaryWriter.writeRow({"method", "N", "dt", "steps", "steps_per_sec", 
                                           "cand_per_particle", "p50_ms", "p95_ms", 
                                           "energy_drift_median", "energy_drift_max", 
                                           "seed", "box_w", "box_h", "radius"});
                }
                
                std::ostringstream energyMedianStr, energyMaxStr;
                if (config.no_energy) {
                    energyMedianStr << "0.0";
                    energyMaxStr << "0.0";
                } else {
                    energyMedianStr << std::scientific << std::setprecision(6) << metrics.energy_drift_median;
                    energyMaxStr << std::scientific << std::setprecision(6) << metrics.energy_drift_max;
                }
                
                std::vector<std::string> summaryRow = {
                    config.method,
                    std::to_string(config.N),
                    std::to_string(config.dt),
                    std::to_string(step),
                    std::to_string(metrics.steps_per_sec),
                    std::to_string(metrics.cand_per_particle),
                    std::to_string(metrics.p50_ms),
                    std::to_string(metrics.p95_ms),
                    energyMedianStr.str(),
                    energyMaxStr.str(),
                    std::to_string(config.seed),
                    std::to_string(config.box_w),
                    std::to_string(config.box_h),
                    std::to_string(config.radius)
                };
                summaryWriter.writeRow(summaryRow);
                summaryWriter.flush(); // Ensure it's written to disk
                
                // Show results screen
                renderWindow->showResults(metrics, step, config.N, config.dt, energyDrift,
                                        config.seed, config.box_w, config.box_h, config.radius);
                
                // Try to load other method's data from summary.csv (after writing)
                renderWindow->loadOtherMethodFromCSV(summaryFile.str());
                
                // Keep window open and show results screen
                while (renderWindow->isWindowOpen() && renderWindow->getState() == RenderWindow::State::RESULTS) {
                    if (!renderWindow->update(particles, metrics, step)) {
                        break;
                    }
                }
                
                // Check if back button was pressed to resume simulation
                if (renderWindow->isBackButtonPressed()) {
                    renderWindow->resetBackButton();
                    // Continue simulation loop instead of breaking
                    continue;
                }
                
                // Exit simulation loop if window closed or user wants to stay on results
                break;
            }
        }
#endif
    }
    
    // Finalize metrics
    double simTime = totalSteps * config.dt;
    metrics.finalize(simTime, initialEnergy);
    
    // Write summary CSV first (before showing results)
    std::ostringstream summaryFile;
    summaryFile << config.outdir << "/summary.csv";
    bool summaryExists = std::ifstream(summaryFile.str()).good();
    CSVWriter summaryWriter(summaryFile.str(), true);  // Append mode
    
    if (!summaryExists) {
        // Write header
        summaryWriter.writeRow({"method", "N", "dt", "steps", "steps_per_sec", 
                               "cand_per_particle", "p50_ms", "p95_ms", 
                               "energy_drift_median", "energy_drift_max", 
                               "seed", "box_w", "box_h", "radius"});
    }
    
    // Write summary row
    std::ostringstream energyMedianStr, energyMaxStr;
    if (config.no_energy) {
        energyMedianStr << "0.0";
        energyMaxStr << "0.0";
    } else {
        energyMedianStr << std::scientific << std::setprecision(6) << metrics.energy_drift_median;
        energyMaxStr << std::scientific << std::setprecision(6) << metrics.energy_drift_max;
    }
    
    std::vector<std::string> summaryRow = {
        config.method,
        std::to_string(config.N),
        std::to_string(config.dt),
        std::to_string(totalSteps),
        std::to_string(metrics.steps_per_sec),
        std::to_string(metrics.cand_per_particle),
        std::to_string(metrics.p50_ms),
        std::to_string(metrics.p95_ms),
        energyMedianStr.str(),
        energyMaxStr.str(),
        std::to_string(config.seed),
        std::to_string(config.box_w),
        std::to_string(config.box_h),
        std::to_string(config.radius)
    };
    summaryWriter.writeRow(summaryRow);
    summaryWriter.flush(); // Ensure it's written to disk
    
    // Show results screen if window is still open (after CSV is written)
#ifdef WITH_SFML
    if (renderWindow && !config.headless && renderWindow->isWindowOpen()) {
        double finalEnergy = 0.0;
        double energyDrift = 0.0;
        if (!config.no_energy) {
            finalEnergy = static_cast<double>(physics::total_energy(particles));
            energyDrift = (finalEnergy - initialEnergy) / initialEnergy;
        }
        renderWindow->showResults(metrics, totalSteps, config.N, config.dt, energyDrift,
                                 config.seed, config.box_w, config.box_h, config.radius);
        
        // Try to load other method's data from summary.csv (after writing current results)
        renderWindow->loadOtherMethodFromCSV(summaryFile.str());
        
        // Keep window open to show results
        while (renderWindow->isWindowOpen() && renderWindow->getState() == RenderWindow::State::RESULTS) {
            if (!renderWindow->update(particles, metrics, totalSteps)) {
                break;
            }
        }
    }
#endif
    
    // Print console summary (exact format specified)
    std::cout << std::fixed << std::setprecision(3);
    std::cout << "method=" << config.method
              << " N=" << config.N
              << " dt=" << config.dt
              << " steps=" << std::setprecision(0) << totalSteps
              << " steps_per_sec=" << std::setprecision(1) << metrics.steps_per_sec
              << " cand_per_particle=" << std::setprecision(2) << metrics.cand_per_particle
              << " p50_ms=" << std::setprecision(2) << metrics.p50_ms
              << " p95_ms=" << std::setprecision(2) << metrics.p95_ms;
    
    if (config.no_energy) {
        std::cout << " energy_drift_median=0.0 energy_drift_max=0.0";
    } else {
        std::cout << std::scientific << std::setprecision(1);
        std::cout << " energy_drift_median=" << metrics.energy_drift_median
                  << " energy_drift_max=" << metrics.energy_drift_max;
    }
    std::cout << std::endl;
    
    // Cleanup
    if (stepsWriter) {
        delete stepsWriter;
    }
    if (pairsWriter) {
        delete pairsWriter;
    }
    
#ifdef WITH_SFML
    if (renderWindow) {
        delete renderWindow;
    }
#endif
    
    return 0;
}
