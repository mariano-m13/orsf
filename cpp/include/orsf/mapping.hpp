#pragma once

#include "core.hpp"
#include "utils.hpp"
#include <string>
#include <map>
#include <vector>
#include <optional>
#include <functional>

namespace orsf {

// ============================================================================
// Field Mapping System
// ============================================================================

/// Direction of field mapping
enum class MappingDirection {
    ORSFToNative,      ///< ORSF -> Native game format
    NativeToORSF       ///< Native game format -> ORSF
};

/// Field mapping definition
struct FieldMapping {
    std::string orsf_path;              ///< ORSF field path (e.g., "setup.aero.front_wing")
    std::string native_key;             ///< Native format key
    std::optional<TransformFunc> to_native;   ///< Transform when going to native
    std::optional<TransformFunc> to_orsf;     ///< Transform when going to ORSF
    bool required;                      ///< Is this field required?

    FieldMapping(
        const std::string& orsf,
        const std::string& native,
        std::optional<TransformFunc> to_nat = std::nullopt,
        std::optional<TransformFunc> to_ors = std::nullopt,
        bool req = false
    ) : orsf_path(orsf), native_key(native), to_native(to_nat), to_orsf(to_ors), required(req) {}
};

/// Flat key-value representation (for native formats)
using FlatSetup = std::map<std::string, double>;

/// Mapping engine for ORSF <-> Native conversions
class MappingEngine {
public:
    /// Flatten ORSF to key-value pairs
    static FlatSetup flatten_orsf(const ORSF& orsf);

    /// Inflate ORSF from key-value pairs using template
    static ORSF inflate_orsf(const FlatSetup& flat, const ORSF& template_orsf);

    /// Apply field mappings to convert ORSF to native format
    static FlatSetup map_to_native(
        const ORSF& orsf,
        const std::vector<FieldMapping>& mappings
    );

    /// Apply field mappings to convert native format to ORSF
    static ORSF map_to_orsf(
        const FlatSetup& native,
        const std::vector<FieldMapping>& mappings,
        const ORSF& template_orsf
    );

    /// Get value from ORSF by path
    static std::optional<double> get_value(const ORSF& orsf, const std::string& path);

    /// Set value in ORSF by path
    static void set_value(ORSF& orsf, const std::string& path, double value);

private:
    // Helper to split path into components
    static std::vector<std::string> split_path(const std::string& path);

    // Flatten individual setup sections
    static void flatten_aero(const std::optional<Aerodynamics>& aero, FlatSetup& flat);
    static void flatten_suspension(const std::optional<Suspension>& susp, FlatSetup& flat);
    static void flatten_corner(const std::optional<CornerSuspension>& corner, const std::string& prefix, FlatSetup& flat);
    static void flatten_tires(const std::optional<Tires>& tires, FlatSetup& flat);
    static void flatten_drivetrain(const std::optional<Drivetrain>& drivetrain, FlatSetup& flat);
    static void flatten_gearing(const std::optional<Gearing>& gearing, FlatSetup& flat);
    static void flatten_brakes(const std::optional<Brakes>& brakes, FlatSetup& flat);
    static void flatten_electronics(const std::optional<Electronics>& electronics, FlatSetup& flat);
    static void flatten_fuel(const std::optional<Fuel>& fuel, FlatSetup& flat);

    // Helper to add optional value to flat map
    template<typename T>
    static void add_optional(FlatSetup& flat, const std::string& key, const std::optional<T>& value) {
        if (value.has_value()) {
            flat[key] = static_cast<double>(value.value());
        }
    }
};

} // namespace orsf
