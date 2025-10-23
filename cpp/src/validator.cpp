#include "orsf/validator.hpp"
#include "orsf/utils.hpp"
#include <sstream>

namespace orsf {

// ============================================================================
// Validation Error Implementation
// ============================================================================

std::string ValidationError::to_string() const {
    std::ostringstream oss;

    // Severity prefix
    switch (severity) {
        case ValidationSeverity::Error:   oss << "[ERROR] "; break;
        case ValidationSeverity::Warning: oss << "[WARN]  "; break;
        case ValidationSeverity::Info:    oss << "[INFO]  "; break;
    }

    // Field and message
    oss << field << ": " << message;

    // Expected/actual values
    if (expected.has_value() && actual.has_value()) {
        oss << " (expected: " << expected.value()
            << ", actual: " << actual.value() << ")";
    } else if (expected.has_value()) {
        oss << " (expected: " << expected.value() << ")";
    }

    return oss.str();
}

// ============================================================================
// Validator Implementation
// ============================================================================

std::vector<ValidationError> Validator::validate(const ORSF& orsf) {
    std::vector<ValidationError> errors;

    validate_schema(orsf, errors);
    validate_metadata(orsf.metadata, errors);
    validate_car(orsf.car, errors);
    validate_context(orsf.context, errors);
    validate_setup(orsf.setup, errors);
    validate_cross_field(orsf, errors);

    return errors;
}

void Validator::validate_schema(const ORSF& orsf, std::vector<ValidationError>& errors) {
    if (orsf.schema != "orsf://v1") {
        errors.push_back(ValidationError(
            ValidationSeverity::Error,
            ValidationCode::SchemaInvalid,
            "schema",
            "Invalid schema version",
            "orsf://v1",
            orsf.schema
        ));
    }
}

void Validator::validate_metadata(const Metadata& metadata, std::vector<ValidationError>& errors) {
    check_required("metadata.id", !metadata.id.empty(), errors);
    check_required("metadata.name", !metadata.name.empty(), errors);
    check_required("metadata.created_at", !metadata.created_at.empty(), errors);

    // Validate ISO8601 timestamps
    if (!metadata.created_at.empty()) {
        check_iso8601("metadata.created_at", metadata.created_at, errors);
    }

    if (metadata.updated_at.has_value() && !metadata.updated_at.value().empty()) {
        check_iso8601("metadata.updated_at", metadata.updated_at.value(), errors);
    }

    // Check that updated_at >= created_at
    if (metadata.updated_at.has_value()) {
        // Simplified check - production should compare timestamps properly
        if (metadata.updated_at.value() < metadata.created_at) {
            errors.push_back(ValidationError(
                ValidationSeverity::Warning,
                ValidationCode::Incompatible,
                "metadata.updated_at",
                "Updated timestamp is before created timestamp"
            ));
        }
    }
}

void Validator::validate_car(const Car& car, std::vector<ValidationError>& errors) {
    check_required("car.make", !car.make.empty(), errors);
    check_required("car.model", !car.model.empty(), errors);

    // Validate class if present
    if (car.car_class.has_value()) {
        const std::vector<std::string> valid_classes = {
            "GT3", "GTE", "LMP2", "LMDh", "GT4", "TCR",
            "F1", "F2", "F3", "F4", "Formula"
        };

        bool valid = false;
        for (const auto& cls : valid_classes) {
            if (car.car_class.value() == cls) {
                valid = true;
                break;
            }
        }

        if (!valid) {
            errors.push_back(ValidationError(
                ValidationSeverity::Warning,
                ValidationCode::InvalidFormat,
                "car.class",
                "Unknown car class: " + car.car_class.value()
            ));
        }
    }
}

void Validator::validate_context(const std::optional<Context>& context, std::vector<ValidationError>& errors) {
    if (!context.has_value()) return;

    const Context& ctx = context.value();

    // Temperature ranges
    if (ctx.ambient_temp_c.has_value()) {
        check_range("context.ambient_temp_c", ctx.ambient_temp_c.value(),
            -50.0, 70.0, errors, ValidationSeverity::Warning);
    }

    if (ctx.track_temp_c.has_value()) {
        check_range("context.track_temp_c", ctx.track_temp_c.value(),
            -20.0, 80.0, errors, ValidationSeverity::Warning);
    }

    // Wetness should be 0-1
    if (ctx.wetness.has_value()) {
        check_range("context.wetness", ctx.wetness.value(), 0.0, 1.0, errors);
    }

    // Rubber level validation
    if (ctx.rubber.has_value()) {
        const std::vector<std::string> valid_levels = {
            "green", "low", "medium", "high", "saturated"
        };
        bool valid = false;
        for (const auto& level : valid_levels) {
            if (ctx.rubber.value() == level) {
                valid = true;
                break;
            }
        }

        if (!valid) {
            errors.push_back(ValidationError(
                ValidationSeverity::Warning,
                ValidationCode::InvalidFormat,
                "context.rubber",
                "Unknown rubber level: " + ctx.rubber.value()
            ));
        }
    }
}

void Validator::validate_setup(const Setup& setup, std::vector<ValidationError>& errors) {
    validate_aero(setup.aero, errors);
    validate_suspension(setup.suspension, errors);
    validate_tires(setup.tires, errors);
    validate_drivetrain(setup.drivetrain, errors);
    validate_gearing(setup.gearing, errors);
    validate_brakes(setup.brakes, errors);
    validate_electronics(setup.electronics, errors);
    validate_fuel(setup.fuel, errors);
}

void Validator::validate_aero(const std::optional<Aerodynamics>& aero, std::vector<ValidationError>& errors) {
    if (!aero.has_value()) return;

    const Aerodynamics& a = aero.value();

    // Ride heights should be positive
    if (a.front_ride_height_mm.has_value()) {
        check_positive("setup.aero.front_ride_height_mm", a.front_ride_height_mm.value(), errors);
    }

    if (a.rear_ride_height_mm.has_value()) {
        check_positive("setup.aero.rear_ride_height_mm", a.rear_ride_height_mm.value(), errors);
    }

    // Brake ducts should be 0-100%
    if (a.brake_duct_front_pct.has_value()) {
        check_percentage("setup.aero.brake_duct_front_pct", a.brake_duct_front_pct.value(), errors);
    }

    if (a.brake_duct_rear_pct.has_value()) {
        check_percentage("setup.aero.brake_duct_rear_pct", a.brake_duct_rear_pct.value(), errors);
    }

    // Radiator opening should be 0-100%
    if (a.radiator_opening_pct.has_value()) {
        check_percentage("setup.aero.radiator_opening_pct", a.radiator_opening_pct.value(), errors);
    }

    // Downforce should be positive
    if (a.front_downforce_n.has_value()) {
        check_non_negative("setup.aero.front_downforce_n", a.front_downforce_n.value(), errors);
    }

    if (a.rear_downforce_n.has_value()) {
        check_non_negative("setup.aero.rear_downforce_n", a.rear_downforce_n.value(), errors);
    }
}

void Validator::validate_suspension(const std::optional<Suspension>& suspension, std::vector<ValidationError>& errors) {
    if (!suspension.has_value()) return;

    const Suspension& s = suspension.value();

    validate_corner_suspension(s.front_left, "setup.suspension.front_left", errors);
    validate_corner_suspension(s.front_right, "setup.suspension.front_right", errors);
    validate_corner_suspension(s.rear_left, "setup.suspension.rear_left", errors);
    validate_corner_suspension(s.rear_right, "setup.suspension.rear_right", errors);

    // Heave spring should be positive if present
    if (s.heave_spring_n_mm.has_value()) {
        check_positive("setup.suspension.heave_spring_n_mm", s.heave_spring_n_mm.value(), errors);
    }
}

void Validator::validate_corner_suspension(
    const std::optional<CornerSuspension>& corner,
    const std::string& corner_name,
    std::vector<ValidationError>& errors
) {
    if (!corner.has_value()) return;

    const CornerSuspension& c = corner.value();

    // Camber typically -10° to +5°
    if (c.camber_deg.has_value()) {
        check_range(corner_name + ".camber_deg", c.camber_deg.value(),
            -10.0, 5.0, errors, ValidationSeverity::Warning);
    }

    // Spring rate should be positive
    if (c.spring_rate_n_mm.has_value()) {
        check_positive(corner_name + ".spring_rate_n_mm", c.spring_rate_n_mm.value(), errors);
    }

    // Ride height should be positive
    if (c.ride_height_mm.has_value()) {
        check_positive(corner_name + ".ride_height_mm", c.ride_height_mm.value(), errors);
    }

    // Bumpstop gap should be non-negative
    if (c.bumpstop_gap_mm.has_value()) {
        check_non_negative(corner_name + ".bumpstop_gap_mm", c.bumpstop_gap_mm.value(), errors);
    }

    // Bumpstop rate should be positive
    if (c.bumpstop_rate_n_mm.has_value()) {
        check_positive(corner_name + ".bumpstop_rate_n_mm", c.bumpstop_rate_n_mm.value(), errors);
    }

    // Dampers should be non-negative
    if (c.damper_bump_slow_n_s_m.has_value()) {
        check_non_negative(corner_name + ".damper_bump_slow_n_s_m", c.damper_bump_slow_n_s_m.value(), errors);
    }

    if (c.damper_bump_fast_n_s_m.has_value()) {
        check_non_negative(corner_name + ".damper_bump_fast_n_s_m", c.damper_bump_fast_n_s_m.value(), errors);
    }

    if (c.damper_rebound_slow_n_s_m.has_value()) {
        check_non_negative(corner_name + ".damper_rebound_slow_n_s_m", c.damper_rebound_slow_n_s_m.value(), errors);
    }

    if (c.damper_rebound_fast_n_s_m.has_value()) {
        check_non_negative(corner_name + ".damper_rebound_fast_n_s_m", c.damper_rebound_fast_n_s_m.value(), errors);
    }
}

void Validator::validate_tires(const std::optional<Tires>& tires, std::vector<ValidationError>& errors) {
    if (!tires.has_value()) return;

    const Tires& t = tires.value();

    // Tire pressures: typically 50-400 kPa (7-60 PSI)
    if (t.pressure_fl_kpa.has_value()) {
        check_range("setup.tires.pressure_fl_kpa", t.pressure_fl_kpa.value(),
            50.0, 400.0, errors, ValidationSeverity::Warning);
    }

    if (t.pressure_fr_kpa.has_value()) {
        check_range("setup.tires.pressure_fr_kpa", t.pressure_fr_kpa.value(),
            50.0, 400.0, errors, ValidationSeverity::Warning);
    }

    if (t.pressure_rl_kpa.has_value()) {
        check_range("setup.tires.pressure_rl_kpa", t.pressure_rl_kpa.value(),
            50.0, 400.0, errors, ValidationSeverity::Warning);
    }

    if (t.pressure_rr_kpa.has_value()) {
        check_range("setup.tires.pressure_rr_kpa", t.pressure_rr_kpa.value(),
            50.0, 400.0, errors, ValidationSeverity::Warning);
    }
}

void Validator::validate_drivetrain(const std::optional<Drivetrain>& drivetrain, std::vector<ValidationError>& errors) {
    if (!drivetrain.has_value()) return;

    const Drivetrain& d = drivetrain.value();

    // Preload should be non-negative
    if (d.diff_preload_nm.has_value()) {
        check_non_negative("setup.drivetrain.diff_preload_nm", d.diff_preload_nm.value(), errors);
    }

    // Ramp percentages should be 0-100%
    if (d.diff_power_ramp_pct.has_value()) {
        check_percentage("setup.drivetrain.diff_power_ramp_pct", d.diff_power_ramp_pct.value(), errors);
    }

    if (d.diff_coast_ramp_pct.has_value()) {
        check_percentage("setup.drivetrain.diff_coast_ramp_pct", d.diff_coast_ramp_pct.value(), errors);
    }

    // Final drive ratio should be positive
    if (d.final_drive_ratio.has_value()) {
        check_positive("setup.drivetrain.final_drive_ratio", d.final_drive_ratio.value(), errors);
    }

    // Clutch plates should be positive
    if (d.lsd_clutch_plates.has_value()) {
        if (d.lsd_clutch_plates.value() <= 0) {
            errors.push_back(ValidationError(
                ValidationSeverity::Error,
                ValidationCode::OutOfRange,
                "setup.drivetrain.lsd_clutch_plates",
                "LSD clutch plates must be positive"
            ));
        }
    }
}

void Validator::validate_gearing(const std::optional<Gearing>& gearing, std::vector<ValidationError>& errors) {
    if (!gearing.has_value()) return;

    const Gearing& g = gearing.value();

    if (g.gear_ratios.has_value()) {
        const auto& ratios = g.gear_ratios.value();

        if (ratios.empty()) {
            errors.push_back(ValidationError(
                ValidationSeverity::Warning,
                ValidationCode::InvalidFormat,
                "setup.gearing.gear_ratios",
                "Gear ratios array is empty"
            ));
        }

        // All gear ratios should be positive
        for (size_t i = 0; i < ratios.size(); ++i) {
            if (ratios[i] <= 0.0) {
                errors.push_back(ValidationError(
                    ValidationSeverity::Error,
                    ValidationCode::OutOfRange,
                    "setup.gearing.gear_ratios[" + std::to_string(i) + "]",
                    "Gear ratio must be positive"
                ));
            }
        }
    }

    // Reverse ratio should be positive
    if (g.reverse_ratio.has_value()) {
        check_positive("setup.gearing.reverse_ratio", g.reverse_ratio.value(), errors);
    }
}

void Validator::validate_brakes(const std::optional<Brakes>& brakes, std::vector<ValidationError>& errors) {
    if (!brakes.has_value()) return;

    const Brakes& b = brakes.value();

    // Brake bias should be 0-100%
    if (b.brake_bias_pct.has_value()) {
        check_percentage("setup.brakes.brake_bias_pct", b.brake_bias_pct.value(), errors);
    }

    // Max force should be positive
    if (b.max_force_n.has_value()) {
        check_positive("setup.brakes.max_force_n", b.max_force_n.value(), errors);
    }
}

void Validator::validate_electronics(const std::optional<Electronics>& electronics, std::vector<ValidationError>& errors) {
    if (!electronics.has_value()) return;

    const Electronics& e = electronics.value();

    // Pit limiter should be positive
    if (e.pit_limiter_kph.has_value()) {
        check_positive("setup.electronics.pit_limiter_kph", e.pit_limiter_kph.value(), errors);
    }
}

void Validator::validate_fuel(const std::optional<Fuel>& fuel, std::vector<ValidationError>& errors) {
    if (!fuel.has_value()) return;

    const Fuel& f = fuel.value();

    // Start fuel should be non-negative
    if (f.start_fuel_l.has_value()) {
        check_non_negative("setup.fuel.start_fuel_l", f.start_fuel_l.value(), errors);
    }

    // Per-lap consumption should be positive
    if (f.per_lap_consumption_l.has_value()) {
        check_positive("setup.fuel.per_lap_consumption_l", f.per_lap_consumption_l.value(), errors);
    }

    // Stint target laps should be positive
    if (f.stint_target_laps.has_value()) {
        if (f.stint_target_laps.value() <= 0) {
            errors.push_back(ValidationError(
                ValidationSeverity::Error,
                ValidationCode::OutOfRange,
                "setup.fuel.stint_target_laps",
                "Stint target laps must be positive"
            ));
        }
    }
}

void Validator::validate_cross_field(const ORSF& orsf, std::vector<ValidationError>& errors) {
    // Temperature consistency check
    if (orsf.context.has_value()) {
        const Context& ctx = orsf.context.value();

        if (ctx.ambient_temp_c.has_value() && ctx.track_temp_c.has_value()) {
            double ambient = ctx.ambient_temp_c.value();
            double track = ctx.track_temp_c.value();

            // Track temp is usually higher than ambient (within reasonable limits)
            if (track < ambient - 5.0) {
                errors.push_back(ValidationError(
                    ValidationSeverity::Warning,
                    ValidationCode::Incompatible,
                    "context.track_temp_c",
                    "Track temperature is significantly lower than ambient temperature"
                ));
            }

            if (track > ambient + 40.0) {
                errors.push_back(ValidationError(
                    ValidationSeverity::Warning,
                    ValidationCode::Incompatible,
                    "context.track_temp_c",
                    "Track temperature is unusually high compared to ambient"
                ));
            }
        }
    }
}

// ============================================================================
// Helper Functions
// ============================================================================

void Validator::check_required(
    const std::string& field,
    bool is_present,
    std::vector<ValidationError>& errors
) {
    if (!is_present) {
        errors.push_back(ValidationError(
            ValidationSeverity::Error,
            ValidationCode::Required,
            field,
            "Required field is missing"
        ));
    }
}

void Validator::check_range(
    const std::string& field,
    double value,
    double min,
    double max,
    std::vector<ValidationError>& errors,
    ValidationSeverity severity
) {
    if (value < min || value > max) {
        std::ostringstream expected;
        expected << min << " to " << max;

        errors.push_back(ValidationError(
            severity,
            ValidationCode::OutOfRange,
            field,
            "Value out of range",
            expected.str(),
            std::to_string(value)
        ));
    }
}

void Validator::check_percentage(
    const std::string& field,
    double value,
    std::vector<ValidationError>& errors
) {
    check_range(field, value, 0.0, 100.0, errors);
}

void Validator::check_positive(
    const std::string& field,
    double value,
    std::vector<ValidationError>& errors
) {
    if (value <= 0.0) {
        errors.push_back(ValidationError(
            ValidationSeverity::Error,
            ValidationCode::OutOfRange,
            field,
            "Value must be positive",
            "> 0",
            std::to_string(value)
        ));
    }
}

void Validator::check_non_negative(
    const std::string& field,
    double value,
    std::vector<ValidationError>& errors
) {
    if (value < 0.0) {
        errors.push_back(ValidationError(
            ValidationSeverity::Error,
            ValidationCode::OutOfRange,
            field,
            "Value must be non-negative",
            ">= 0",
            std::to_string(value)
        ));
    }
}

void Validator::check_iso8601(
    const std::string& field,
    const std::string& value,
    std::vector<ValidationError>& errors
) {
    if (!DateTimeUtils::is_valid_iso8601(value)) {
        errors.push_back(ValidationError(
            ValidationSeverity::Warning,
            ValidationCode::InvalidFormat,
            field,
            "Invalid ISO8601 timestamp format",
            "YYYY-MM-DDTHH:MM:SS(.sss)?(Z|[+-]HH:MM)?",
            value
        ));
    }
}

} // namespace orsf
