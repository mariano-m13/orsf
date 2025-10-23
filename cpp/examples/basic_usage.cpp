/**
 * Basic ORSF Usage Example
 *
 * Demonstrates:
 * - Creating an ORSF setup from scratch
 * - Setting metadata, car, and setup parameters
 * - Serializing to JSON
 * - Deserializing from JSON
 */

#include <iostream>
#include "orsf/orsf.hpp"

using namespace orsf;

int main() {
    std::cout << "=== ORSF Basic Usage Example ===" << std::endl;
    std::cout << "ORSF Version: " << VERSION << std::endl;
    std::cout << "Schema: " << SCHEMA_VERSION << std::endl << std::endl;

    // Create a new ORSF setup
    ORSF setup;

    // Set metadata
    setup.metadata.id = "spa-quali-2024-01";
    setup.metadata.name = "Spa Qualifying Setup";
    setup.metadata.created_at = DateTimeUtils::now_iso8601();
    setup.metadata.created_by = "John Doe";
    setup.metadata.tags = {"qualifying", "dry", "high-downforce"};
    setup.metadata.notes = "Aggressive setup for qualifying at Spa";

    // Set car information
    setup.car.make = "Porsche";
    setup.car.model = "911 GT3 R";
    setup.car.car_class = "GT3";
    setup.car.variant = "2023";

    // Set context
    setup.context = Context{};
    setup.context->track = "Spa-Francorchamps";
    setup.context->layout = "Grand Prix";
    setup.context->ambient_temp_c = 18.0;
    setup.context->track_temp_c = 28.0;
    setup.context->rubber = "medium";
    setup.context->wetness = 0.0;
    setup.context->session_type = "qualifying";

    // Set aerodynamics
    setup.setup.aero = Aerodynamics{};
    setup.setup.aero->front_wing = 2;
    setup.setup.aero->rear_wing = 5;
    setup.setup.aero->front_ride_height_mm = 53.0;
    setup.setup.aero->rear_ride_height_mm = 58.0;
    setup.setup.aero->brake_duct_front_pct = 45.0;
    setup.setup.aero->brake_duct_rear_pct = 50.0;

    // Set tire pressures
    setup.setup.tires = Tires{};
    setup.setup.tires->compound = "Soft";
    setup.setup.tires->pressure_fl_kpa = 172.0;
    setup.setup.tires->pressure_fr_kpa = 172.0;
    setup.setup.tires->pressure_rl_kpa = 168.0;
    setup.setup.tires->pressure_rr_kpa = 168.0;

    // Set brakes
    setup.setup.brakes = Brakes{};
    setup.setup.brakes->brake_bias_pct = 57.5;

    // Set electronics
    setup.setup.electronics = Electronics{};
    setup.setup.electronics->tc_level = 3;
    setup.setup.electronics->abs_level = 2;

    std::cout << "Created setup: " << setup.metadata.name << std::endl;
    std::cout << "Car: " << setup.car.make << " " << setup.car.model << std::endl;
    std::cout << "Track: " << setup.context->track.value() << std::endl << std::endl;

    // Serialize to JSON
    std::string json_str = setup.to_json_string(2);  // 2-space indentation

    std::cout << "=== JSON Output ===" << std::endl;
    std::cout << json_str << std::endl << std::endl;

    // Deserialize from JSON
    std::cout << "=== Parsing JSON back to ORSF ===" << std::endl;
    ORSF parsed_setup = ORSF::from_json(json_str);

    std::cout << "Parsed setup name: " << parsed_setup.metadata.name << std::endl;
    std::cout << "Front wing: " << parsed_setup.setup.aero->front_wing.value() << std::endl;
    std::cout << "Rear wing: " << parsed_setup.setup.aero->rear_wing.value() << std::endl;
    std::cout << "FL tire pressure: " << parsed_setup.setup.tires->pressure_fl_kpa.value() << " kPa" << std::endl;
    std::cout << "Brake bias: " << parsed_setup.setup.brakes->brake_bias_pct.value() << "%" << std::endl;

    std::cout << std::endl << "✓ Basic usage example completed successfully!" << std::endl;

    return 0;
}
