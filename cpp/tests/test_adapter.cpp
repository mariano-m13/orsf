#include <catch2/catch_test_macros.hpp>
#include "orsf/orsf.hpp"

using namespace orsf;

TEST_CASE("AdapterRegistry is a singleton", "[adapter]") {
    auto& registry1 = AdapterRegistry::instance();
    auto& registry2 = AdapterRegistry::instance();

    REQUIRE(&registry1 == &registry2);
}

TEST_CASE("AdapterRegistry can register and resolve adapters", "[adapter]") {
    auto& registry = AdapterRegistry::instance();
    registry.clear();

    auto adapter = std::make_shared<ExampleAdapter>();
    registry.register_adapter(adapter);

    SECTION("Resolve by exact match") {
        auto resolved = registry.resolve("example", "1.0", "generic");
        REQUIRE(resolved != nullptr);
        REQUIRE(resolved->get_id() == "example");
    }

    SECTION("Resolve with partial match") {
        auto resolved = registry.resolve("example");
        REQUIRE(resolved != nullptr);
        REQUIRE(resolved->get_id() == "example");
    }

    SECTION("Resolve non-existent adapter") {
        auto resolved = registry.resolve("nonexistent");
        REQUIRE(resolved == nullptr);
    }

    registry.clear();
}

TEST_CASE("AdapterRegistry can unregister adapters", "[adapter]") {
    auto& registry = AdapterRegistry::instance();
    registry.clear();

    auto adapter = std::make_shared<ExampleAdapter>();
    registry.register_adapter(adapter);

    auto resolved = registry.resolve("example");
    REQUIRE(resolved != nullptr);

    registry.unregister_adapter("example", "1.0", "generic");

    resolved = registry.resolve("example");
    REQUIRE(resolved == nullptr);

    registry.clear();
}

TEST_CASE("AdapterRegistry can get all adapters", "[adapter]") {
    auto& registry = AdapterRegistry::instance();
    registry.clear();

    auto adapter1 = std::make_shared<ExampleAdapter>();
    registry.register_adapter(adapter1);

    auto all_adapters = registry.get_all_adapters();
    REQUIRE(all_adapters.size() == 1);

    registry.clear();
}

TEST_CASE("ExampleAdapter converts ORSF to native", "[adapter]") {
    ORSF setup;
    setup.metadata.id = "test";
    setup.metadata.name = "Test Setup";
    setup.metadata.created_at = "2024-01-01T00:00:00Z";
    setup.car.make = "Porsche";
    setup.car.model = "911 GT3 R";

    ExampleAdapter adapter;
    auto data = adapter.orsf_to_native(setup);

    REQUIRE_FALSE(data.empty());

    // Should be valid JSON
    std::string json_str(data.begin(), data.end());
    REQUIRE(json_str.find("orsf://v1") != std::string::npos);
}

TEST_CASE("ExampleAdapter converts native to ORSF", "[adapter]") {
    std::string json_str = R"({
        "schema": "orsf://v1",
        "metadata": {
            "id": "test",
            "name": "Test",
            "created_at": "2024-01-01T00:00:00Z"
        },
        "car": {
            "make": "Test",
            "model": "Car"
        },
        "setup": {}
    })";

    std::vector<uint8_t> data(json_str.begin(), json_str.end());

    ExampleAdapter adapter;
    ORSF setup = adapter.native_to_orsf(data);

    REQUIRE(setup.schema == "orsf://v1");
    REQUIRE(setup.metadata.id == "test");
    REQUIRE(setup.car.make == "Test");
}

TEST_CASE("ExampleAdapter provides metadata", "[adapter]") {
    ExampleAdapter adapter;

    REQUIRE(adapter.get_id() == "example");
    REQUIRE(adapter.get_version() == "1.0");
    REQUIRE(adapter.get_car_key() == "generic");
    REQUIRE(adapter.get_file_extension() == "json");
    REQUIRE(adapter.get_suggested_filename() == "setup_example.json");

    auto metadata = adapter.get_metadata();
    REQUIRE(metadata.id == "example");
    REQUIRE(metadata.description == "Example adapter for demonstration");
}

TEST_CASE("ExampleAdapter validates ORSF", "[adapter]") {
    ORSF setup;
    setup.metadata.id = "test";
    setup.metadata.name = "Test";
    setup.metadata.created_at = "2024-01-01T00:00:00Z";
    setup.car.make = "Porsche";
    setup.car.model = "911 GT3 R";

    ExampleAdapter adapter;
    auto errors = adapter.validate_orsf(setup);

    REQUIRE(errors.empty());
}

TEST_CASE("BaseAdapter provides default functionality", "[adapter]") {
    class TestAdapter : public BaseAdapter {
    public:
        TestAdapter() : BaseAdapter("test", "1.0", "test-car", "Test adapter", "Test author") {}

        std::vector<uint8_t> orsf_to_native(const ORSF& orsf) const override {
            std::string json = orsf.to_json_string();
            return std::vector<uint8_t>(json.begin(), json.end());
        }

        ORSF native_to_orsf(const std::vector<uint8_t>& data) const override {
            std::string json(data.begin(), data.end());
            return ORSF::from_json(json);
        }

        std::string get_suggested_filename() const override { return "test.json"; }
        std::string get_file_extension() const override { return "json"; }
        std::optional<std::string> get_install_path() const override { return std::nullopt; }
        std::vector<FieldMapping> get_field_mappings() const override { return {}; }
    };

    TestAdapter adapter;

    REQUIRE(adapter.get_id() == "test");
    REQUIRE(adapter.get_version() == "1.0");
    REQUIRE(adapter.get_car_key() == "test-car");

    auto metadata = adapter.get_metadata();
    REQUIRE(metadata.description == "Test adapter");
    REQUIRE(metadata.author == "Test author");
}

TEST_CASE("Adapter field mappings work end-to-end", "[adapter]") {
    ORSF setup;
    setup.metadata.id = "test";
    setup.metadata.name = "Test";
    setup.metadata.created_at = "2024-01-01T00:00:00Z";
    setup.car.make = "Test";
    setup.car.model = "Car";

    setup.setup.tires = Tires{};
    setup.setup.tires->pressure_fl_kpa = 170.0;

    ExampleAdapter adapter;
    auto mappings = adapter.get_field_mappings();

    FlatSetup native = MappingEngine::map_to_native(setup, mappings);

    // Example adapter converts kPa to PSI
    REQUIRE(native.find("tire_fl_pressure") != native.end());
}
