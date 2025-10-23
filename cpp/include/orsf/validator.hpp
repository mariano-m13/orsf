#pragma once

#include "core.hpp"
#include <string>
#include <vector>

namespace orsf {

// ============================================================================
// Validation Framework
// ============================================================================

/// Validation error severity levels
enum class ValidationSeverity {
    Error,      ///< Critical error, setup is invalid
    Warning,    ///< Non-critical issue, setup may work but is unusual
    Info        ///< Informational note
};

/// Validation error codes
enum class ValidationCode {
    Required,           ///< Required field is missing
    OutOfRange,         ///< Value is outside acceptable range
    InvalidFormat,      ///< Value format is incorrect
    Incompatible,       ///< Values are incompatible with each other
    Deprecated,         ///< Field is deprecated
    SchemaInvalid       ///< Schema version is invalid
};

/// Validation error details
struct ValidationError {
    ValidationSeverity severity;
    ValidationCode code;
    std::string field;          ///< Field path (e.g., "setup.suspension.front_left.camber_deg")
    std::string message;        ///< Human-readable error message
    std::optional<std::string> expected;  ///< Expected value/format
    std::optional<std::string> actual;    ///< Actual value found

    ValidationError(
        ValidationSeverity sev,
        ValidationCode c,
        const std::string& f,
        const std::string& msg,
        const std::optional<std::string>& exp = std::nullopt,
        const std::optional<std::string>& act = std::nullopt
    ) : severity(sev), code(c), field(f), message(msg), expected(exp), actual(act) {}

    /// Convert to human-readable string
    std::string to_string() const;
};

/// Validator for ORSF format
class Validator {
public:
    /// Validate complete ORSF structure
    static std::vector<ValidationError> validate(const ORSF& orsf);

    /// Validate schema version
    static void validate_schema(const ORSF& orsf, std::vector<ValidationError>& errors);

    /// Validate metadata section
    static void validate_metadata(const Metadata& metadata, std::vector<ValidationError>& errors);

    /// Validate car section
    static void validate_car(const Car& car, std::vector<ValidationError>& errors);

    /// Validate context section
    static void validate_context(const std::optional<Context>& context, std::vector<ValidationError>& errors);

    /// Validate setup section
    static void validate_setup(const Setup& setup, std::vector<ValidationError>& errors);

    /// Validate aerodynamics
    static void validate_aero(const std::optional<Aerodynamics>& aero, std::vector<ValidationError>& errors);

    /// Validate suspension
    static void validate_suspension(const std::optional<Suspension>& suspension, std::vector<ValidationError>& errors);

    /// Validate corner suspension
    static void validate_corner_suspension(
        const std::optional<CornerSuspension>& corner,
        const std::string& corner_name,
        std::vector<ValidationError>& errors
    );

    /// Validate tires
    static void validate_tires(const std::optional<Tires>& tires, std::vector<ValidationError>& errors);

    /// Validate drivetrain
    static void validate_drivetrain(const std::optional<Drivetrain>& drivetrain, std::vector<ValidationError>& errors);

    /// Validate gearing
    static void validate_gearing(const std::optional<Gearing>& gearing, std::vector<ValidationError>& errors);

    /// Validate brakes
    static void validate_brakes(const std::optional<Brakes>& brakes, std::vector<ValidationError>& errors);

    /// Validate electronics
    static void validate_electronics(const std::optional<Electronics>& electronics, std::vector<ValidationError>& errors);

    /// Validate fuel
    static void validate_fuel(const std::optional<Fuel>& fuel, std::vector<ValidationError>& errors);

    /// Cross-field validation (e.g., temperature consistency)
    static void validate_cross_field(const ORSF& orsf, std::vector<ValidationError>& errors);

private:
    // Helper functions for common validation patterns
    static void check_required(
        const std::string& field,
        bool is_present,
        std::vector<ValidationError>& errors
    );

    static void check_range(
        const std::string& field,
        double value,
        double min,
        double max,
        std::vector<ValidationError>& errors,
        ValidationSeverity severity = ValidationSeverity::Error
    );

    static void check_percentage(
        const std::string& field,
        double value,
        std::vector<ValidationError>& errors
    );

    static void check_positive(
        const std::string& field,
        double value,
        std::vector<ValidationError>& errors
    );

    static void check_non_negative(
        const std::string& field,
        double value,
        std::vector<ValidationError>& errors
    );

    static void check_iso8601(
        const std::string& field,
        const std::string& value,
        std::vector<ValidationError>& errors
    );
};

} // namespace orsf
