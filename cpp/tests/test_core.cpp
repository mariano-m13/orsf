#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_string.hpp>
#include "orsf/orsf.hpp"

using namespace orsf;

TEST_CASE("ORSF can be created with default schema", "[core]") {
    ORSF setup;
    REQUIRE(setup.schema == "orsf://v1");
}

TEST_CASE("ORSF can be serialized to JSON", "[core]") {
    ORSF setup;
    setup.metadata.id = "test-id";
    setup.metadata.name = "Test Setup";
    setup.metadata.created_at = "2024-01-01T12:00:00Z";
    setup.car.make = "Porsche";
    setup.car.model = "911 GT3 R";

    std::string json_str = setup.to_json_string();

    REQUIRE_FALSE(json_str.empty());
    REQUIRE(json_str.find("orsf://v1") != std::string::npos);
    REQUIRE(json_str.find("test-id") != std::string::npos);
    REQUIRE(json_str.find("Porsche") != std::string::npos);
}

TEST_CASE("ORSF can be deserialized from JSON", "[core]") {
    std::string json_str = R"({
        "schema": "orsf://v1",
        "metadata": {
            "id": "abc123",
            "name": "Spa Quali Setup",
            "created_at": "2024-01-15T10:30:00Z"
        },
        "car": {
            "make": "Porsche",
            "model": "911 GT3 R",
            "class": "GT3"
        },
        "setup": {
            "aero": {
                "front_wing": 3,
                "rear_wing": 5
            }
        }
    })";

    ORSF setup = ORSF::from_json(json_str);

    REQUIRE(setup.schema == "orsf://v1");
    REQUIRE(setup.metadata.id == "abc123");
    REQUIRE(setup.metadata.name == "Spa Quali Setup");
    REQUIRE(setup.car.make == "Porsche");
    REQUIRE(setup.car.model == "911 GT3 R");
    REQUIRE(setup.car.car_class.value() == "GT3");
    REQUIRE(setup.setup.aero.has_value());
    REQUIRE(setup.setup.aero->front_wing.value() == 3.0);
    REQUIRE(setup.setup.aero->rear_wing.value() == 5.0);
}

TEST_CASE("ORSF rejects invalid schema version", "[core]") {
    std::string json_str = R"({
        "schema": "orsf://v99",
        "metadata": {
            "id": "test",
            "name": "Test",
            "created_at": "2024-01-01T00:00:00Z"
        },
        "car": {
            "make": "Test",
            "model": "Test"
        },
        "setup": {}
    })";

    REQUIRE_THROWS_AS(ORSF::from_json(json_str), std::runtime_error);
}

TEST_CASE("ORSF handles optional fields correctly", "[core]") {
    ORSF setup;
    setup.metadata.id = "test";
    setup.metadata.name = "Test";
    setup.metadata.created_at = "2024-01-01T00:00:00Z";
    setup.car.make = "Test";
    setup.car.model = "Car";

    // Optional context
    setup.context = Context{};
    setup.context->track = "Spa-Francorchamps";
    setup.context->ambient_temp_c = 20.0;

    std::string json_str = setup.to_json_string();
    ORSF parsed = ORSF::from_json(json_str);

    REQUIRE(parsed.context.has_value());
    REQUIRE(parsed.context->track.value() == "Spa-Francorchamps");
    REQUIRE(parsed.context->ambient_temp_c.value() == 20.0);
}

TEST_CASE("ORSF handles complex setup data", "[core]") {
    ORSF setup;
    setup.metadata.id = "complex-test";
    setup.metadata.name = "Complex Setup";
    setup.metadata.created_at = "2024-01-01T00:00:00Z";
    setup.car.make = "Porsche";
    setup.car.model = "911 GT3 R";

    // Aero
    setup.setup.aero = Aerodynamics{};
    setup.setup.aero->front_wing = 2;
    setup.setup.aero->rear_wing = 4;
    setup.setup.aero->front_ride_height_mm = 55.0;
    setup.setup.aero->rear_ride_height_mm = 60.0;

    // Suspension
    setup.setup.suspension = Suspension{};
    setup.setup.suspension->front_left = CornerSuspension{};
    setup.setup.suspension->front_left->camber_deg = -2.8;
    setup.setup.suspension->front_left->toe_deg = 0.1;
    setup.setup.suspension->front_left->spring_rate_n_mm = 90.0;

    // Tires
    setup.setup.tires = Tires{};
    setup.setup.tires->compound = "Medium";
    setup.setup.tires->pressure_fl_kpa = 170.0;
    setup.setup.tires->pressure_fr_kpa = 170.0;
    setup.setup.tires->pressure_rl_kpa = 165.0;
    setup.setup.tires->pressure_rr_kpa = 165.0;

    // Serialize and deserialize
    std::string json_str = setup.to_json_string(2);
    ORSF parsed = ORSF::from_json(json_str);

    REQUIRE(parsed.setup.aero.has_value());
    REQUIRE(parsed.setup.aero->front_wing.value() == 2.0);
    REQUIRE(parsed.setup.suspension.has_value());
    REQUIRE(parsed.setup.suspension->front_left.has_value());
    REQUIRE(parsed.setup.suspension->front_left->camber_deg.value() == -2.8);
    REQUIRE(parsed.setup.tires.has_value());
    REQUIRE(parsed.setup.tires->compound.value() == "Medium");
    REQUIRE(parsed.setup.tires->pressure_fl_kpa.value() == 170.0);
}

TEST_CASE("Metadata structure validation", "[core]") {
    Metadata meta;
    meta.id = "test-123";
    meta.name = "Test Metadata";
    meta.created_at = "2024-01-01T00:00:00Z";
    meta.tags = std::vector<std::string>{"quali", "dry", "high-downforce"};
    meta.source = "coach_dave";

    REQUIRE(meta.id == "test-123");
    REQUIRE(meta.name == "Test Metadata");
    REQUIRE(meta.tags.has_value());
    REQUIRE(meta.tags->size() == 3);
    REQUIRE(meta.tags->at(0) == "quali");
}

TEST_CASE("Car structure validation", "[core]") {
    Car car;
    car.make = "Mercedes";
    car.model = "AMG GT3";
    car.car_class = "GT3";
    car.variant = "2020";
    car.bop_id = "bop_2024_1";

    REQUIRE(car.make == "Mercedes");
    REQUIRE(car.car_class.value() == "GT3");
    REQUIRE(car.bop_id.value() == "bop_2024_1");
}

TEST_CASE("Context structure validation", "[core]") {
    Context ctx;
    ctx.track = "Monza";
    ctx.layout = "Grand Prix";
    ctx.ambient_temp_c = 25.0;
    ctx.track_temp_c = 35.0;
    ctx.rubber = "medium";
    ctx.wetness = 0.0;
    ctx.session_type = "race";

    REQUIRE(ctx.track.value() == "Monza");
    REQUIRE(ctx.ambient_temp_c.value() == 25.0);
    REQUIRE(ctx.rubber.value() == "medium");
    REQUIRE(ctx.wetness.value() == 0.0);
}
