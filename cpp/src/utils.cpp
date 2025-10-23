#include "orsf/utils.hpp"
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <stdexcept>
#include <cctype>
#include <regex>

namespace orsf {

// ============================================================================
// Unit Converter Implementation
// ============================================================================

double UnitConverter::convert(double value, Unit from, Unit to) {
    if (from == to) return value;

    // Convert to base unit first, then to target unit
    double base = to_base(value, from);
    return from_base(base, to);
}

double UnitConverter::to_base(double value, Unit unit) {
    switch (unit) {
        // Pressure (base: kPa)
        case Unit::KPA: return value;
        case Unit::PSI: return value * 6.89476;
        case Unit::BAR: return value * 100.0;

        // Spring rate (base: N/mm)
        case Unit::N_MM: return value;
        case Unit::LB_IN: return value * 0.175127;

        // Damping (base: N·s/m)
        case Unit::N_S_M: return value;
        case Unit::LB_S_IN: return value * 175.127;

        // Length (base: mm)
        case Unit::MM: return value;
        case Unit::INCHES: return value * 25.4;
        case Unit::CM: return value * 10.0;

        // Temperature (base: Celsius)
        case Unit::CELSIUS: return value;
        case Unit::FAHRENHEIT: return (value - 32.0) * 5.0 / 9.0;
        case Unit::KELVIN: return value - 273.15;

        // Torque (base: N·m)
        case Unit::NM: return value;
        case Unit::LB_FT: return value * 1.35582;

        // Force (base: Newtons)
        case Unit::NEWTONS: return value;
        case Unit::POUNDS: return value * 4.44822;

        // Speed (base: km/h)
        case Unit::KPH: return value;
        case Unit::MPH: return value * 1.60934;
        case Unit::MS: return value * 3.6;

        // Volume (base: Liters)
        case Unit::LITERS: return value;
        case Unit::GALLONS_US: return value * 3.78541;
        case Unit::GALLONS_UK: return value * 4.54609;

        default:
            throw std::runtime_error("Unknown unit in to_base conversion");
    }
}

double UnitConverter::from_base(double value, Unit unit) {
    switch (unit) {
        // Pressure
        case Unit::KPA: return value;
        case Unit::PSI: return value / 6.89476;
        case Unit::BAR: return value / 100.0;

        // Spring rate
        case Unit::N_MM: return value;
        case Unit::LB_IN: return value / 0.175127;

        // Damping
        case Unit::N_S_M: return value;
        case Unit::LB_S_IN: return value / 175.127;

        // Length
        case Unit::MM: return value;
        case Unit::INCHES: return value / 25.4;
        case Unit::CM: return value / 10.0;

        // Temperature
        case Unit::CELSIUS: return value;
        case Unit::FAHRENHEIT: return value * 9.0 / 5.0 + 32.0;
        case Unit::KELVIN: return value + 273.15;

        // Torque
        case Unit::NM: return value;
        case Unit::LB_FT: return value / 1.35582;

        // Force
        case Unit::NEWTONS: return value;
        case Unit::POUNDS: return value / 4.44822;

        // Speed
        case Unit::KPH: return value;
        case Unit::MPH: return value / 1.60934;
        case Unit::MS: return value / 3.6;

        // Volume
        case Unit::LITERS: return value;
        case Unit::GALLONS_US: return value / 3.78541;
        case Unit::GALLONS_UK: return value / 4.54609;

        default:
            throw std::runtime_error("Unknown unit in from_base conversion");
    }
}

double UnitConverter::clamp(double value, double min, double max, double step) {
    double clamped = std::max(min, std::min(max, value));
    if (step > 0.0) {
        clamped = round_to_step(clamped, step);
    }
    return clamped;
}

double UnitConverter::round_to_step(double value, double step) {
    if (step <= 0.0) return value;
    return std::round(value / step) * step;
}

// ============================================================================
// Lookup Table Converter Implementation
// ============================================================================

LookupTableConverter::LookupTableConverter(std::vector<LUTEntry> table)
    : table_(std::move(table)) {
    // Sort by input value
    std::sort(table_.begin(), table_.end(),
        [](const LUTEntry& a, const LUTEntry& b) { return a.input < b.input; });
}

double LookupTableConverter::interpolate(double value) const {
    if (table_.empty()) {
        throw std::runtime_error("Empty lookup table");
    }

    // Clamp to table bounds
    if (value <= table_.front().input) return table_.front().output;
    if (value >= table_.back().input) return table_.back().output;

    // Find surrounding entries
    for (size_t i = 0; i < table_.size() - 1; ++i) {
        if (value >= table_[i].input && value <= table_[i + 1].input) {
            return lerp(value,
                table_[i].input, table_[i + 1].input,
                table_[i].output, table_[i + 1].output);
        }
    }

    return table_.back().output;
}

double LookupTableConverter::reverse_lookup(double value) const {
    if (table_.empty()) {
        throw std::runtime_error("Empty lookup table");
    }

    // Create reverse table (sorted by output)
    std::vector<LUTEntry> reverse_table;
    for (const auto& entry : table_) {
        reverse_table.push_back({entry.output, entry.input});
    }
    std::sort(reverse_table.begin(), reverse_table.end(),
        [](const LUTEntry& a, const LUTEntry& b) { return a.input < b.input; });

    // Clamp to bounds
    if (value <= reverse_table.front().input) return reverse_table.front().output;
    if (value >= reverse_table.back().input) return reverse_table.back().output;

    // Find surrounding entries
    for (size_t i = 0; i < reverse_table.size() - 1; ++i) {
        if (value >= reverse_table[i].input && value <= reverse_table[i + 1].input) {
            return lerp(value,
                reverse_table[i].input, reverse_table[i + 1].input,
                reverse_table[i].output, reverse_table[i + 1].output);
        }
    }

    return reverse_table.back().output;
}

double LookupTableConverter::lerp(double x, double x0, double x1, double y0, double y1) {
    if (std::abs(x1 - x0) < 1e-10) return y0;
    return y0 + (y1 - y0) * (x - x0) / (x1 - x0);
}

// ============================================================================
// Transform Functions
// ============================================================================

TransformFunc Transform::identity() {
    return [](double x) { return x; };
}

TransformFunc Transform::scale(double factor) {
    return [factor](double x) { return x * factor; };
}

TransformFunc Transform::offset(double amount) {
    return [amount](double x) { return x + amount; };
}

TransformFunc Transform::linear(double scale_factor, double offset_amount) {
    return [scale_factor, offset_amount](double x) {
        return x * scale_factor + offset_amount;
    };
}

TransformFunc Transform::invert() {
    return [](double x) {
        if (std::abs(x) < 1e-10) {
            throw std::runtime_error("Cannot invert zero value");
        }
        return 1.0 / x;
    };
}

TransformFunc Transform::negate() {
    return [](double x) { return -x; };
}

TransformFunc Transform::clamp(double min, double max) {
    return [min, max](double x) {
        return UnitConverter::clamp(x, min, max);
    };
}

TransformFunc Transform::percent_to_ratio() {
    return [](double x) { return x / 100.0; };
}

TransformFunc Transform::ratio_to_percent() {
    return [](double x) { return x * 100.0; };
}

TransformFunc Transform::unit_convert(Unit from, Unit to) {
    return [from, to](double x) {
        return UnitConverter::convert(x, from, to);
    };
}

TransformFunc Transform::lookup_table(const LookupTableConverter& lut) {
    return [lut](double x) {
        return lut.interpolate(x);
    };
}

TransformFunc Transform::compose(const std::vector<TransformFunc>& transforms) {
    return [transforms](double x) {
        double result = x;
        for (const auto& transform : transforms) {
            result = transform(result);
        }
        return result;
    };
}

// ============================================================================
// String Utilities
// ============================================================================

std::string StringUtils::trim(const std::string& str) {
    auto start = std::find_if_not(str.begin(), str.end(), ::isspace);
    auto end = std::find_if_not(str.rbegin(), str.rend(), ::isspace).base();
    return (start < end) ? std::string(start, end) : std::string();
}

std::string StringUtils::to_lower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}

std::string StringUtils::to_upper(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), ::toupper);
    return result;
}

std::vector<std::string> StringUtils::split(const std::string& str, char delim) {
    std::vector<std::string> parts;
    std::stringstream ss(str);
    std::string part;
    while (std::getline(ss, part, delim)) {
        parts.push_back(part);
    }
    return parts;
}

std::string StringUtils::join(const std::vector<std::string>& parts, const std::string& delim) {
    if (parts.empty()) return "";

    std::ostringstream oss;
    oss << parts[0];
    for (size_t i = 1; i < parts.size(); ++i) {
        oss << delim << parts[i];
    }
    return oss.str();
}

bool StringUtils::starts_with(const std::string& str, const std::string& prefix) {
    return str.size() >= prefix.size() &&
           str.compare(0, prefix.size(), prefix) == 0;
}

bool StringUtils::ends_with(const std::string& str, const std::string& suffix) {
    return str.size() >= suffix.size() &&
           str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

std::string StringUtils::replace_all(
    const std::string& str,
    const std::string& from,
    const std::string& to
) {
    if (from.empty()) return str;

    std::string result = str;
    size_t pos = 0;
    while ((pos = result.find(from, pos)) != std::string::npos) {
        result.replace(pos, from.length(), to);
        pos += to.length();
    }
    return result;
}

// ============================================================================
// Date/Time Utilities
// ============================================================================

std::string DateTimeUtils::now_iso8601() {
    auto now = std::time(nullptr);
    return unix_to_iso8601(now);
}

bool DateTimeUtils::is_valid_iso8601(const std::string& timestamp) {
    // Basic ISO8601 regex pattern (simplified)
    // Matches: YYYY-MM-DDTHH:MM:SS(.sss)?(Z|[+-]HH:MM)?
    std::regex iso8601_pattern(
        R"(\d{4}-\d{2}-\d{2}T\d{2}:\d{2}:\d{2}(\.\d{3})?(Z|[+-]\d{2}:\d{2})?)"
    );
    return std::regex_match(timestamp, iso8601_pattern);
}

int64_t DateTimeUtils::iso8601_to_unix(const std::string& timestamp) {
    // Simplified implementation - production code should use a proper date library
    std::tm tm = {};
    std::istringstream ss(timestamp);
    ss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");

    if (ss.fail()) {
        throw std::runtime_error("Invalid ISO8601 timestamp: " + timestamp);
    }

    return static_cast<int64_t>(std::mktime(&tm));
}

std::string DateTimeUtils::unix_to_iso8601(int64_t unix_time) {
    std::time_t time = static_cast<std::time_t>(unix_time);
    std::tm* tm = std::gmtime(&time);

    std::ostringstream oss;
    oss << std::put_time(tm, "%Y-%m-%dT%H:%M:%SZ");
    return oss.str();
}

} // namespace orsf
