/**
 * ORSF Adapter Example
 *
 * Demonstrates:
 * - Creating custom adapters for game-specific formats
 * - Registering adapters
 * - Converting between ORSF and native formats
 */

#include <iostream>
#include "orsf/orsf.hpp"

using namespace orsf;

/**
 * Example custom adapter for a fictional racing game
 */
class CustomGameAdapter : public BaseAdapter {
public:
    CustomGameAdapter()
        : BaseAdapter(
            "custom_game",
            "1.0",
            "gt3_car",
            "Custom racing game adapter",
            "Example Author"
        ) {}

    std::vector<uint8_t> orsf_to_native(const ORSF& orsf) const override {
        std::cout << "  Converting ORSF to custom game format..." << std::endl;

        // Get field mappings
        FlatSetup native = orsf_to_flat(orsf);

        // For this example, we'll create a simple text format
        std::string output = "[CustomGameSetup]\n";
        output += "name=" + orsf.metadata.name + "\n";
        output += "car=" + orsf.car.make + " " + orsf.car.model + "\n\n";

        output += "[Settings]\n";
        for (const auto& [key, value] : native) {
            output += key + "=" + std::to_string(value) + "\n";
        }

        return std::vector<uint8_t>(output.begin(), output.end());
    }

    ORSF native_to_orsf(const std::vector<uint8_t>& data) const override {
        std::cout << "  Converting custom game format to ORSF..." << std::endl;

        // For this example, we'll just parse the text format
        std::string content(data.begin(), data.end());

        ORSF setup;
        setup.metadata.id = "converted-" + std::to_string(std::time(nullptr));
        setup.metadata.name = "Converted Setup";
        setup.metadata.created_at = DateTimeUtils::now_iso8601();
        setup.car.make = "Custom";
        setup.car.model = "Car";

        // In a real adapter, you would parse the native format here
        // and populate the ORSF structure

        return setup;
    }

    std::string get_suggested_filename() const override {
        return "setup_custom.cfg";
    }

    std::string get_file_extension() const override {
        return "cfg";
    }

    std::optional<std::string> get_install_path() const override {
        // In a real adapter, you might detect the game installation path
        return std::nullopt;
    }

    std::vector<FieldMapping> get_field_mappings() const override {
        // Define how ORSF fields map to the native format
        return {
            // Aero (no conversion needed)
            FieldMapping("setup.aero.front_wing", "aero_front", std::nullopt, std::nullopt, false),
            FieldMapping("setup.aero.rear_wing", "aero_rear", std::nullopt, std::nullopt, false),

            // Tires (convert kPa to PSI)
            FieldMapping(
                "setup.tires.pressure_fl_kpa",
                "tire_fl",
                Transform::unit_convert(Unit::KPA, Unit::PSI),
                Transform::unit_convert(Unit::PSI, Unit::KPA),
                false
            ),
            FieldMapping(
                "setup.tires.pressure_fr_kpa",
                "tire_fr",
                Transform::unit_convert(Unit::KPA, Unit::PSI),
                Transform::unit_convert(Unit::PSI, Unit::KPA),
                false
            ),
            FieldMapping(
                "setup.tires.pressure_rl_kpa",
                "tire_rl",
                Transform::unit_convert(Unit::KPA, Unit::PSI),
                Transform::unit_convert(Unit::PSI, Unit::KPA),
                false
            ),
            FieldMapping(
                "setup.tires.pressure_rr_kpa",
                "tire_rr",
                Transform::unit_convert(Unit::KPA, Unit::PSI),
                Transform::unit_convert(Unit::PSI, Unit::KPA),
                false
            ),

            // Brakes (convert percentage to ratio)
            FieldMapping(
                "setup.brakes.brake_bias_pct",
                "brake_balance",
                Transform::percent_to_ratio(),
                Transform::ratio_to_percent(),
                false
            ),
        };
    }
};

int main() {
    std::cout << "=== ORSF Adapter Example ===" << std::endl << std::endl;

    // Get the adapter registry
    auto& registry = AdapterRegistry::instance();
    registry.clear();

    // Register the example adapter (already available)
    std::cout << "--- Registering Adapters ---" << std::endl;
    auto example_adapter = std::make_shared<ExampleAdapter>();
    registry.register_adapter(example_adapter);
    std::cout << "Registered: " << example_adapter->get_id()
              << " v" << example_adapter->get_version() << std::endl;

    // Register our custom adapter
    auto custom_adapter = std::make_shared<CustomGameAdapter>();
    registry.register_adapter(custom_adapter);
    std::cout << "Registered: " << custom_adapter->get_id()
              << " v" << custom_adapter->get_version() << std::endl;

    std::cout << std::endl;

    // List all registered adapters
    std::cout << "--- All Registered Adapters ---" << std::endl;
    auto all_adapters = registry.get_all_adapters();
    for (const auto& adapter : all_adapters) {
        auto metadata = adapter->get_metadata();
        std::cout << "  " << metadata.id << " v" << metadata.version
                  << " - " << metadata.description << std::endl;
    }

    std::cout << std::endl;

    // Create a test ORSF setup
    std::cout << "--- Creating Test Setup ---" << std::endl;
    ORSF setup;
    setup.metadata.id = "adapter-test";
    setup.metadata.name = "Spa Race Setup";
    setup.metadata.created_at = DateTimeUtils::now_iso8601();
    setup.car.make = "Porsche";
    setup.car.model = "911 GT3 R";
    setup.car.car_class = "GT3";

    setup.setup.aero = Aerodynamics{};
    setup.setup.aero->front_wing = 3;
    setup.setup.aero->rear_wing = 5;

    setup.setup.tires = Tires{};
    setup.setup.tires->pressure_fl_kpa = 170.0;
    setup.setup.tires->pressure_fr_kpa = 170.0;
    setup.setup.tires->pressure_rl_kpa = 165.0;
    setup.setup.tires->pressure_rr_kpa = 165.0;

    setup.setup.brakes = Brakes{};
    setup.setup.brakes->brake_bias_pct = 58.0;

    std::cout << "Created: " << setup.metadata.name << std::endl;
    std::cout << std::endl;

    // Resolve and use the custom adapter
    std::cout << "--- Using Custom Game Adapter ---" << std::endl;
    auto resolved_adapter = registry.resolve("custom_game");

    if (resolved_adapter) {
        std::cout << "Resolved adapter: " << resolved_adapter->get_id() << std::endl;
        std::cout << "File extension: ." << resolved_adapter->get_file_extension() << std::endl;
        std::cout << "Suggested filename: " << resolved_adapter->get_suggested_filename() << std::endl;
        std::cout << std::endl;

        // Convert ORSF to native format
        std::cout << "Converting ORSF to native format..." << std::endl;
        auto native_data = resolved_adapter->orsf_to_native(setup);

        std::cout << "Native format output (" << native_data.size() << " bytes):" << std::endl;
        std::string native_str(native_data.begin(), native_data.end());
        std::cout << "---" << std::endl;
        std::cout << native_str;
        std::cout << "---" << std::endl;
        std::cout << std::endl;

        // Validate the ORSF before conversion
        std::cout << "Validating ORSF..." << std::endl;
        auto errors = resolved_adapter->validate_orsf(setup);

        if (errors.empty()) {
            std::cout << "✓ Setup is valid!" << std::endl;
        } else {
            std::cout << "Found " << errors.size() << " validation issue(s):" << std::endl;
            for (const auto& error : errors) {
                std::cout << "  " << error.to_string() << std::endl;
            }
        }
    } else {
        std::cout << "Failed to resolve adapter!" << std::endl;
    }

    std::cout << std::endl;

    // Example: Resolve adapter for specific game version and car
    std::cout << "--- Resolving Specific Adapter ---" << std::endl;
    auto specific = registry.resolve("example", "1.0", "generic");
    if (specific) {
        std::cout << "Resolved: " << specific->get_metadata().description << std::endl;
    }

    std::cout << std::endl;

    // Get all adapters for a specific game
    std::cout << "--- Adapters for 'custom_game' ---" << std::endl;
    auto custom_adapters = registry.get_adapters_for_game("custom_game");
    std::cout << "Found " << custom_adapters.size() << " adapter(s)" << std::endl;
    for (const auto& adapter : custom_adapters) {
        std::cout << "  - " << adapter->get_car_key() << " (v" << adapter->get_version() << ")" << std::endl;
    }

    std::cout << std::endl << "✓ Adapter example completed!" << std::endl;

    return 0;
}
