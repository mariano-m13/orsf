/**
 * ORSF Conversion Example
 *
 * Demonstrates:
 * - Unit conversion (kPa ↔ PSI, N/mm ↔ lb/in, etc.)
 * - Transformations
 * - Lookup table interpolation
 * - Flattening and mapping ORSF data
 */

#include <iostream>
#include <iomanip>
#include "orsf/orsf.hpp"

using namespace orsf;

int main() {
    std::cout << "=== ORSF Conversion Example ===" << std::endl << std::endl;
    std::cout << std::fixed << std::setprecision(2);

    // Example 1: Unit conversions
    std::cout << "--- Example 1: Unit Conversions ---" << std::endl;
    {
        // Pressure conversions
        double kpa = 170.0;
        double psi = UnitConverter::convert(kpa, Unit::KPA, Unit::PSI);
        std::cout << kpa << " kPa = " << psi << " PSI" << std::endl;

        double bar = UnitConverter::convert(kpa, Unit::KPA, Unit::BAR);
        std::cout << kpa << " kPa = " << bar << " Bar" << std::endl;

        // Spring rate conversions
        double n_mm = 100.0;
        double lb_in = UnitConverter::convert(n_mm, Unit::N_MM, Unit::LB_IN);
        std::cout << n_mm << " N/mm = " << lb_in << " lb/in" << std::endl;

        // Temperature conversions
        double celsius = 20.0;
        double fahrenheit = UnitConverter::convert(celsius, Unit::CELSIUS, Unit::FAHRENHEIT);
        std::cout << celsius << " °C = " << fahrenheit << " °F" << std::endl;

        // Length conversions
        double mm = 50.8;
        double inches = UnitConverter::convert(mm, Unit::MM, Unit::INCHES);
        std::cout << mm << " mm = " << inches << " inches" << std::endl;
    }

    std::cout << std::endl;

    // Example 2: Transformations
    std::cout << "--- Example 2: Transformations ---" << std::endl;
    {
        // Scale transformation
        auto scale2x = Transform::scale(2.0);
        std::cout << "Scale 2x: 10 -> " << scale2x(10.0) << std::endl;

        // Offset transformation
        auto plus5 = Transform::offset(5.0);
        std::cout << "Offset +5: 10 -> " << plus5(10.0) << std::endl;

        // Linear transformation (scale + offset)
        auto linear = Transform::linear(2.0, 3.0);  // 2x + 3
        std::cout << "Linear (2x + 3): 10 -> " << linear(10.0) << std::endl;

        // Percentage to ratio
        auto pct_to_ratio = Transform::percent_to_ratio();
        std::cout << "Percent to ratio: 75% -> " << pct_to_ratio(75.0) << std::endl;

        // Composed transformation
        auto composed = Transform::compose({
            Transform::scale(2.0),
            Transform::offset(10.0)
        });
        std::cout << "Composed (2x then +10): 5 -> " << composed(5.0) << std::endl;
    }

    std::cout << std::endl;

    // Example 3: Lookup table interpolation
    std::cout << "--- Example 3: Lookup Table Interpolation ---" << std::endl;
    {
        // Wing level to downforce lookup table
        std::vector<LUTEntry> wing_lut = {
            {0.0, 0.0},      // Level 0 = 0N downforce
            {1.0, 500.0},    // Level 1 = 500N
            {2.0, 1200.0},   // Level 2 = 1200N
            {3.0, 2000.0},   // Level 3 = 2000N
            {4.0, 2900.0},   // Level 4 = 2900N
            {5.0, 3900.0}    // Level 5 = 3900N
        };

        LookupTableConverter lut(wing_lut);

        std::cout << "Wing level -> Downforce:" << std::endl;
        std::cout << "  Level 0: " << lut.interpolate(0.0) << " N" << std::endl;
        std::cout << "  Level 2.5: " << lut.interpolate(2.5) << " N (interpolated)" << std::endl;
        std::cout << "  Level 5: " << lut.interpolate(5.0) << " N" << std::endl;

        std::cout << "Reverse lookup (Downforce -> Wing level):" << std::endl;
        std::cout << "  2000N: Level " << lut.reverse_lookup(2000.0) << std::endl;
        std::cout << "  1600N: Level " << lut.reverse_lookup(1600.0) << " (interpolated)" << std::endl;
    }

    std::cout << std::endl;

    // Example 4: Flattening ORSF
    std::cout << "--- Example 4: Flattening ORSF to Key-Value Pairs ---" << std::endl;
    {
        ORSF setup;
        setup.metadata.id = "flatten-test";
        setup.metadata.name = "Flatten Test";
        setup.metadata.created_at = "2024-01-15T10:00:00Z";
        setup.car.make = "Porsche";
        setup.car.model = "911 GT3 R";

        setup.setup.aero = Aerodynamics{};
        setup.setup.aero->front_wing = 2;
        setup.setup.aero->rear_wing = 5;

        setup.setup.tires = Tires{};
        setup.setup.tires->pressure_fl_kpa = 170.0;
        setup.setup.tires->pressure_fr_kpa = 170.0;

        setup.setup.brakes = Brakes{};
        setup.setup.brakes->brake_bias_pct = 58.0;

        FlatSetup flat = MappingEngine::flatten_orsf(setup);

        std::cout << "Flattened setup (" << flat.size() << " fields):" << std::endl;
        for (const auto& [key, value] : flat) {
            std::cout << "  " << key << " = " << value << std::endl;
        }
    }

    std::cout << std::endl;

    // Example 5: Field mapping with unit conversion
    std::cout << "--- Example 5: Field Mapping with Unit Conversion ---" << std::endl;
    {
        ORSF setup;
        setup.metadata.id = "mapping-test";
        setup.metadata.name = "Mapping Test";
        setup.metadata.created_at = "2024-01-15T10:00:00Z";
        setup.car.make = "Porsche";
        setup.car.model = "911 GT3 R";

        setup.setup.tires = Tires{};
        setup.setup.tires->pressure_fl_kpa = 172.0;
        setup.setup.tires->pressure_fr_kpa = 172.0;

        setup.setup.brakes = Brakes{};
        setup.setup.brakes->brake_bias_pct = 58.5;

        // Define field mappings (ORSF -> Native game format)
        std::vector<FieldMapping> mappings = {
            FieldMapping(
                "setup.tires.pressure_fl_kpa",
                "tire_fl_psi",
                Transform::unit_convert(Unit::KPA, Unit::PSI),  // ORSF uses kPa, game uses PSI
                Transform::unit_convert(Unit::PSI, Unit::KPA),
                false
            ),
            FieldMapping(
                "setup.tires.pressure_fr_kpa",
                "tire_fr_psi",
                Transform::unit_convert(Unit::KPA, Unit::PSI),
                Transform::unit_convert(Unit::PSI, Unit::KPA),
                false
            ),
            FieldMapping(
                "setup.brakes.brake_bias_pct",
                "brake_balance",
                Transform::percent_to_ratio(),  // Game uses 0-1 ratio, ORSF uses 0-100%
                Transform::ratio_to_percent(),
                false
            )
        };

        FlatSetup native = MappingEngine::map_to_native(setup, mappings);

        std::cout << "ORSF -> Native game format:" << std::endl;
        std::cout << "  ORSF tire FL: " << setup.setup.tires->pressure_fl_kpa.value() << " kPa" << std::endl;
        std::cout << "  Native tire_fl_psi: " << native["tire_fl_psi"] << " PSI" << std::endl;
        std::cout << std::endl;
        std::cout << "  ORSF brake bias: " << setup.setup.brakes->brake_bias_pct.value() << "%" << std::endl;
        std::cout << "  Native brake_balance: " << native["brake_balance"] << " (ratio)" << std::endl;

        // Convert back from native to ORSF
        std::cout << std::endl << "Native -> ORSF (round-trip):" << std::endl;
        ORSF converted_back = MappingEngine::map_to_orsf(native, mappings, setup);
        std::cout << "  FL pressure: " << converted_back.setup.tires->pressure_fl_kpa.value() << " kPa" << std::endl;
        std::cout << "  Brake bias: " << converted_back.setup.brakes->brake_bias_pct.value() << "%" << std::endl;
    }

    std::cout << std::endl << "✓ Conversion example completed!" << std::endl;

    return 0;
}
