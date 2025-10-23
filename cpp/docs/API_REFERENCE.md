# ORSF C++ API Reference

Complete API reference for the ORSF C++ library.

## Table of Contents

- [Core Data Structures](#core-data-structures)
- [Validation](#validation)
- [Unit Conversion](#unit-conversion)
- [Transformations](#transformations)
- [Mapping Engine](#mapping-engine)
- [Adapter System](#adapter-system)
- [Utilities](#utilities)

---

## Core Data Structures

### ORSF

Main structure representing a complete racing setup.

```cpp
struct ORSF {
    std::string schema;                              // Must be "orsf://v1"
    Metadata metadata;
    Car car;
    std::optional<Context> context;
    Setup setup;
    std::optional<std::map<std::string, json>> compat;

    // Methods
    static ORSF from_json(const std::string& json_str);
    static ORSF from_json(const json& j);
    std::string to_json_string(int indent = -1) const;
    json to_json() const;
};
```

### Metadata

Setup identification and tracking information.

```cpp
struct Metadata {
    std::string id;                                  // Required: Unique identifier
    std::string name;                                // Required: Human-readable name
    std::optional<std::string> notes;
    std::string created_at;                          // Required: ISO8601 timestamp
    std::optional<std::string> updated_at;           // ISO8601 timestamp
    std::optional<std::string> created_by;
    std::optional<std::vector<std::string>> tags;
    std::optional<std::string> source;
    std::optional<std::string> origin_sim;
};
```

### Car

Vehicle identification.

```cpp
struct Car {
    std::string make;                                // Required: Manufacturer
    std::string model;                               // Required: Model name
    std::optional<std::string> variant;
    std::optional<std::string> car_class;            // GT3/GTE/LMP2/LMDh/GT4/TCR/F1/etc.
    std::optional<std::string> bop_id;
};
```

### Context

Environmental and session information.

```cpp
struct Context {
    std::optional<std::string> track;
    std::optional<std::string> layout;
    std::optional<double> ambient_temp_c;            // Celsius
    std::optional<double> track_temp_c;              // Celsius
    std::optional<std::string> rubber;               // green/low/medium/high/saturated
    std::optional<double> wetness;                   // 0.0 (dry) to 1.0 (wet)
    std::optional<std::string> session_type;         // practice/qualifying/race
    std::optional<std::string> fuel_rule;
};
```

### Setup Subsystems

#### Aerodynamics

```cpp
struct Aerodynamics {
    std::optional<double> front_wing;
    std::optional<double> rear_wing;
    std::optional<double> front_downforce_n;         // Newtons
    std::optional<double> rear_downforce_n;          // Newtons
    std::optional<double> front_ride_height_mm;      // Millimeters
    std::optional<double> rear_ride_height_mm;
    std::optional<double> rake_mm;
    std::optional<double> brake_duct_front_pct;      // 0-100%
    std::optional<double> brake_duct_rear_pct;
    std::optional<double> radiator_opening_pct;
};
```

#### CornerSuspension

Per-corner suspension configuration (FL/FR/RL/RR).

```cpp
struct CornerSuspension {
    std::optional<double> camber_deg;                // Degrees
    std::optional<double> toe_deg;
    std::optional<double> caster_deg;                // Front only
    std::optional<double> spring_rate_n_mm;          // N/mm
    std::optional<double> ride_height_mm;
    std::optional<double> bumpstop_gap_mm;
    std::optional<double> bumpstop_rate_n_mm;
    std::optional<double> packer_mm;
    std::optional<double> damper_bump_slow_n_s_m;    // N·s/m
    std::optional<double> damper_bump_fast_n_s_m;
    std::optional<double> damper_rebound_slow_n_s_m;
    std::optional<double> damper_rebound_fast_n_s_m;
};
```

#### Suspension

```cpp
struct Suspension {
    std::optional<CornerSuspension> front_left;
    std::optional<CornerSuspension> front_right;
    std::optional<CornerSuspension> rear_left;
    std::optional<CornerSuspension> rear_right;
    std::optional<double> front_arb;
    std::optional<double> rear_arb;
    std::optional<double> heave_spring_n_mm;
    std::optional<double> heave_packer_mm;
};
```

#### Tires

```cpp
struct Tires {
    std::optional<std::string> compound;
    std::optional<double> pressure_fl_kpa;           // kPa
    std::optional<double> pressure_fr_kpa;
    std::optional<double> pressure_rl_kpa;
    std::optional<double> pressure_rr_kpa;
    std::optional<double> stagger_mm;
};
```

#### Other Subsystems

```cpp
struct Drivetrain { /* ... */ };
struct Gearing { /* ... */ };
struct Brakes { /* ... */ };
struct Electronics { /* ... */ };
struct Fuel { /* ... */ };
struct Strategy { /* ... */ };
```

---

## Validation

### Validator

Static validation class for ORSF structures.

```cpp
class Validator {
public:
    // Main validation entry point
    static std::vector<ValidationError> validate(const ORSF& orsf);

    // Section-specific validation
    static void validate_schema(const ORSF& orsf, std::vector<ValidationError>& errors);
    static void validate_metadata(const Metadata& metadata, std::vector<ValidationError>& errors);
    static void validate_car(const Car& car, std::vector<ValidationError>& errors);
    static void validate_context(const std::optional<Context>& context, std::vector<ValidationError>& errors);
    static void validate_setup(const Setup& setup, std::vector<ValidationError>& errors);
    static void validate_cross_field(const ORSF& orsf, std::vector<ValidationError>& errors);
};
```

### ValidationError

```cpp
struct ValidationError {
    ValidationSeverity severity;                     // Error/Warning/Info
    ValidationCode code;                             // Error code
    std::string field;                               // Field path
    std::string message;                             // Human-readable message
    std::optional<std::string> expected;
    std::optional<std::string> actual;

    std::string to_string() const;
};
```

### ValidationSeverity

```cpp
enum class ValidationSeverity {
    Error,      // Critical error, setup is invalid
    Warning,    // Non-critical issue, may work but unusual
    Info        // Informational note
};
```

### ValidationCode

```cpp
enum class ValidationCode {
    Required,           // Required field missing
    OutOfRange,         // Value outside acceptable range
    InvalidFormat,      // Value format incorrect
    Incompatible,       // Values incompatible with each other
    Deprecated,         // Field deprecated
    SchemaInvalid       // Schema version invalid
};
```

---

## Unit Conversion

### UnitConverter

Static utility class for unit conversions.

```cpp
class UnitConverter {
public:
    // Convert value from one unit to another
    static double convert(double value, Unit from, Unit to);

    // Clamp value to range with optional step precision
    static double clamp(double value, double min, double max, double step = 0.0);

    // Round to nearest step value
    static double round_to_step(double value, double step);
};
```

### Unit

Supported unit types.

```cpp
enum class Unit {
    // Pressure
    KPA, PSI, BAR,

    // Spring rate
    N_MM, LB_IN,

    // Damping
    N_S_M, LB_S_IN,

    // Length
    MM, INCHES, CM,

    // Temperature
    CELSIUS, FAHRENHEIT, KELVIN,

    // Torque
    NM, LB_FT,

    // Force
    NEWTONS, POUNDS,

    // Speed
    KPH, MPH, MS,

    // Volume
    LITERS, GALLONS_US, GALLONS_UK
};
```

**Example:**

```cpp
double psi = UnitConverter::convert(170.0, Unit::KPA, Unit::PSI);
double clamped = UnitConverter::clamp(52.3, 0.0, 100.0, 5.0);  // 50.0
```

---

## Transformations

### Transform

Factory for transformation functions.

```cpp
class Transform {
public:
    static TransformFunc identity();
    static TransformFunc scale(double factor);
    static TransformFunc offset(double amount);
    static TransformFunc linear(double scale, double offset);
    static TransformFunc invert();
    static TransformFunc negate();
    static TransformFunc clamp(double min, double max);
    static TransformFunc percent_to_ratio();
    static TransformFunc ratio_to_percent();
    static TransformFunc unit_convert(Unit from, Unit to);
    static TransformFunc lookup_table(const LookupTableConverter& lut);
    static TransformFunc compose(const std::vector<TransformFunc>& transforms);
};
```

**Example:**

```cpp
auto transform = Transform::compose({
    Transform::scale(2.0),
    Transform::offset(10.0)
});

double result = transform(5.0);  // (5 * 2) + 10 = 20
```

### LookupTableConverter

Non-linear mapping with linear interpolation.

```cpp
class LookupTableConverter {
public:
    explicit LookupTableConverter(std::vector<LUTEntry> table);

    double interpolate(double value) const;
    double reverse_lookup(double value) const;

    const std::vector<LUTEntry>& get_table() const;
};

struct LUTEntry {
    double input;
    double output;
};
```

**Example:**

```cpp
std::vector<LUTEntry> table = {
    {0.0, 0.0},
    {50.0, 25.0},
    {100.0, 75.0}
};

LookupTableConverter lut(table);
double output = lut.interpolate(25.0);         // 12.5
double input = lut.reverse_lookup(50.0);       // 75.0
```

---

## Mapping Engine

### MappingEngine

Convert between ORSF and flat key-value representations.

```cpp
class MappingEngine {
public:
    // Flatten ORSF to key-value pairs
    static FlatSetup flatten_orsf(const ORSF& orsf);

    // Inflate ORSF from key-value pairs
    static ORSF inflate_orsf(const FlatSetup& flat, const ORSF& template_orsf);

    // Apply field mappings
    static FlatSetup map_to_native(const ORSF& orsf, const std::vector<FieldMapping>& mappings);
    static ORSF map_to_orsf(const FlatSetup& native, const std::vector<FieldMapping>& mappings, const ORSF& template_orsf);

    // Get/set values by path
    static std::optional<double> get_value(const ORSF& orsf, const std::string& path);
    static void set_value(ORSF& orsf, const std::string& path, double value);
};
```

### FieldMapping

Defines mapping between ORSF and native format fields.

```cpp
struct FieldMapping {
    std::string orsf_path;                           // "setup.aero.front_wing"
    std::string native_key;                          // "front_aero"
    std::optional<TransformFunc> to_native;          // ORSF -> Native
    std::optional<TransformFunc> to_orsf;            // Native -> ORSF
    bool required;                                   // Is field required?
};
```

**Example:**

```cpp
FieldMapping mapping(
    "setup.tires.pressure_fl_kpa",
    "tire_fl_psi",
    Transform::unit_convert(Unit::KPA, Unit::PSI),
    Transform::unit_convert(Unit::PSI, Unit::KPA),
    false
);

std::vector<FieldMapping> mappings = {mapping, /* ... */};
FlatSetup native = MappingEngine::map_to_native(orsf, mappings);
```

---

## Adapter System

### Adapter

Abstract base class for game-specific adapters.

```cpp
class Adapter {
public:
    virtual ~Adapter() = default;

    virtual std::string get_id() const = 0;
    virtual std::string get_version() const = 0;
    virtual std::string get_car_key() const = 0;
    virtual std::string get_suggested_filename() const = 0;

    virtual std::vector<uint8_t> orsf_to_native(const ORSF& orsf) const = 0;
    virtual ORSF native_to_orsf(const std::vector<uint8_t>& data) const = 0;

    virtual std::vector<ValidationError> validate_orsf(const ORSF& orsf) const = 0;
    virtual std::string get_file_extension() const = 0;
    virtual std::optional<std::string> get_install_path() const = 0;
    virtual std::vector<FieldMapping> get_field_mappings() const = 0;

    struct Metadata {
        std::string id, version, car_key, description, author;
    };
    virtual Metadata get_metadata() const = 0;
};
```

### BaseAdapter

Convenience base class with common functionality.

```cpp
class BaseAdapter : public Adapter {
public:
    BaseAdapter(
        const std::string& id,
        const std::string& version,
        const std::string& car_key,
        const std::string& description = "",
        const std::string& author = ""
    );

    std::vector<ValidationError> validate_orsf(const ORSF& orsf) const override;

protected:
    FlatSetup orsf_to_flat(const ORSF& orsf) const;
    ORSF flat_to_orsf(const FlatSetup& flat, const ORSF& template_orsf) const;
};
```

### AdapterRegistry

Thread-safe singleton registry for adapters.

```cpp
class AdapterRegistry {
public:
    static AdapterRegistry& instance();

    void register_adapter(std::shared_ptr<Adapter> adapter);

    std::shared_ptr<Adapter> resolve(
        const std::string& id,
        const std::string& version = "",
        const std::string& car_key = ""
    ) const;

    std::vector<std::shared_ptr<Adapter>> get_all_adapters() const;
    std::vector<std::shared_ptr<Adapter>> get_adapters_for_game(const std::string& id) const;

    void unregister_adapter(const std::string& id, const std::string& version, const std::string& car_key);
    void clear();
};
```

**Example:**

```cpp
// Create adapter
class MyAdapter : public BaseAdapter {
    // ... implementation
};

// Register
auto& registry = AdapterRegistry::instance();
auto adapter = std::make_shared<MyAdapter>();
registry.register_adapter(adapter);

// Resolve and use
auto resolved = registry.resolve("mygame");
auto data = resolved->orsf_to_native(setup);
```

---

## Utilities

### DateTimeUtils

ISO8601 timestamp utilities.

```cpp
class DateTimeUtils {
public:
    static std::string now_iso8601();
    static bool is_valid_iso8601(const std::string& timestamp);
    static int64_t iso8601_to_unix(const std::string& timestamp);
    static std::string unix_to_iso8601(int64_t unix_time);
};
```

### StringUtils

String manipulation utilities.

```cpp
class StringUtils {
public:
    static std::string trim(const std::string& str);
    static std::string to_lower(const std::string& str);
    static std::string to_upper(const std::string& str);
    static std::vector<std::string> split(const std::string& str, char delim);
    static std::string join(const std::vector<std::string>& parts, const std::string& delim);
    static bool starts_with(const std::string& str, const std::string& prefix);
    static bool ends_with(const std::string& str, const std::string& suffix);
    static std::string replace_all(const std::string& str, const std::string& from, const std::string& to);
};
```

---

## Type Aliases

```cpp
using json = nlohmann::json;
using FlatSetup = std::map<std::string, double>;
using TransformFunc = std::function<double(double)>;
```

---

## Constants

```cpp
namespace orsf {
    constexpr const char* VERSION = "1.0.0";
    constexpr const char* SCHEMA_VERSION = "orsf://v1";
}
```

---

## Error Handling

All parsing and conversion functions throw `std::runtime_error` on failure:

```cpp
try {
    ORSF setup = ORSF::from_json(invalid_json);
} catch (const std::runtime_error& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}
```

---

## Thread Safety

- **Thread-safe**: `AdapterRegistry` (uses mutex)
- **Immutable/Stateless**: `ORSF`, `Validator`, `MappingEngine`, `UnitConverter`, `Transform`, `DateTimeUtils`, `StringUtils`
- **Custom adapters**: Should be stateless for thread safety

---

## Best Practices

1. **Always validate** before using setups in production
2. **Use optional fields** for non-required data
3. **Prefer BaseAdapter** over raw Adapter interface
4. **Register adapters** at startup, resolve at runtime
5. **Handle exceptions** when parsing JSON
6. **Use transformations** for unit conversions in mappings
7. **Test mappings** with round-trip conversions
8. **Version your adapters** appropriately

---

For more examples, see the `examples/` directory.
