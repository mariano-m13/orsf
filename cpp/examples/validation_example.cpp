/**
 * ORSF Validation Example
 *
 * Demonstrates:
 * - Validating ORSF setups
 * - Handling validation errors
 * - Understanding severity levels
 */

#include <iostream>
#include "orsf/orsf.hpp"

using namespace orsf;

void print_validation_results(const std::vector<ValidationError>& errors) {
    if (errors.empty()) {
        std::cout << "✓ Setup is valid!" << std::endl;
        return;
    }

    std::cout << "Found " << errors.size() << " validation issue(s):" << std::endl;
    for (const auto& error : errors) {
        std::cout << "  " << error.to_string() << std::endl;
    }
}

int main() {
    std::cout << "=== ORSF Validation Example ===" << std::endl << std::endl;

    // Example 1: Valid setup
    std::cout << "--- Example 1: Valid Setup ---" << std::endl;
    {
        ORSF setup;
        setup.metadata.id = "valid-setup";
        setup.metadata.name = "Valid GT3 Setup";
        setup.metadata.created_at = "2024-01-15T10:00:00Z";
        setup.car.make = "Porsche";
        setup.car.model = "911 GT3 R";

        setup.setup.aero = Aerodynamics{};
        setup.setup.aero->front_wing = 3;
        setup.setup.aero->rear_wing = 5;
        setup.setup.aero->brake_duct_front_pct = 50.0;

        auto errors = Validator::validate(setup);
        print_validation_results(errors);
    }

    std::cout << std::endl;

    // Example 2: Missing required fields
    std::cout << "--- Example 2: Missing Required Fields ---" << std::endl;
    {
        ORSF setup;
        setup.metadata.id = "";  // Missing required ID
        setup.metadata.name = "Test Setup";
        setup.metadata.created_at = "2024-01-15T10:00:00Z";
        setup.car.make = "";  // Missing required make
        setup.car.model = "Test Car";

        auto errors = Validator::validate(setup);
        print_validation_results(errors);
    }

    std::cout << std::endl;

    // Example 3: Out of range values
    std::cout << "--- Example 3: Out of Range Values ---" << std::endl;
    {
        ORSF setup;
        setup.metadata.id = "range-test";
        setup.metadata.name = "Range Test";
        setup.metadata.created_at = "2024-01-15T10:00:00Z";
        setup.car.make = "Test";
        setup.car.model = "Car";

        setup.setup.aero = Aerodynamics{};
        setup.setup.aero->brake_duct_front_pct = 150.0;  // Invalid: > 100%

        setup.setup.tires = Tires{};
        setup.setup.tires->pressure_fl_kpa = 30.0;  // Too low (warning)
        setup.setup.tires->pressure_fr_kpa = 500.0;  // Too high (warning)

        auto errors = Validator::validate(setup);
        print_validation_results(errors);
    }

    std::cout << std::endl;

    // Example 4: Invalid suspension settings
    std::cout << "--- Example 4: Invalid Suspension Settings ---" << std::endl;
    {
        ORSF setup;
        setup.metadata.id = "suspension-test";
        setup.metadata.name = "Suspension Test";
        setup.metadata.created_at = "2024-01-15T10:00:00Z";
        setup.car.make = "Test";
        setup.car.model = "Car";

        setup.setup.suspension = Suspension{};
        setup.setup.suspension->front_left = CornerSuspension{};
        setup.setup.suspension->front_left->camber_deg = -15.0;  // Extreme (warning)
        setup.setup.suspension->front_left->spring_rate_n_mm = -50.0;  // Negative (error)
        setup.setup.suspension->front_left->damper_bump_slow_n_s_m = -100.0;  // Negative (error)

        auto errors = Validator::validate(setup);
        print_validation_results(errors);
    }

    std::cout << std::endl;

    // Example 5: Temperature consistency check
    std::cout << "--- Example 5: Temperature Consistency ---" << std::endl;
    {
        ORSF setup;
        setup.metadata.id = "temp-test";
        setup.metadata.name = "Temperature Test";
        setup.metadata.created_at = "2024-01-15T10:00:00Z";
        setup.car.make = "Test";
        setup.car.model = "Car";

        setup.context = Context{};
        setup.context->ambient_temp_c = 25.0;
        setup.context->track_temp_c = 10.0;  // Track colder than ambient (warning)

        auto errors = Validator::validate(setup);
        print_validation_results(errors);
    }

    std::cout << std::endl;

    // Example 6: Invalid gear ratios
    std::cout << "--- Example 6: Invalid Gear Ratios ---" << std::endl;
    {
        ORSF setup;
        setup.metadata.id = "gearing-test";
        setup.metadata.name = "Gearing Test";
        setup.metadata.created_at = "2024-01-15T10:00:00Z";
        setup.car.make = "Test";
        setup.car.model = "Car";

        setup.setup.gearing = Gearing{};
        setup.setup.gearing->gear_ratios = {3.5, -2.8, 2.3, 0.0, 1.6};  // Negative and zero ratios

        auto errors = Validator::validate(setup);
        print_validation_results(errors);
    }

    std::cout << std::endl;

    // Example 7: Filtering by severity
    std::cout << "--- Example 7: Filtering by Severity ---" << std::endl;
    {
        ORSF setup;
        setup.metadata.id = "severity-test";
        setup.metadata.name = "Severity Test";
        setup.metadata.created_at = "2024-01-15T10:00:00Z";
        setup.car.make = "Test";
        setup.car.model = "Car";

        setup.setup.aero = Aerodynamics{};
        setup.setup.aero->brake_duct_front_pct = 150.0;  // Error

        setup.context = Context{};
        setup.context->ambient_temp_c = 25.0;
        setup.context->track_temp_c = 10.0;  // Warning

        auto errors = Validator::validate(setup);

        int error_count = 0;
        int warning_count = 0;

        for (const auto& error : errors) {
            if (error.severity == ValidationSeverity::Error) error_count++;
            if (error.severity == ValidationSeverity::Warning) warning_count++;
        }

        std::cout << "Total errors: " << error_count << std::endl;
        std::cout << "Total warnings: " << warning_count << std::endl;
    }

    std::cout << std::endl << "✓ Validation example completed!" << std::endl;

    return 0;
}
