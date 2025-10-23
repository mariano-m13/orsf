#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "orsf/orsf.hpp"

using namespace orsf;
using Catch::Approx;

ORSF create_test_setup() {
    ORSF setup;
    setup.metadata.id = "test-123";
    setup.metadata.name = "Test Setup";
    setup.metadata.created_at = "2024-01-01T00:00:00Z";
    setup.car.make = "Porsche";
    setup.car.model = "911 GT3 R";

    // Aero
    setup.setup.aero = Aerodynamics{};
    setup.setup.aero->front_wing = 2;
    setup.setup.aero->rear_wing = 4;

    // Tires
    setup.setup.tires = Tires{};
    setup.setup.tires->pressure_fl_kpa = 170.0;
    setup.setup.tires->pressure_fr_kpa = 170.0;
    setup.setup.tires->pressure_rl_kpa = 165.0;
    setup.setup.tires->pressure_rr_kpa = 165.0;

    // Brakes
    setup.setup.brakes = Brakes{};
    setup.setup.brakes->brake_bias_pct = 58.0;

    return setup;
}

TEST_CASE("MappingEngine flattens ORSF correctly", "[mapping]") {
    ORSF setup = create_test_setup();
    FlatSetup flat = MappingEngine::flatten_orsf(setup);

    REQUIRE(flat["setup.aero.front_wing"] == 2.0);
    REQUIRE(flat["setup.aero.rear_wing"] == 4.0);
    REQUIRE(flat["setup.tires.pressure_fl_kpa"] == 170.0);
    REQUIRE(flat["setup.tires.pressure_rl_kpa"] == 165.0);
    REQUIRE(flat["setup.brakes.brake_bias_pct"] == 58.0);
}

TEST_CASE("MappingEngine get_value retrieves values correctly", "[mapping]") {
    ORSF setup = create_test_setup();

    SECTION("Get aero values") {
        auto front_wing = MappingEngine::get_value(setup, "setup.aero.front_wing");
        REQUIRE(front_wing.has_value());
        REQUIRE(front_wing.value() == 2.0);

        auto rear_wing = MappingEngine::get_value(setup, "setup.aero.rear_wing");
        REQUIRE(rear_wing.has_value());
        REQUIRE(rear_wing.value() == 4.0);
    }

    SECTION("Get tire pressures") {
        auto pressure_fl = MappingEngine::get_value(setup, "setup.tires.pressure_fl_kpa");
        REQUIRE(pressure_fl.has_value());
        REQUIRE(pressure_fl.value() == 170.0);
    }

    SECTION("Get non-existent value") {
        auto missing = MappingEngine::get_value(setup, "setup.nonexistent.field");
        REQUIRE_FALSE(missing.has_value());
    }
}

TEST_CASE("MappingEngine set_value modifies values correctly", "[mapping]") {
    ORSF setup = create_test_setup();

    SECTION("Set aero values") {
        MappingEngine::set_value(setup, "setup.aero.front_wing", 5.0);
        REQUIRE(setup.setup.aero->front_wing.value() == 5.0);
    }

    SECTION("Set tire pressure") {
        MappingEngine::set_value(setup, "setup.tires.pressure_fl_kpa", 180.0);
        REQUIRE(setup.setup.tires->pressure_fl_kpa.value() == 180.0);
    }

    SECTION("Set brake bias") {
        MappingEngine::set_value(setup, "setup.brakes.brake_bias_pct", 60.0);
        REQUIRE(setup.setup.brakes->brake_bias_pct.value() == 60.0);
    }
}

TEST_CASE("MappingEngine handles suspension correctly", "[mapping]") {
    ORSF setup = create_test_setup();

    setup.setup.suspension = Suspension{};
    setup.setup.suspension->front_left = CornerSuspension{};
    setup.setup.suspension->front_left->camber_deg = -2.5;
    setup.setup.suspension->front_left->spring_rate_n_mm = 90.0;

    FlatSetup flat = MappingEngine::flatten_orsf(setup);

    REQUIRE(flat["setup.suspension.front_left.camber_deg"] == Approx(-2.5).margin(0.001));
    REQUIRE(flat["setup.suspension.front_left.spring_rate_n_mm"] == Approx(90.0).margin(0.001));
}

TEST_CASE("MappingEngine handles gearing correctly", "[mapping]") {
    ORSF setup = create_test_setup();

    setup.setup.gearing = Gearing{};
    setup.setup.gearing->gear_ratios = {3.5, 2.8, 2.3, 1.9, 1.6, 1.4};

    FlatSetup flat = MappingEngine::flatten_orsf(setup);

    REQUIRE(flat["setup.gearing.gear_0"] == Approx(3.5).margin(0.001));
    REQUIRE(flat["setup.gearing.gear_1"] == Approx(2.8).margin(0.001));
    REQUIRE(flat["setup.gearing.gear_5"] == Approx(1.4).margin(0.001));
}

TEST_CASE("MappingEngine maps to native format with transformations", "[mapping]") {
    ORSF setup = create_test_setup();

    std::vector<FieldMapping> mappings = {
        FieldMapping(
            "setup.tires.pressure_fl_kpa",
            "tire_fl_psi",
            Transform::unit_convert(Unit::KPA, Unit::PSI),  // ORSF to native
            Transform::unit_convert(Unit::PSI, Unit::KPA),  // Native to ORSF
            false
        ),
        FieldMapping(
            "setup.brakes.brake_bias_pct",
            "brake_balance",
            Transform::percent_to_ratio(),  // Convert 58% to 0.58
            Transform::ratio_to_percent(),  // Convert 0.58 to 58%
            false
        )
    };

    FlatSetup native = MappingEngine::map_to_native(setup, mappings);

    // Check tire pressure conversion (170 kPa ≈ 24.66 PSI)
    REQUIRE(native["tire_fl_psi"] == Approx(24.66).margin(0.1));

    // Check brake bias conversion (58% to 0.58)
    REQUIRE(native["brake_balance"] == Approx(0.58).margin(0.001));
}

TEST_CASE("MappingEngine maps from native format with transformations", "[mapping]") {
    ORSF template_setup = create_test_setup();

    FlatSetup native;
    native["tire_fl_psi"] = 25.0;  // PSI
    native["brake_balance"] = 0.6;  // Ratio

    std::vector<FieldMapping> mappings = {
        FieldMapping(
            "setup.tires.pressure_fl_kpa",
            "tire_fl_psi",
            Transform::unit_convert(Unit::KPA, Unit::PSI),
            Transform::unit_convert(Unit::PSI, Unit::KPA),
            false
        ),
        FieldMapping(
            "setup.brakes.brake_bias_pct",
            "brake_balance",
            Transform::percent_to_ratio(),
            Transform::ratio_to_percent(),
            false
        )
    };

    ORSF result = MappingEngine::map_to_orsf(native, mappings, template_setup);

    // Check tire pressure conversion (25 PSI ≈ 172.4 kPa)
    REQUIRE(result.setup.tires->pressure_fl_kpa.value() == Approx(172.4).margin(0.5));

    // Check brake bias conversion (0.6 to 60%)
    REQUIRE(result.setup.brakes->brake_bias_pct.value() == Approx(60.0).margin(0.001));
}

TEST_CASE("FieldMapping with scale transformation", "[mapping]") {
    ORSF setup = create_test_setup();
    setup.setup.aero->front_wing = 5.0;

    std::vector<FieldMapping> mappings = {
        FieldMapping(
            "setup.aero.front_wing",
            "front_aero_level",
            Transform::scale(10.0),  // Multiply by 10
            Transform::scale(0.1),   // Divide by 10 (reverse)
            false
        )
    };

    FlatSetup native = MappingEngine::map_to_native(setup, mappings);
    REQUIRE(native["front_aero_level"] == Approx(50.0).margin(0.001));

    // Reverse mapping
    ORSF result = MappingEngine::map_to_orsf(native, mappings, setup);
    REQUIRE(result.setup.aero->front_wing.value() == Approx(5.0).margin(0.001));
}

TEST_CASE("FieldMapping with composed transformation", "[mapping]") {
    ORSF setup = create_test_setup();
    setup.setup.aero->front_wing = 2.0;

    auto to_native = Transform::compose({
        Transform::scale(2.0),      // Multiply by 2 (2 -> 4)
        Transform::offset(1.0)      // Add 1 (4 -> 5)
    });

    auto to_orsf = Transform::compose({
        Transform::offset(-1.0),    // Subtract 1 (5 -> 4)
        Transform::scale(0.5)       // Divide by 2 (4 -> 2)
    });

    std::vector<FieldMapping> mappings = {
        FieldMapping("setup.aero.front_wing", "native_wing", to_native, to_orsf, false)
    };

    FlatSetup native = MappingEngine::map_to_native(setup, mappings);
    REQUIRE(native["native_wing"] == Approx(5.0).margin(0.001));

    // Reverse mapping
    ORSF result = MappingEngine::map_to_orsf(native, mappings, setup);
    REQUIRE(result.setup.aero->front_wing.value() == Approx(2.0).margin(0.001));
}
