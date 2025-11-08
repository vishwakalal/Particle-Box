# Particle Box Simulation

A C++ project that simulates a 2D gas of many moving particles in a box with elastic particle to particle and particle to wall collisions.

## Compiling

### Prerequisites

- C++17 compatible compiler
- CMake 3.10+
- SFML 3.0+

### Run Instructions

```bash
mkdir build

cd build

cmake .. -DWITH_SFML=ON

cmake --build . --target particle-box


#For QuadTree:
./particle-box --method quadtree --N 200 --dt 0.002 --steps 5000 --seed 1337 --outdir ../runs/comparison
method=quadtree N=200 dt=0.002 steps=5000 steps_per_sec=44.9 cand_per_particle=1.04 p50_ms=0.99 p95_ms=1.15 energy_drift_median=0.0e+00 energy_drift_max=0.0e+00s

#For Spatial Hash:
./particle-box --method hash --N 200 --dt 0.002 --steps 5000 --seed 1337 --outdir ../runs/comparison
method=quadtree N=200 dt=0.002 steps=5000 steps_per_sec=44.9 cand_per_particle=1.04 p50_ms=0.99 p95_ms=1.15 energy_drift_median=0.0e+00 energy_drift_max=0.0e+00s
```

### Controls (SFML Window)

- **N**: Go to results page (shows metrics)
- **B**: Go back from results to simulation
- Run once as a Quadtree, once as a Spatial Hash, and then one more time for the results page to be accurate.

## Usage

### Command Line Options

- `--method {quadtree|hash}`: Broad-phase method (default: quadtree)
- `--N <int>`: Number of particles (default: 100)
- `--radius <float>`: Particle radius (default: 3.0)
- `--box <W>x<H>`: Box dimensions (default: 1200x800)
- `--dt <float>`: Fixed timestep (default: 0.002)
- `--steps <int>`: Total steps to run (default: 1000)
- `--time_limit <float>`: Alternative to --steps (seconds)
- `--seed <uint64>`: RNG seed (default: 1337)
- `--headless`: No rendering window
- `--outdir <path>`: CSV output directory (required)
- `--log_pairs`: Also log tested candidate pairs
- `--no_energy`: Skip energy calculations
- `--summary_only`: Only write summary.csv, no per-step logs
- `--help, -h`: Show help message

## Metrics

The simulation outputs:

- Steps per second
- Average candidate pairs checked per particle per step
- P50/P95 step time (ms)
- Energy drift (relative to initial energy)
