#ifndef SIM_CONFIG_HPP
#define SIM_CONFIG_HPP

#include <string>
#include <cstdint>

struct SimConfig {
    std::string method = "quadtree";  //"quadtree" or "hash"
    int N = 100;                      //number of particles
    float radius = 5.0f;              //particle radius
    float box_w = 800.0f;              //box width
    float box_h = 600.0f;              //box height
    float dt = 0.002f;                //timestep
    int steps = 1000;                 //total steps
    float time_limit = -1.0f;         //alternative to steps (seconds)
    uint64_t seed = 1337;             //RNG seed
    bool headless = false;            //no rendering
    std::string outdir = ".";         //output directory
    bool log_pairs = false;           //log candidate pairs
    bool no_energy = false;           //skip energy calculations
    bool summary_only = false;         //only write summary.csv, no per-step logs
};

#endif

