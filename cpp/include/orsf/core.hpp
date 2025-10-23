#pragma once

#include <string>
#include <vector>
#include <map>
#include <optional>
#include <nlohmann/json.hpp>

namespace orsf {

using json = nlohmann::json;

} // namespace orsf

// ============================================================================
// nlohmann/json support for std::optional
// ============================================================================

namespace nlohmann {
    template <typename T>
    struct adl_serializer<std::optional<T>> {
        static void to_json(json& j, const std::optional<T>& opt) {
            if (opt.has_value()) {
                j = opt.value();
            } else {
                j = nullptr;
            }
        }

        static void from_json(const json& j, std::optional<T>& opt) {
            if (j.is_null()) {
                opt = std::nullopt;
            } else {
                opt = j.get<T>();
            }
        }
    };
}

namespace orsf {

// ============================================================================
// Core Data Structures
// ============================================================================

/// Metadata - Setup identification and tracking
struct Metadata {
    std::string id;                                  ///< Unique identifier (UUID recommended)
    std::string name;                                ///< Human-readable setup name
    std::optional<std::string> notes;                ///< Optional setup notes
    std::string created_at;                          ///< ISO8601 timestamp
    std::optional<std::string> updated_at;           ///< ISO8601 timestamp
    std::optional<std::string> created_by;           ///< Creator name/identifier
    std::optional<std::vector<std::string>> tags;    ///< Tags for categorization
    std::optional<std::string> source;               ///< Source of setup (e.g., "coach_dave")
    std::optional<std::string> origin_sim;           ///< Original simulator ID

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Metadata,
        id, name, notes, created_at, updated_at, created_by, tags, source, origin_sim)
};

/// Car - Vehicle identification
struct Car {
    std::string make;                                ///< Manufacturer (e.g., "Porsche")
    std::string model;                               ///< Model name (e.g., "911 GT3 R")
    std::optional<std::string> variant;              ///< Variant/year (e.g., "2023")
    std::optional<std::string> car_class;            ///< GT3/GTE/LMP2/LMDh/GT4/TCR/F3/F4/F1
    std::optional<std::string> bop_id;               ///< Balance of Performance identifier

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Car,
        make, model, variant, car_class, bop_id)
};

/// Context - Environmental and session information
struct Context {
    std::optional<std::string> track;                ///< Track name
    std::optional<std::string> layout;               ///< Track layout/configuration
    std::optional<double> ambient_temp_c;            ///< Ambient temperature (Celsius)
    std::optional<double> track_temp_c;              ///< Track surface temperature (Celsius)
    std::optional<std::string> rubber;               ///< green/low/medium/high/saturated
    std::optional<double> wetness;                   ///< 0.0 (dry) to 1.0 (fully wet)
    std::optional<std::string> session_type;         ///< practice/qualifying/race
    std::optional<std::string> fuel_rule;            ///< Fuel rule description

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Context,
        track, layout, ambient_temp_c, track_temp_c, rubber, wetness, session_type, fuel_rule)
};

// ============================================================================
// Setup Subsystems
// ============================================================================

/// Aerodynamics configuration
struct Aerodynamics {
    std::optional<double> front_wing;                ///< Front wing setting (context-dependent)
    std::optional<double> rear_wing;                 ///< Rear wing setting (context-dependent)
    std::optional<double> front_downforce_n;         ///< Front downforce (Newtons)
    std::optional<double> rear_downforce_n;          ///< Rear downforce (Newtons)
    std::optional<double> front_ride_height_mm;      ///< Front ride height (millimeters)
    std::optional<double> rear_ride_height_mm;       ///< Rear ride height (millimeters)
    std::optional<double> rake_mm;                   ///< Rake angle (millimeters)
    std::optional<double> brake_duct_front_pct;      ///< Front brake duct opening (0-100%)
    std::optional<double> brake_duct_rear_pct;       ///< Rear brake duct opening (0-100%)
    std::optional<double> radiator_opening_pct;      ///< Radiator opening (0-100%)

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Aerodynamics,
        front_wing, rear_wing, front_downforce_n, rear_downforce_n,
        front_ride_height_mm, rear_ride_height_mm, rake_mm,
        brake_duct_front_pct, brake_duct_rear_pct, radiator_opening_pct)
};

/// Per-corner suspension data (FL/FR/RL/RR)
struct CornerSuspension {
    std::optional<double> camber_deg;                ///< Camber angle (degrees)
    std::optional<double> toe_deg;                   ///< Toe angle (degrees)
    std::optional<double> caster_deg;                ///< Caster angle (degrees, front only)
    std::optional<double> spring_rate_n_mm;          ///< Spring rate (N/mm)
    std::optional<double> ride_height_mm;            ///< Ride height (millimeters)
    std::optional<double> bumpstop_gap_mm;           ///< Bumpstop gap (millimeters)
    std::optional<double> bumpstop_rate_n_mm;        ///< Bumpstop rate (N/mm)
    std::optional<double> packer_mm;                 ///< Packer thickness (millimeters)
    std::optional<double> damper_bump_slow_n_s_m;    ///< Slow bump damping (N·s/m)
    std::optional<double> damper_bump_fast_n_s_m;    ///< Fast bump damping (N·s/m)
    std::optional<double> damper_rebound_slow_n_s_m; ///< Slow rebound damping (N·s/m)
    std::optional<double> damper_rebound_fast_n_s_m; ///< Fast rebound damping (N·s/m)

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(CornerSuspension,
        camber_deg, toe_deg, caster_deg, spring_rate_n_mm, ride_height_mm,
        bumpstop_gap_mm, bumpstop_rate_n_mm, packer_mm,
        damper_bump_slow_n_s_m, damper_bump_fast_n_s_m,
        damper_rebound_slow_n_s_m, damper_rebound_fast_n_s_m)
};

/// Suspension configuration
struct Suspension {
    std::optional<CornerSuspension> front_left;
    std::optional<CornerSuspension> front_right;
    std::optional<CornerSuspension> rear_left;
    std::optional<CornerSuspension> rear_right;
    std::optional<double> front_arb;                 ///< Front anti-roll bar
    std::optional<double> rear_arb;                  ///< Rear anti-roll bar
    std::optional<double> heave_spring_n_mm;         ///< Heave spring rate (N/mm)
    std::optional<double> heave_packer_mm;           ///< Heave packer (millimeters)

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Suspension,
        front_left, front_right, rear_left, rear_right,
        front_arb, rear_arb, heave_spring_n_mm, heave_packer_mm)
};

/// Tire/Tyre configuration
struct Tires {
    std::optional<std::string> compound;             ///< Tire compound name
    std::optional<double> pressure_fl_kpa;           ///< Front-left pressure (kPa)
    std::optional<double> pressure_fr_kpa;           ///< Front-right pressure (kPa)
    std::optional<double> pressure_rl_kpa;           ///< Rear-left pressure (kPa)
    std::optional<double> pressure_rr_kpa;           ///< Rear-right pressure (kPa)
    std::optional<double> stagger_mm;                ///< Tire stagger (millimeters)

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Tires,
        compound, pressure_fl_kpa, pressure_fr_kpa,
        pressure_rl_kpa, pressure_rr_kpa, stagger_mm)
};

/// Drivetrain configuration
struct Drivetrain {
    std::optional<double> diff_preload_nm;           ///< Differential preload (N·m)
    std::optional<double> diff_power_ramp_pct;       ///< Power ramp (0-100%)
    std::optional<double> diff_coast_ramp_pct;       ///< Coast ramp (0-100%)
    std::optional<double> final_drive_ratio;         ///< Final drive ratio
    std::optional<int> lsd_clutch_plates;            ///< LSD clutch plate count

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Drivetrain,
        diff_preload_nm, diff_power_ramp_pct, diff_coast_ramp_pct,
        final_drive_ratio, lsd_clutch_plates)
};

/// Gearing configuration
struct Gearing {
    std::optional<std::vector<double>> gear_ratios;  ///< Gear ratios (indexed 0-N)
    std::optional<double> reverse_ratio;             ///< Reverse gear ratio

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Gearing,
        gear_ratios, reverse_ratio)
};

/// Brake configuration
struct Brakes {
    std::optional<std::string> pad_compound;         ///< Brake pad compound
    std::optional<std::string> disc_type;            ///< Brake disc type
    std::optional<double> brake_bias_pct;            ///< Brake bias (0-100%, front bias)
    std::optional<double> max_force_n;               ///< Maximum brake force (Newtons)

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Brakes,
        pad_compound, disc_type, brake_bias_pct, max_force_n)
};

/// Electronics configuration
struct Electronics {
    std::optional<int> tc_level;                     ///< Traction control level
    std::optional<int> tc2_level;                    ///< Secondary TC level
    std::optional<int> abs_level;                    ///< ABS level
    std::optional<int> engine_map;                   ///< Engine map selection
    std::optional<int> engine_brake_level;           ///< Engine braking level
    std::optional<double> pit_limiter_kph;           ///< Pit limiter speed (km/h)

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Electronics,
        tc_level, tc2_level, abs_level, engine_map,
        engine_brake_level, pit_limiter_kph)
};

/// Fuel configuration
struct Fuel {
    std::optional<double> start_fuel_l;              ///< Starting fuel (liters)
    std::optional<double> per_lap_consumption_l;     ///< Fuel per lap (liters)
    std::optional<int> stint_target_laps;            ///< Target stint length (laps)
    std::optional<int> mixture_setting;              ///< Fuel mixture setting

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Fuel,
        start_fuel_l, per_lap_consumption_l, stint_target_laps, mixture_setting)
};

/// Strategy configuration (extensible)
struct Strategy {
    std::optional<std::string> tire_change_policy;   ///< Tire change strategy
    std::optional<std::string> notes;                ///< Strategy notes
    std::map<std::string, json> custom;              ///< Custom strategy data

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Strategy,
        tire_change_policy, notes, custom)
};

/// Complete setup configuration
struct Setup {
    std::optional<Aerodynamics> aero;
    std::optional<Suspension> suspension;
    std::optional<Tires> tires;
    std::optional<Drivetrain> drivetrain;
    std::optional<Gearing> gearing;
    std::optional<Brakes> brakes;
    std::optional<Electronics> electronics;
    std::optional<Fuel> fuel;
    std::optional<Strategy> strategy;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(Setup,
        aero, suspension, tires, drivetrain, gearing,
        brakes, electronics, fuel, strategy)
};

// ============================================================================
// Main ORSF Structure
// ============================================================================

/// Complete ORSF setup format
struct ORSF {
    std::string schema;                              ///< Must be "orsf://v1"
    Metadata metadata;
    Car car;
    std::optional<Context> context;
    Setup setup;
    std::optional<std::map<std::string, json>> compat; ///< Sim-specific compatibility data

    /// Default constructor
    ORSF() : schema("orsf://v1") {}

    /// Parse ORSF from JSON string
    static ORSF from_json(const std::string& json_str);

    /// Parse ORSF from JSON object
    static ORSF from_json(const json& j);

    /// Serialize to JSON string
    std::string to_json_string(int indent = -1) const;

    /// Serialize to JSON object
    json to_json() const;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE_WITH_DEFAULT(ORSF,
        schema, metadata, car, context, setup, compat)
};

} // namespace orsf
