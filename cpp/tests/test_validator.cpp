#include <catch2/catch_test_macros.hpp>
#include "orsf/orsf.hpp"

using namespace orsf;

ORSF create_valid_setup() {
    ORSF setup;
    setup.metadata.id = "test-123";
    setup.metadata.name = "Valid Setup";
    setup.metadata.created_at = "2024-01-01T12:00:00Z";
    setup.car.make = "Porsche";
    setup.car.model = "911 GT3 R";
    return setup;
}

TEST_CASE("Validator accepts valid ORSF", "[validator]") {
    ORSF setup = create_valid_setup();
    auto errors = Validator::validate(setup);
    REQUIRE(errors.empty());
}

TEST_CASE("Validator catches invalid schema", "[validator]") {
    ORSF setup = create_valid_setup();
    setup.schema = "invalid://v99";

    auto errors = Validator::validate(setup);
    REQUIRE_FALSE(errors.empty());

    bool found_schema_error = false;
    for (const auto& error : errors) {
        if (error.code == ValidationCode::SchemaInvalid) {
            found_schema_error = true;
            break;
        }
    }
    REQUIRE(found_schema_error);
}

TEST_CASE("Validator catches missing required metadata", "[validator]") {
    ORSF setup = create_valid_setup();

    SECTION("Missing ID") {
        setup.metadata.id = "";
        auto errors = Validator::validate(setup);
        REQUIRE_FALSE(errors.empty());
    }

    SECTION("Missing name") {
        setup.metadata.name = "";
        auto errors = Validator::validate(setup);
        REQUIRE_FALSE(errors.empty());
    }

    SECTION("Missing created_at") {
        setup.metadata.created_at = "";
        auto errors = Validator::validate(setup);
        REQUIRE_FALSE(errors.empty());
    }
}

TEST_CASE("Validator catches missing required car fields", "[validator]") {
    ORSF setup = create_valid_setup();

    SECTION("Missing make") {
        setup.car.make = "";
        auto errors = Validator::validate(setup);
        REQUIRE_FALSE(errors.empty());
    }

    SECTION("Missing model") {
        setup.car.model = "";
        auto errors = Validator::validate(setup);
        REQUIRE_FALSE(errors.empty());
    }
}

TEST_CASE("Validator validates temperature ranges", "[validator]") {
    ORSF setup = create_valid_setup();
    setup.context = Context{};

    SECTION("Valid temperatures") {
        setup.context->ambient_temp_c = 20.0;
        setup.context->track_temp_c = 30.0;
        auto errors = Validator::validate(setup);

        // Should have no errors or only warnings
        bool has_temp_errors = false;
        for (const auto& error : errors) {
            if (error.severity == ValidationSeverity::Error &&
                (error.field.find("temp") != std::string::npos)) {
                has_temp_errors = true;
            }
        }
        REQUIRE_FALSE(has_temp_errors);
    }

    SECTION("Extreme ambient temperature") {
        setup.context->ambient_temp_c = -100.0;
        auto errors = Validator::validate(setup);
        REQUIRE_FALSE(errors.empty());
    }

    SECTION("Track temp below ambient by large margin") {
        setup.context->ambient_temp_c = 25.0;
        setup.context->track_temp_c = 10.0;
        auto errors = Validator::validate(setup);

        // Should have warning about incompatible temps
        bool found_warning = false;
        for (const auto& error : errors) {
            if (error.code == ValidationCode::Incompatible) {
                found_warning = true;
            }
        }
        REQUIRE(found_warning);
    }
}

TEST_CASE("Validator validates wetness range", "[validator]") {
    ORSF setup = create_valid_setup();
    setup.context = Context{};

    SECTION("Valid wetness") {
        setup.context->wetness = 0.5;
        auto errors = Validator::validate(setup);
        bool has_wetness_errors = false;
        for (const auto& error : errors) {
            if (error.field == "context.wetness") {
                has_wetness_errors = true;
            }
        }
        REQUIRE_FALSE(has_wetness_errors);
    }

    SECTION("Invalid wetness (negative)") {
        setup.context->wetness = -0.1;
        auto errors = Validator::validate(setup);
        REQUIRE_FALSE(errors.empty());
    }

    SECTION("Invalid wetness (> 1)") {
        setup.context->wetness = 1.5;
        auto errors = Validator::validate(setup);
        REQUIRE_FALSE(errors.empty());
    }
}

TEST_CASE("Validator validates aero settings", "[validator]") {
    ORSF setup = create_valid_setup();
    setup.setup.aero = Aerodynamics{};

    SECTION("Valid brake duct percentages") {
        setup.setup.aero->brake_duct_front_pct = 50.0;
        setup.setup.aero->brake_duct_rear_pct = 60.0;
        auto errors = Validator::validate(setup);

        bool has_duct_errors = false;
        for (const auto& error : errors) {
            if (error.field.find("brake_duct") != std::string::npos) {
                has_duct_errors = true;
            }
        }
        REQUIRE_FALSE(has_duct_errors);
    }

    SECTION("Invalid brake duct percentage") {
        setup.setup.aero->brake_duct_front_pct = 150.0;
        auto errors = Validator::validate(setup);
        REQUIRE_FALSE(errors.empty());
    }

    SECTION("Negative ride height") {
        setup.setup.aero->front_ride_height_mm = -5.0;
        auto errors = Validator::validate(setup);
        REQUIRE_FALSE(errors.empty());
    }
}

TEST_CASE("Validator validates suspension settings", "[validator]") {
    ORSF setup = create_valid_setup();
    setup.setup.suspension = Suspension{};
    setup.setup.suspension->front_left = CornerSuspension{};

    SECTION("Valid camber") {
        setup.setup.suspension->front_left->camber_deg = -2.5;
        auto errors = Validator::validate(setup);

        bool has_camber_errors = false;
        for (const auto& error : errors) {
            if (error.field.find("camber") != std::string::npos &&
                error.severity == ValidationSeverity::Error) {
                has_camber_errors = true;
            }
        }
        REQUIRE_FALSE(has_camber_errors);
    }

    SECTION("Extreme camber (warning)") {
        setup.setup.suspension->front_left->camber_deg = -15.0;
        auto errors = Validator::validate(setup);
        REQUIRE_FALSE(errors.empty());
    }

    SECTION("Negative spring rate") {
        setup.setup.suspension->front_left->spring_rate_n_mm = -50.0;
        auto errors = Validator::validate(setup);
        REQUIRE_FALSE(errors.empty());
    }

    SECTION("Negative damper values") {
        setup.setup.suspension->front_left->damper_bump_slow_n_s_m = -100.0;
        auto errors = Validator::validate(setup);
        REQUIRE_FALSE(errors.empty());
    }
}

TEST_CASE("Validator validates tire pressures", "[validator]") {
    ORSF setup = create_valid_setup();
    setup.setup.tires = Tires{};

    SECTION("Valid tire pressures") {
        setup.setup.tires->pressure_fl_kpa = 170.0;
        setup.setup.tires->pressure_fr_kpa = 170.0;
        setup.setup.tires->pressure_rl_kpa = 165.0;
        setup.setup.tires->pressure_rr_kpa = 165.0;

        auto errors = Validator::validate(setup);

        bool has_pressure_errors = false;
        for (const auto& error : errors) {
            if (error.field.find("pressure") != std::string::npos &&
                error.severity == ValidationSeverity::Error) {
                has_pressure_errors = true;
            }
        }
        REQUIRE_FALSE(has_pressure_errors);
    }

    SECTION("Extreme tire pressure (low)") {
        setup.setup.tires->pressure_fl_kpa = 30.0;
        auto errors = Validator::validate(setup);
        REQUIRE_FALSE(errors.empty());
    }

    SECTION("Extreme tire pressure (high)") {
        setup.setup.tires->pressure_fl_kpa = 500.0;
        auto errors = Validator::validate(setup);
        REQUIRE_FALSE(errors.empty());
    }
}

TEST_CASE("Validator validates drivetrain settings", "[validator]") {
    ORSF setup = create_valid_setup();
    setup.setup.drivetrain = Drivetrain{};

    SECTION("Valid diff settings") {
        setup.setup.drivetrain->diff_preload_nm = 50.0;
        setup.setup.drivetrain->diff_power_ramp_pct = 75.0;
        setup.setup.drivetrain->diff_coast_ramp_pct = 45.0;

        auto errors = Validator::validate(setup);

        bool has_diff_errors = false;
        for (const auto& error : errors) {
            if (error.field.find("drivetrain") != std::string::npos &&
                error.severity == ValidationSeverity::Error) {
                has_diff_errors = true;
            }
        }
        REQUIRE_FALSE(has_diff_errors);
    }

    SECTION("Invalid ramp percentage") {
        setup.setup.drivetrain->diff_power_ramp_pct = 150.0;
        auto errors = Validator::validate(setup);
        REQUIRE_FALSE(errors.empty());
    }

    SECTION("Negative preload") {
        setup.setup.drivetrain->diff_preload_nm = -10.0;
        auto errors = Validator::validate(setup);
        REQUIRE_FALSE(errors.empty());
    }
}

TEST_CASE("Validator validates gearing", "[validator]") {
    ORSF setup = create_valid_setup();
    setup.setup.gearing = Gearing{};

    SECTION("Valid gear ratios") {
        setup.setup.gearing->gear_ratios = {3.5, 2.8, 2.3, 1.9, 1.6, 1.4};
        auto errors = Validator::validate(setup);

        bool has_gearing_errors = false;
        for (const auto& error : errors) {
            if (error.field.find("gearing") != std::string::npos &&
                error.severity == ValidationSeverity::Error) {
                has_gearing_errors = true;
            }
        }
        REQUIRE_FALSE(has_gearing_errors);
    }

    SECTION("Invalid gear ratio (negative)") {
        setup.setup.gearing->gear_ratios = {3.5, -2.8, 2.3};
        auto errors = Validator::validate(setup);
        REQUIRE_FALSE(errors.empty());
    }

    SECTION("Invalid gear ratio (zero)") {
        setup.setup.gearing->gear_ratios = {3.5, 0.0, 2.3};
        auto errors = Validator::validate(setup);
        REQUIRE_FALSE(errors.empty());
    }
}

TEST_CASE("Validator validates brakes", "[validator]") {
    ORSF setup = create_valid_setup();
    setup.setup.brakes = Brakes{};

    SECTION("Valid brake bias") {
        setup.setup.brakes->brake_bias_pct = 58.5;
        auto errors = Validator::validate(setup);

        bool has_brake_errors = false;
        for (const auto& error : errors) {
            if (error.field.find("brake_bias") != std::string::npos &&
                error.severity == ValidationSeverity::Error) {
                has_brake_errors = true;
            }
        }
        REQUIRE_FALSE(has_brake_errors);
    }

    SECTION("Invalid brake bias") {
        setup.setup.brakes->brake_bias_pct = 150.0;
        auto errors = Validator::validate(setup);
        REQUIRE_FALSE(errors.empty());
    }
}

TEST_CASE("ValidationError to_string works", "[validator]") {
    ValidationError error(
        ValidationSeverity::Error,
        ValidationCode::OutOfRange,
        "test.field",
        "Test error message",
        "0-100",
        "150"
    );

    std::string str = error.to_string();

    REQUIRE(str.find("[ERROR]") != std::string::npos);
    REQUIRE(str.find("test.field") != std::string::npos);
    REQUIRE(str.find("Test error message") != std::string::npos);
    REQUIRE(str.find("expected: 0-100") != std::string::npos);
    REQUIRE(str.find("actual: 150") != std::string::npos);
}
