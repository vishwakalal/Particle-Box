#include "cli.hpp"
#include <iostream>
#include <sstream>
#include <cstdlib>

SimConfig CLI::parse(int argc, char* argv[]) {
    SimConfig config;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--method" && i + 1 < argc) {
            config.method = argv[++i];
        } else if (arg == "--N" && i + 1 < argc) {
            config.N = parse_int(argv[++i]);
        } else if (arg == "--radius" && i + 1 < argc) {
            config.radius = parse_float(argv[++i]);
        } else if (arg == "--box" && i + 1 < argc) {
            auto parts = split_string(argv[++i], 'x');
            if (parts.size() == 2) {
                config.box_w = parse_float(parts[0]);
                config.box_h = parse_float(parts[1]);
            }
        } else if (arg == "--dt" && i + 1 < argc) {
            config.dt = parse_float(argv[++i]);
        } else if (arg == "--steps" && i + 1 < argc) {
            config.steps = parse_int(argv[++i]);
        } else if (arg == "--time_limit" && i + 1 < argc) {
            config.time_limit = parse_float(argv[++i]);
        } else if (arg == "--seed" && i + 1 < argc) {
            config.seed = parse_uint64(argv[++i]);
        } else if (arg == "--headless") {
            config.headless = true;
        } else if (arg == "--outdir" && i + 1 < argc) {
            config.outdir = argv[++i];
        } else if (arg == "--log_pairs") {
            config.log_pairs = true;
        } else if (arg == "--no_energy") {
            config.no_energy = true;
        } else if (arg == "--summary_only") {
            config.summary_only = true;
        } else if (arg == "--help" || arg == "-h") {
            print_usage(argv[0]);
            std::exit(0);
        }
    }
    
    return config;
}

void CLI::print_usage(const char* progname) {
    std::cout << "Usage: " << progname << " [options]\n"
              << "Options:\n"
              << "  --method {quadtree|hash}     Broad-phase method (default: quadtree)\n"
              << "  --N <int>                    Number of particles (default: 100)\n"
              << "  --radius <float>             Particle radius (default: 3.0)\n"
              << "  --box <W>x<H>                Box dimensions (default: 1200x800)\n"
              << "  --dt <float>                 Timestep (default: 0.002)\n"
              << "  --steps <int>                Total steps (default: 1000)\n"
              << "  --time_limit <float>         Alternative to --steps (seconds)\n"
              << "  --seed <uint64>              RNG seed (default: 1337)\n"
              << "  --headless                   No rendering window\n"
              << "  --outdir <path>              Output directory (required)\n"
              << "  --log_pairs                  Log candidate pairs\n"
              << "  --no_energy                  Skip energy calculations\n"
              << "  --summary_only               Only write summary.csv, no per-step logs\n"
              << "  --help, -h                   Show this help\n";
}

std::vector<std::string> CLI::split_string(const std::string& s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        result.push_back(item);
    }
    return result;
}

float CLI::parse_float(const std::string& s) {
    return std::stof(s);
}

int CLI::parse_int(const std::string& s) {
    return std::stoi(s);
}

uint64_t CLI::parse_uint64(const std::string& s) {
    return std::stoull(s);
}

