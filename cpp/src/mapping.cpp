#include "orsf/mapping.hpp"
#include "orsf/utils.hpp"
#include <stdexcept>

namespace orsf {

// ============================================================================
// Mapping Engine Implementation
// ============================================================================

FlatSetup MappingEngine::flatten_orsf(const ORSF& orsf) {
    FlatSetup flat;

    flatten_aero(orsf.setup.aero, flat);
    flatten_suspension(orsf.setup.suspension, flat);
    flatten_tires(orsf.setup.tires, flat);
    flatten_drivetrain(orsf.setup.drivetrain, flat);
    flatten_gearing(orsf.setup.gearing, flat);
    flatten_brakes(orsf.setup.brakes, flat);
    flatten_electronics(orsf.setup.electronics, flat);
    flatten_fuel(orsf.setup.fuel, flat);

    return flat;
}

ORSF MappingEngine::inflate_orsf(const FlatSetup& flat, const ORSF& template_orsf) {
    ORSF result = template_orsf;

    // This is a simplified implementation
    // Production code would need to handle all fields systematically
    for (const auto& [key, value] : flat) {
        set_value(result, key, value);
    }

    return result;
}

FlatSetup MappingEngine::map_to_native(
    const ORSF& orsf,
    const std::vector<FieldMapping>& mappings
) {
    FlatSetup native;

    for (const auto& mapping : mappings) {
        auto value = get_value(orsf, mapping.orsf_path);

        if (value.has_value()) {
            double mapped_value = value.value();

            // Apply transformation if present
            if (mapping.to_native.has_value()) {
                mapped_value = mapping.to_native.value()(mapped_value);
            }

            native[mapping.native_key] = mapped_value;
        } else if (mapping.required) {
            throw std::runtime_error(
                "Required field missing: " + mapping.orsf_path
            );
        }
    }

    return native;
}

ORSF MappingEngine::map_to_orsf(
    const FlatSetup& native,
    const std::vector<FieldMapping>& mappings,
    const ORSF& template_orsf
) {
    ORSF result = template_orsf;

    for (const auto& mapping : mappings) {
        auto it = native.find(mapping.native_key);

        if (it != native.end()) {
            double value = it->second;

            // Apply reverse transformation if present
            if (mapping.to_orsf.has_value()) {
                value = mapping.to_orsf.value()(value);
            }

            set_value(result, mapping.orsf_path, value);
        } else if (mapping.required) {
            throw std::runtime_error(
                "Required native field missing: " + mapping.native_key
            );
        }
    }

    return result;
}

std::optional<double> MappingEngine::get_value(const ORSF& orsf, const std::string& path) {
    auto parts = split_path(path);

    if (parts.empty()) return std::nullopt;

    // Navigate the path
    if (parts[0] != "setup") return std::nullopt;
    if (parts.size() < 2) return std::nullopt;

    const std::string& section = parts[1];

    // Aerodynamics
    if (section == "aero" && orsf.setup.aero.has_value()) {
        const auto& aero = orsf.setup.aero.value();
        if (parts.size() < 3) return std::nullopt;

        if (parts[2] == "front_wing") return aero.front_wing;
        if (parts[2] == "rear_wing") return aero.rear_wing;
        if (parts[2] == "front_downforce_n") return aero.front_downforce_n;
        if (parts[2] == "rear_downforce_n") return aero.rear_downforce_n;
        if (parts[2] == "front_ride_height_mm") return aero.front_ride_height_mm;
        if (parts[2] == "rear_ride_height_mm") return aero.rear_ride_height_mm;
        if (parts[2] == "rake_mm") return aero.rake_mm;
        if (parts[2] == "brake_duct_front_pct") return aero.brake_duct_front_pct;
        if (parts[2] == "brake_duct_rear_pct") return aero.brake_duct_rear_pct;
        if (parts[2] == "radiator_opening_pct") return aero.radiator_opening_pct;
    }

    // Suspension
    if (section == "suspension" && orsf.setup.suspension.has_value()) {
        const auto& susp = orsf.setup.suspension.value();
        if (parts.size() < 3) return std::nullopt;

        // Corner-specific
        const CornerSuspension* corner = nullptr;
        if (parts[2] == "front_left") corner = susp.front_left.has_value() ? &susp.front_left.value() : nullptr;
        else if (parts[2] == "front_right") corner = susp.front_right.has_value() ? &susp.front_right.value() : nullptr;
        else if (parts[2] == "rear_left") corner = susp.rear_left.has_value() ? &susp.rear_left.value() : nullptr;
        else if (parts[2] == "rear_right") corner = susp.rear_right.has_value() ? &susp.rear_right.value() : nullptr;

        if (corner && parts.size() >= 4) {
            if (parts[3] == "camber_deg") return corner->camber_deg;
            if (parts[3] == "toe_deg") return corner->toe_deg;
            if (parts[3] == "caster_deg") return corner->caster_deg;
            if (parts[3] == "spring_rate_n_mm") return corner->spring_rate_n_mm;
            if (parts[3] == "ride_height_mm") return corner->ride_height_mm;
            if (parts[3] == "bumpstop_gap_mm") return corner->bumpstop_gap_mm;
            if (parts[3] == "bumpstop_rate_n_mm") return corner->bumpstop_rate_n_mm;
            if (parts[3] == "damper_bump_slow_n_s_m") return corner->damper_bump_slow_n_s_m;
            if (parts[3] == "damper_bump_fast_n_s_m") return corner->damper_bump_fast_n_s_m;
            if (parts[3] == "damper_rebound_slow_n_s_m") return corner->damper_rebound_slow_n_s_m;
            if (parts[3] == "damper_rebound_fast_n_s_m") return corner->damper_rebound_fast_n_s_m;
        }

        // ARBs
        if (parts[2] == "front_arb") return susp.front_arb;
        if (parts[2] == "rear_arb") return susp.rear_arb;
        if (parts[2] == "heave_spring_n_mm") return susp.heave_spring_n_mm;
    }

    // Tires
    if (section == "tires" && orsf.setup.tires.has_value()) {
        const auto& tires = orsf.setup.tires.value();
        if (parts.size() < 3) return std::nullopt;

        if (parts[2] == "pressure_fl_kpa") return tires.pressure_fl_kpa;
        if (parts[2] == "pressure_fr_kpa") return tires.pressure_fr_kpa;
        if (parts[2] == "pressure_rl_kpa") return tires.pressure_rl_kpa;
        if (parts[2] == "pressure_rr_kpa") return tires.pressure_rr_kpa;
        if (parts[2] == "stagger_mm") return tires.stagger_mm;
    }

    // Drivetrain
    if (section == "drivetrain" && orsf.setup.drivetrain.has_value()) {
        const auto& dt = orsf.setup.drivetrain.value();
        if (parts.size() < 3) return std::nullopt;

        if (parts[2] == "diff_preload_nm") return dt.diff_preload_nm;
        if (parts[2] == "diff_power_ramp_pct") return dt.diff_power_ramp_pct;
        if (parts[2] == "diff_coast_ramp_pct") return dt.diff_coast_ramp_pct;
        if (parts[2] == "final_drive_ratio") return dt.final_drive_ratio;
        if (parts[2] == "lsd_clutch_plates" && dt.lsd_clutch_plates.has_value())
            return static_cast<double>(dt.lsd_clutch_plates.value());
    }

    // Brakes
    if (section == "brakes" && orsf.setup.brakes.has_value()) {
        const auto& brakes = orsf.setup.brakes.value();
        if (parts.size() < 3) return std::nullopt;

        if (parts[2] == "brake_bias_pct") return brakes.brake_bias_pct;
        if (parts[2] == "max_force_n") return brakes.max_force_n;
    }

    // Electronics
    if (section == "electronics" && orsf.setup.electronics.has_value()) {
        const auto& elec = orsf.setup.electronics.value();
        if (parts.size() < 3) return std::nullopt;

        if (parts[2] == "tc_level" && elec.tc_level.has_value())
            return static_cast<double>(elec.tc_level.value());
        if (parts[2] == "abs_level" && elec.abs_level.has_value())
            return static_cast<double>(elec.abs_level.value());
        if (parts[2] == "engine_map" && elec.engine_map.has_value())
            return static_cast<double>(elec.engine_map.value());
        if (parts[2] == "pit_limiter_kph") return elec.pit_limiter_kph;
    }

    // Fuel
    if (section == "fuel" && orsf.setup.fuel.has_value()) {
        const auto& fuel = orsf.setup.fuel.value();
        if (parts.size() < 3) return std::nullopt;

        if (parts[2] == "start_fuel_l") return fuel.start_fuel_l;
        if (parts[2] == "per_lap_consumption_l") return fuel.per_lap_consumption_l;
        if (parts[2] == "stint_target_laps" && fuel.stint_target_laps.has_value())
            return static_cast<double>(fuel.stint_target_laps.value());
    }

    return std::nullopt;
}

void MappingEngine::set_value(ORSF& orsf, const std::string& path, double value) {
    auto parts = split_path(path);

    if (parts.empty() || parts[0] != "setup" || parts.size() < 3) {
        return; // Invalid path
    }

    const std::string& section = parts[1];
    const std::string& field = parts[2];

    // Aerodynamics
    if (section == "aero") {
        if (!orsf.setup.aero.has_value()) {
            orsf.setup.aero = Aerodynamics{};
        }
        auto& aero = orsf.setup.aero.value();

        if (field == "front_wing") aero.front_wing = value;
        else if (field == "rear_wing") aero.rear_wing = value;
        else if (field == "front_downforce_n") aero.front_downforce_n = value;
        else if (field == "rear_downforce_n") aero.rear_downforce_n = value;
        else if (field == "front_ride_height_mm") aero.front_ride_height_mm = value;
        else if (field == "rear_ride_height_mm") aero.rear_ride_height_mm = value;
        else if (field == "rake_mm") aero.rake_mm = value;
        else if (field == "brake_duct_front_pct") aero.brake_duct_front_pct = value;
        else if (field == "brake_duct_rear_pct") aero.brake_duct_rear_pct = value;
        else if (field == "radiator_opening_pct") aero.radiator_opening_pct = value;
    }

    // Tires
    else if (section == "tires") {
        if (!orsf.setup.tires.has_value()) {
            orsf.setup.tires = Tires{};
        }
        auto& tires = orsf.setup.tires.value();

        if (field == "pressure_fl_kpa") tires.pressure_fl_kpa = value;
        else if (field == "pressure_fr_kpa") tires.pressure_fr_kpa = value;
        else if (field == "pressure_rl_kpa") tires.pressure_rl_kpa = value;
        else if (field == "pressure_rr_kpa") tires.pressure_rr_kpa = value;
        else if (field == "stagger_mm") tires.stagger_mm = value;
    }

    // Brakes
    else if (section == "brakes") {
        if (!orsf.setup.brakes.has_value()) {
            orsf.setup.brakes = Brakes{};
        }
        auto& brakes = orsf.setup.brakes.value();

        if (field == "brake_bias_pct") brakes.brake_bias_pct = value;
        else if (field == "max_force_n") brakes.max_force_n = value;
    }

    // Add more sections as needed...
}

std::vector<std::string> MappingEngine::split_path(const std::string& path) {
    return StringUtils::split(path, '.');
}

// ============================================================================
// Flatten Helpers
// ============================================================================

void MappingEngine::flatten_aero(const std::optional<Aerodynamics>& aero, FlatSetup& flat) {
    if (!aero.has_value()) return;

    add_optional(flat, "setup.aero.front_wing", aero->front_wing);
    add_optional(flat, "setup.aero.rear_wing", aero->rear_wing);
    add_optional(flat, "setup.aero.front_downforce_n", aero->front_downforce_n);
    add_optional(flat, "setup.aero.rear_downforce_n", aero->rear_downforce_n);
    add_optional(flat, "setup.aero.front_ride_height_mm", aero->front_ride_height_mm);
    add_optional(flat, "setup.aero.rear_ride_height_mm", aero->rear_ride_height_mm);
    add_optional(flat, "setup.aero.rake_mm", aero->rake_mm);
    add_optional(flat, "setup.aero.brake_duct_front_pct", aero->brake_duct_front_pct);
    add_optional(flat, "setup.aero.brake_duct_rear_pct", aero->brake_duct_rear_pct);
    add_optional(flat, "setup.aero.radiator_opening_pct", aero->radiator_opening_pct);
}

void MappingEngine::flatten_suspension(const std::optional<Suspension>& susp, FlatSetup& flat) {
    if (!susp.has_value()) return;

    flatten_corner(susp->front_left, "setup.suspension.front_left", flat);
    flatten_corner(susp->front_right, "setup.suspension.front_right", flat);
    flatten_corner(susp->rear_left, "setup.suspension.rear_left", flat);
    flatten_corner(susp->rear_right, "setup.suspension.rear_right", flat);

    add_optional(flat, "setup.suspension.front_arb", susp->front_arb);
    add_optional(flat, "setup.suspension.rear_arb", susp->rear_arb);
    add_optional(flat, "setup.suspension.heave_spring_n_mm", susp->heave_spring_n_mm);
    add_optional(flat, "setup.suspension.heave_packer_mm", susp->heave_packer_mm);
}

void MappingEngine::flatten_corner(const std::optional<CornerSuspension>& corner, const std::string& prefix, FlatSetup& flat) {
    if (!corner.has_value()) return;

    add_optional(flat, prefix + ".camber_deg", corner->camber_deg);
    add_optional(flat, prefix + ".toe_deg", corner->toe_deg);
    add_optional(flat, prefix + ".caster_deg", corner->caster_deg);
    add_optional(flat, prefix + ".spring_rate_n_mm", corner->spring_rate_n_mm);
    add_optional(flat, prefix + ".ride_height_mm", corner->ride_height_mm);
    add_optional(flat, prefix + ".bumpstop_gap_mm", corner->bumpstop_gap_mm);
    add_optional(flat, prefix + ".bumpstop_rate_n_mm", corner->bumpstop_rate_n_mm);
    add_optional(flat, prefix + ".packer_mm", corner->packer_mm);
    add_optional(flat, prefix + ".damper_bump_slow_n_s_m", corner->damper_bump_slow_n_s_m);
    add_optional(flat, prefix + ".damper_bump_fast_n_s_m", corner->damper_bump_fast_n_s_m);
    add_optional(flat, prefix + ".damper_rebound_slow_n_s_m", corner->damper_rebound_slow_n_s_m);
    add_optional(flat, prefix + ".damper_rebound_fast_n_s_m", corner->damper_rebound_fast_n_s_m);
}

void MappingEngine::flatten_tires(const std::optional<Tires>& tires, FlatSetup& flat) {
    if (!tires.has_value()) return;

    add_optional(flat, "setup.tires.pressure_fl_kpa", tires->pressure_fl_kpa);
    add_optional(flat, "setup.tires.pressure_fr_kpa", tires->pressure_fr_kpa);
    add_optional(flat, "setup.tires.pressure_rl_kpa", tires->pressure_rl_kpa);
    add_optional(flat, "setup.tires.pressure_rr_kpa", tires->pressure_rr_kpa);
    add_optional(flat, "setup.tires.stagger_mm", tires->stagger_mm);
}

void MappingEngine::flatten_drivetrain(const std::optional<Drivetrain>& drivetrain, FlatSetup& flat) {
    if (!drivetrain.has_value()) return;

    add_optional(flat, "setup.drivetrain.diff_preload_nm", drivetrain->diff_preload_nm);
    add_optional(flat, "setup.drivetrain.diff_power_ramp_pct", drivetrain->diff_power_ramp_pct);
    add_optional(flat, "setup.drivetrain.diff_coast_ramp_pct", drivetrain->diff_coast_ramp_pct);
    add_optional(flat, "setup.drivetrain.final_drive_ratio", drivetrain->final_drive_ratio);
    add_optional(flat, "setup.drivetrain.lsd_clutch_plates", drivetrain->lsd_clutch_plates);
}

void MappingEngine::flatten_gearing(const std::optional<Gearing>& gearing, FlatSetup& flat) {
    if (!gearing.has_value()) return;

    if (gearing->gear_ratios.has_value()) {
        const auto& ratios = gearing->gear_ratios.value();
        for (size_t i = 0; i < ratios.size(); ++i) {
            flat["setup.gearing.gear_" + std::to_string(i)] = ratios[i];
        }
    }

    add_optional(flat, "setup.gearing.reverse_ratio", gearing->reverse_ratio);
}

void MappingEngine::flatten_brakes(const std::optional<Brakes>& brakes, FlatSetup& flat) {
    if (!brakes.has_value()) return;

    add_optional(flat, "setup.brakes.brake_bias_pct", brakes->brake_bias_pct);
    add_optional(flat, "setup.brakes.max_force_n", brakes->max_force_n);
}

void MappingEngine::flatten_electronics(const std::optional<Electronics>& electronics, FlatSetup& flat) {
    if (!electronics.has_value()) return;

    add_optional(flat, "setup.electronics.tc_level", electronics->tc_level);
    add_optional(flat, "setup.electronics.tc2_level", electronics->tc2_level);
    add_optional(flat, "setup.electronics.abs_level", electronics->abs_level);
    add_optional(flat, "setup.electronics.engine_map", electronics->engine_map);
    add_optional(flat, "setup.electronics.engine_brake_level", electronics->engine_brake_level);
    add_optional(flat, "setup.electronics.pit_limiter_kph", electronics->pit_limiter_kph);
}

void MappingEngine::flatten_fuel(const std::optional<Fuel>& fuel, FlatSetup& flat) {
    if (!fuel.has_value()) return;

    add_optional(flat, "setup.fuel.start_fuel_l", fuel->start_fuel_l);
    add_optional(flat, "setup.fuel.per_lap_consumption_l", fuel->per_lap_consumption_l);
    add_optional(flat, "setup.fuel.stint_target_laps", fuel->stint_target_laps);
    add_optional(flat, "setup.fuel.mixture_setting", fuel->mixture_setting);
}

} // namespace orsf
