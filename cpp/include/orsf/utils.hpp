#pragma once

#include <string>
#include <map>
#include <vector>
#include <functional>
#include <cmath>

namespace orsf {

// ============================================================================
// Unit Conversion
// ============================================================================

/// Unit types for conversion
enum class Unit {
    // Pressure
    KPA,            ///< Kilopascals
    PSI,            ///< Pounds per square inch
    BAR,            ///< Bar

    // Spring rate
    N_MM,           ///< Newtons per millimeter
    LB_IN,          ///< Pounds per inch

    // Damping
    N_S_M,          ///< Newton-seconds per meter
    LB_S_IN,        ///< Pound-seconds per inch

    // Length
    MM,             ///< Millimeters
    INCHES,         ///< Inches
    CM,             ///< Centimeters

    // Temperature
    CELSIUS,        ///< Degrees Celsius
    FAHRENHEIT,     ///< Degrees Fahrenheit
    KELVIN,         ///< Kelvin

    // Torque
    NM,             ///< Newton-meters
    LB_FT,          ///< Pound-feet

    // Force
    NEWTONS,        ///< Newtons
    POUNDS,         ///< Pounds-force

    // Speed
    KPH,            ///< Kilometers per hour
    MPH,            ///< Miles per hour
    MS,             ///< Meters per second

    // Volume
    LITERS,         ///< Liters
    GALLONS_US,     ///< US Gallons
    GALLONS_UK      ///< UK Gallons
};

/// Unit converter utility
class UnitConverter {
public:
    /// Convert value from one unit to another
    static double convert(double value, Unit from, Unit to);

    /// Clamp value to range with optional step precision
    static double clamp(double value, double min, double max, double step = 0.0);

    /// Round to nearest step value
    static double round_to_step(double value, double step);

private:
    static double to_base(double value, Unit unit);
    static double from_base(double value, Unit unit);
};

// ============================================================================
// Lookup Table Converter
// ============================================================================

/// Lookup table entry (input -> output mapping)
struct LUTEntry {
    double input;
    double output;

    LUTEntry(double in, double out) : input(in), output(out) {}
};

/// Lookup table converter with linear interpolation
class LookupTableConverter {
public:
    /// Create converter from lookup table
    explicit LookupTableConverter(std::vector<LUTEntry> table);

    /// Interpolate value using the lookup table
    double interpolate(double value) const;

    /// Reverse lookup (find input for given output)
    double reverse_lookup(double value) const;

    /// Get the lookup table
    const std::vector<LUTEntry>& get_table() const { return table_; }

private:
    std::vector<LUTEntry> table_;

    // Linear interpolation between two points
    static double lerp(double x, double x0, double x1, double y0, double y1);
};

// ============================================================================
// Transformation Functions
// ============================================================================

/// Transformation type for field mapping
using TransformFunc = std::function<double(double)>;

/// Common transformation functions
class Transform {
public:
    /// Identity transform (no change)
    static TransformFunc identity();

    /// Scale by constant factor
    static TransformFunc scale(double factor);

    /// Add constant offset
    static TransformFunc offset(double amount);

    /// Linear transform (scale then offset)
    static TransformFunc linear(double scale, double offset);

    /// Invert value (1/x)
    static TransformFunc invert();

    /// Negate value (-x)
    static TransformFunc negate();

    /// Clamp to range
    static TransformFunc clamp(double min, double max);

    /// Map percentage (0-100) to ratio (0-1)
    static TransformFunc percent_to_ratio();

    /// Map ratio (0-1) to percentage (0-100)
    static TransformFunc ratio_to_percent();

    /// Unit conversion transform
    static TransformFunc unit_convert(Unit from, Unit to);

    /// Lookup table transform
    static TransformFunc lookup_table(const LookupTableConverter& lut);

    /// Compose multiple transforms (apply in order)
    static TransformFunc compose(const std::vector<TransformFunc>& transforms);
};

// ============================================================================
// String Utilities
// ============================================================================

/// String utilities for ORSF
class StringUtils {
public:
    /// Trim whitespace from string
    static std::string trim(const std::string& str);

    /// Convert string to lowercase
    static std::string to_lower(const std::string& str);

    /// Convert string to uppercase
    static std::string to_upper(const std::string& str);

    /// Split string by delimiter
    static std::vector<std::string> split(const std::string& str, char delim);

    /// Join strings with delimiter
    static std::string join(const std::vector<std::string>& parts, const std::string& delim);

    /// Check if string starts with prefix
    static bool starts_with(const std::string& str, const std::string& prefix);

    /// Check if string ends with suffix
    static bool ends_with(const std::string& str, const std::string& suffix);

    /// Replace all occurrences of substring
    static std::string replace_all(
        const std::string& str,
        const std::string& from,
        const std::string& to
    );
};

// ============================================================================
// Date/Time Utilities
// ============================================================================

/// Date/time utilities for ISO8601
class DateTimeUtils {
public:
    /// Get current timestamp in ISO8601 format
    static std::string now_iso8601();

    /// Validate ISO8601 timestamp
    static bool is_valid_iso8601(const std::string& timestamp);

    /// Parse ISO8601 to Unix timestamp
    static int64_t iso8601_to_unix(const std::string& timestamp);

    /// Format Unix timestamp to ISO8601
    static std::string unix_to_iso8601(int64_t unix_time);
};

} // namespace orsf
