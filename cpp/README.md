# ORSF C++ Library

**Open Racing Setup Format (ORSF)** - A universal, JSON-based car setup format for seamless setup exchange across different racing simulations.

[![Version](https://img.shields.io/badge/version-1.0.0-blue.svg)](https://github.com/mariano-m13/orsf)
[![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](https://en.cppreference.com/w/cpp/17)
[![License](https://img.shields.io/badge/license-MIT-green.svg)](LICENSE)

## Features

- **Modern C++17** implementation with zero dependencies (except nlohmann/json)
- **Comprehensive data model** supporting all major racing series (GT3, GTE, LMP2, F1, etc.)
- **Robust validation** with detailed error reporting
- **Unit conversion** (kPa↔PSI, N/mm↔lb/in, °C↔°F, etc.)
- **Flexible mapping engine** for converting between ORSF and native game formats
- **Pluggable adapter system** for game-specific implementations
- **Thread-safe adapter registry**
- **Header-only option** for easy integration
- **Comprehensive test suite** (Catch2)
- **Production-ready** with extensive validation and error handling

## Quick Start

### Installation

#### Option 1: CMake Integration (Recommended)

```cmake
# Add to your CMakeLists.txt
include(FetchContent)

FetchContent_Declare(
    orsf
    GIT_REPOSITORY https://github.com/mariano-m13/orsf.git
    GIT_TAG v1.0.0
)
FetchContent_MakeAvailable(orsf)

target_link_libraries(your_target PRIVATE orsf)
```

#### Option 2: Build from Source

```bash
git clone https://github.com/mariano-m13/orsf.git
cd orsf/cpp
mkdir build && cd build
cmake ..
cmake --build .
cmake --install .
```

### Basic Usage

```cpp
#include <orsf/orsf.hpp>

using namespace orsf;

int main() {
    // Create a new setup
    ORSF setup;

    // Set metadata
    setup.metadata.id = "spa-quali-2024";
    setup.metadata.name = "Spa Qualifying Setup";
    setup.metadata.created_at = DateTimeUtils::now_iso8601();

    // Set car information
    setup.car.make = "Porsche";
    setup.car.model = "911 GT3 R";
    setup.car.car_class = "GT3";

    // Set aerodynamics
    setup.setup.aero = Aerodynamics{};
    setup.setup.aero->front_wing = 2;
    setup.setup.aero->rear_wing = 5;

    // Set tire pressures
    setup.setup.tires = Tires{};
    setup.setup.tires->pressure_fl_kpa = 170.0;
    setup.setup.tires->pressure_fr_kpa = 170.0;

    // Validate
    auto errors = Validator::validate(setup);
    if (!errors.empty()) {
        for (const auto& error : errors) {
            std::cerr << error.to_string() << std::endl;
        }
    }

    // Serialize to JSON
    std::string json = setup.to_json_string(2);
    std::cout << json << std::endl;

    // Parse from JSON
    ORSF parsed = ORSF::from_json(json);

    return 0;
}
```

## Core Components

### 1. Data Model

The ORSF format consists of five main sections:

```cpp
struct ORSF {
    std::string schema;                  // "orsf://v1"
    Metadata metadata;                   // Setup identification
    Car car;                            // Vehicle information
    std::optional<Context> context;     // Track/environmental data
    Setup setup;                        // Actual setup parameters
    std::optional<std::map<std::string, json>> compat;  // Sim-specific data
};
```

#### Setup Subsystems

- **Aerodynamics**: Wings, ride height, brake ducts, radiator
- **Suspension**: Per-corner camber, toe, springs, dampers, ARBs
- **Tires**: Compound, pressures (FL/FR/RL/RR), stagger
- **Drivetrain**: Differential settings, final drive
- **Gearing**: Gear ratios, reverse ratio
- **Brakes**: Bias, pad compound, disc type
- **Electronics**: TC, ABS, engine map, pit limiter
- **Fuel**: Start fuel, consumption, stint planning

### 2. Validation

Comprehensive validation with three severity levels:

```cpp
auto errors = Validator::validate(setup);

for (const auto& error : errors) {
    switch (error.severity) {
        case ValidationSeverity::Error:   // Critical - setup invalid
        case ValidationSeverity::Warning: // Non-critical - may work
        case ValidationSeverity::Info:    // Informational
    }
}
```

**Validation checks include:**
- Required fields (ID, name, make, model, created_at)
- Range validation (tire pressure: 50-400 kPa, brake bias: 0-100%, etc.)
- ISO8601 timestamp format
- Temperature consistency (ambient vs track temp)
- Positive values (spring rates, gear ratios, etc.)
- Percentage bounds (0-100%)

### 3. Unit Conversion

Built-in support for common racing units:

```cpp
// Pressure
double psi = UnitConverter::convert(170.0, Unit::KPA, Unit::PSI);

// Spring rate
double lb_in = UnitConverter::convert(100.0, Unit::N_MM, Unit::LB_IN);

// Temperature
double fahrenheit = UnitConverter::convert(20.0, Unit::CELSIUS, Unit::FAHRENHEIT);

// Length
double inches = UnitConverter::convert(50.8, Unit::MM, Unit::INCHES);

// With clamping and step precision
double clamped = UnitConverter::clamp(52.3, 0.0, 100.0, 5.0);  // = 50.0
```

**Supported units:**
- Pressure: kPa, PSI, Bar
- Spring rate: N/mm, lb/in
- Damping: N·s/m, lb·s/in
- Length: mm, inches, cm
- Temperature: °C, °F, Kelvin
- Torque: N·m, lb·ft
- Force: Newtons, pounds
- Speed: km/h, mph, m/s
- Volume: Liters, US gallons, UK gallons

### 4. Transformations

Flexible transformation pipeline for data mapping:

```cpp
// Simple transformations
auto scale2x = Transform::scale(2.0);
auto plus5 = Transform::offset(5.0);
auto linear = Transform::linear(2.0, 3.0);  // 2x + 3

// Percentage/ratio conversion
auto pct_to_ratio = Transform::percent_to_ratio();  // 75% -> 0.75
auto ratio_to_pct = Transform::ratio_to_percent();  // 0.75 -> 75%

// Unit conversion
auto kpa_to_psi = Transform::unit_convert(Unit::KPA, Unit::PSI);

// Composed transformations
auto composed = Transform::compose({
    Transform::scale(2.0),
    Transform::offset(10.0)
});
```

### 5. Lookup Table Converter

For non-linear transformations (e.g., wing level to downforce):

```cpp
std::vector<LUTEntry> wing_table = {
    {0.0, 0.0},      // Level 0 = 0N downforce
    {1.0, 500.0},
    {2.0, 1200.0},
    {3.0, 2000.0},
    {5.0, 3900.0}
};

LookupTableConverter lut(wing_table);

double downforce = lut.interpolate(2.5);        // Forward lookup
double wing_level = lut.reverse_lookup(1600.0); // Reverse lookup
```

### 6. Mapping Engine

Convert between ORSF and native game formats:

```cpp
// Flatten ORSF to key-value pairs
FlatSetup flat = MappingEngine::flatten_orsf(setup);

// Define field mappings
std::vector<FieldMapping> mappings = {
    FieldMapping(
        "setup.tires.pressure_fl_kpa",  // ORSF field
        "tire_fl_psi",                  // Native key
        Transform::unit_convert(Unit::KPA, Unit::PSI),  // To native
        Transform::unit_convert(Unit::PSI, Unit::KPA),  // To ORSF
        false  // Not required
    )
};

// Convert ORSF -> Native
FlatSetup native = MappingEngine::map_to_native(setup, mappings);

// Convert Native -> ORSF
ORSF converted = MappingEngine::map_to_orsf(native, mappings, template_setup);
```

### 7. Adapter System

Create game-specific adapters:

```cpp
class MyGameAdapter : public BaseAdapter {
public:
    MyGameAdapter() : BaseAdapter("mygame", "1.0", "gt3", "My Racing Game", "Author") {}

    std::vector<uint8_t> orsf_to_native(const ORSF& orsf) const override {
        // Convert ORSF to your game's format
        FlatSetup native = orsf_to_flat(orsf);
        // ... serialize to your format
    }

    ORSF native_to_orsf(const std::vector<uint8_t>& data) const override {
        // Parse your game's format
        // ... convert to ORSF
    }

    std::vector<FieldMapping> get_field_mappings() const override {
        return {
            FieldMapping("setup.aero.front_wing", "front_aero", ...),
            // ... more mappings
        };
    }

    // ... implement other methods
};

// Register adapter
auto& registry = AdapterRegistry::instance();
auto adapter = std::make_shared<MyGameAdapter>();
registry.register_adapter(adapter);

// Resolve and use
auto resolved = registry.resolve("mygame", "1.0", "gt3");
auto native_data = resolved->orsf_to_native(setup);
```

## Examples

See the `examples/` directory for complete examples:

- **basic_usage.cpp** - Creating, serializing, and parsing ORSF
- **validation_example.cpp** - Comprehensive validation demonstrations
- **conversion_example.cpp** - Unit conversion and transformations
- **adapter_example.cpp** - Creating and using custom adapters

Build and run examples:

```bash
cd build
./examples/basic_usage
./examples/validation_example
./examples/conversion_example
./examples/adapter_example
```

## Testing

Run the comprehensive test suite:

```bash
cd build
ctest --verbose

# Or run directly
./tests/orsf_tests
```

Tests cover:
- Core data model serialization/deserialization
- Validation logic (all severity levels)
- Unit conversions
- Transformations and lookup tables
- Mapping engine
- Adapter system

## Building Options

```bash
# Standard build
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Header-only mode
cmake -B build -DORSF_HEADER_ONLY=ON

# Disable tests
cmake -B build -DORSF_BUILD_TESTS=OFF

# Disable examples
cmake -B build -DORSF_BUILD_EXAMPLES=OFF

# Install
cmake --install build --prefix /usr/local
```

## API Documentation

### Key Classes

- **`ORSF`** - Main setup structure
- **`Validator`** - Setup validation
- **`UnitConverter`** - Unit conversions
- **`Transform`** - Data transformations
- **`LookupTableConverter`** - Non-linear mapping
- **`MappingEngine`** - ORSF ↔ Native conversion
- **`Adapter`** - Game adapter interface
- **`AdapterRegistry`** - Thread-safe adapter registry
- **`DateTimeUtils`** - ISO8601 timestamp utilities
- **`StringUtils`** - String manipulation helpers

### Namespaces

All ORSF components are in the `orsf` namespace:

```cpp
using namespace orsf;
```

## Integration with Game Engines

### Unreal Engine

```cpp
// In your UStruct
UPROPERTY()
FString OrsfJson;

// Parse
ORSF setup = ORSF::from_json(std::string(TCHAR_TO_UTF8(*OrsfJson)));

// Serialize
FString json = FString(setup.to_json_string().c_str());
```

### Unity (via C# wrapper or native plugin)

```cpp
// Export C API for P/Invoke
extern "C" {
    EXPORT_API const char* orsf_to_json(const ORSF* setup);
    EXPORT_API ORSF* orsf_from_json(const char* json);
}
```

## Performance

- **Zero-copy** JSON parsing (nlohmann/json)
- **Header-only option** for compile-time optimization
- **Thread-safe** adapter registry with mutex locks
- **Minimal allocations** for flat map conversions
- **Fast validation** with early exit on critical errors

## Thread Safety

- `AdapterRegistry` is **thread-safe** (uses mutex)
- `ORSF`, `Validator`, `MappingEngine`, `UnitConverter` are **immutable** (safe for concurrent reads)
- Adapters should be **stateless** for thread safety

## Error Handling

All parsing and conversion functions throw `std::runtime_error` on failure:

```cpp
try {
    ORSF setup = ORSF::from_json(json_str);
} catch (const std::runtime_error& e) {
    std::cerr << "Parse error: " << e.what() << std::endl;
}
```

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure all tests pass
5. Submit a pull request

## License

MIT License - see [LICENSE](../LICENSE) for details.

## Support

- **Issues**: https://github.com/mariano-m13/orsf/issues
- **Discussions**: https://github.com/mariano-m13/orsf/discussions
- **Documentation**: https://github.com/mariano-m13/orsf/wiki

## Roadmap

- [ ] Additional adapters (iRacing, Assetto Corsa Competizione, rFactor 2)
- [ ] Setup comparison utilities
- [ ] Telemetry integration
- [ ] Web assembly (WASM) build
- [ ] Python bindings (pybind11)
- [ ] Rust bindings (cxx)

## Acknowledgments

- **nlohmann/json** - Excellent JSON library for C++
- **Catch2** - Modern C++ test framework
- Racing simulation community for feedback and testing

---

**Made with ❤️ for the simracing community**
