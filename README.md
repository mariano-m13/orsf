 ORSF v1 - Final Specification

  ORSF is your universal, JSON-based car setup format designed for seamless setup exchange across different racing
  simulations.

  Core Characteristics

  - Schema: orsf://v1
  - Format: JSON with ISO8601 dates
  - Units: SI-based (kPa, mm, °C, N/mm, N·m, etc.)
  - Extension: .orsf or .json
  - Encoding: UTF-8

  Top-Level Structure

  {
    "schema": "orsf://v1",
    "metadata": { /* Setup identification */ },
    "car": { /* Vehicle identification */ },
    "context": { /* Track/environmental data */ },
    "setup": { /* Actual setup parameters */ },
    "compat": { /* Optional sim-specific data */ }
  }

  Main Sections

  1. Metadata - Setup tracking
  - id, name, notes, created_at, updated_at, created_by
  - tags, source, origin_sim

  2. Car - Vehicle identification
  - make, model, variant, class (GT3/GTE/LMP2/LMDh/GT4/TCR/F3/F4/F1)
  - bop_id

  3. Context - Environmental/session info
  - track, layout, ambient_temp_c, track_temp_c
  - rubber (green/low/medium/high/saturated), wetness (0-1)
  - session_type, fuel_rule

  4. Setup - Main configuration with subsystems:

  4.1 Aerodynamics

  - Downforce (front/rear in Newtons)
  - Ride height, rake (mm)
  - Brake ducts, radiator opening (%)
  - Wing levels, brake bias

  4.2 Suspension (per-corner: FL/FR/RL/RR)

  - Camber, toe, caster (degrees)
  - Spring rates (N/mm)
  - Bumpstops (gap, rate)
  - Dampers (low/high speed bump & rebound in N·s/m)
  - ARBs, heave springs, packers

  4.3 Tires

  - Compound name
  - Pressures per wheel (kPa): {fl, fr, rl, rr}
  - Stagger (mm)

  4.4 Drivetrain

  - Diff preload (N·m)
  - Power/coast ramp (%)
  - Final drive ratio
  - LSD clutch plates

  4.5 Gearing

  - Gear ratios array
  - Reverse ratio

  4.6 Brakes

  - Pad compound, disc type
  - Brake bias

  4.7 Electronics

  - TC/ABS levels
  - Engine map, brake level
  - Pit limiter speed

  4.8 Fuel

  - Start fuel, per-lap consumption
  - Stint target laps

  4.9 Strategy (optional)

  - Tire change policy
  - Notes

    📁 For game developers (C++)

  cpp/
  ├── CMakeLists.txt              # Root build config
  ├── README.md                   # Main documentation
  ├── .gitignore
  ├── include/orsf/              # Public API (6 headers)
  ├── src/                       # Implementation (5 files)
  ├── tests/                     # 100+ test cases
  ├── examples/                  # 4 example programs
  └── docs/                      # API reference, build guide

  ---
  🚀 Next Steps

  1. Test the build:
  cd cpp && mkdir build && cd build && cmake .. && cmake --build . && ctest --verbose
  2. Try the examples:
  ./examples/basic_usage
  ./examples/validation_example
  3. Integrate into your Raceday app:
    - Use CMake FetchContent or add as subdirectory
    - Create adapters for each sim you support
  4. Commit to your repo (remember: use mariano-m13 account, no Claude/Anthropic mentions in commits)

  ---
  🎁 Bonus Features

  - Lookup table converter - For non-linear mappings (wing level → downforce)
  - ISO8601 utilities - Timestamp generation & validation
  - String utilities - Trim, split, join, case conversion
  - Transformation composition - Chain multiple transforms
  - Detailed error messages - Field path, expected vs actual values

