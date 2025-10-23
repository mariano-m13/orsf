#include "orsf/adapter.hpp"
#include <algorithm>

namespace orsf {

// ============================================================================
// Adapter Registry Implementation
// ============================================================================

AdapterRegistry& AdapterRegistry::instance() {
    static AdapterRegistry instance;
    return instance;
}

void AdapterRegistry::register_adapter(std::shared_ptr<Adapter> adapter) {
    std::lock_guard<std::mutex> lock(mutex_);
    adapters_.push_back(adapter);
}

std::shared_ptr<Adapter> AdapterRegistry::resolve(
    const std::string& id,
    const std::string& version,
    const std::string& car_key
) const {
    std::lock_guard<std::mutex> lock(mutex_);

    // Try exact match first
    for (const auto& adapter : adapters_) {
        bool id_match = adapter->get_id() == id;
        bool version_match = version.empty() || adapter->get_version() == version;
        bool car_match = car_key.empty() || adapter->get_car_key() == car_key;

        if (id_match && version_match && car_match) {
            return adapter;
        }
    }

    // Try partial match (no version/car requirements)
    for (const auto& adapter : adapters_) {
        if (adapter->get_id() == id) {
            return adapter;
        }
    }

    return nullptr;
}

std::vector<std::shared_ptr<Adapter>> AdapterRegistry::get_all_adapters() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return adapters_;
}

std::vector<std::shared_ptr<Adapter>> AdapterRegistry::get_adapters_for_game(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);

    std::vector<std::shared_ptr<Adapter>> result;
    for (const auto& adapter : adapters_) {
        if (adapter->get_id() == id) {
            result.push_back(adapter);
        }
    }

    return result;
}

void AdapterRegistry::unregister_adapter(
    const std::string& id,
    const std::string& version,
    const std::string& car_key
) {
    std::lock_guard<std::mutex> lock(mutex_);

    adapters_.erase(
        std::remove_if(adapters_.begin(), adapters_.end(),
            [&](const std::shared_ptr<Adapter>& adapter) {
                return adapter->get_id() == id &&
                       adapter->get_version() == version &&
                       adapter->get_car_key() == car_key;
            }),
        adapters_.end()
    );
}

void AdapterRegistry::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    adapters_.clear();
}

std::string AdapterRegistry::make_key(
    const std::string& id,
    const std::string& version,
    const std::string& car_key
) {
    return id + ":" + version + ":" + car_key;
}

// ============================================================================
// Base Adapter Implementation
// ============================================================================

BaseAdapter::BaseAdapter(
    const std::string& id,
    const std::string& version,
    const std::string& car_key,
    const std::string& description,
    const std::string& author
) {
    metadata_.id = id;
    metadata_.version = version;
    metadata_.car_key = car_key;
    metadata_.description = description;
    metadata_.author = author;
}

std::vector<ValidationError> BaseAdapter::validate_orsf(const ORSF& orsf) const {
    return Validator::validate(orsf);
}

FlatSetup BaseAdapter::orsf_to_flat(const ORSF& orsf) const {
    return MappingEngine::map_to_native(orsf, get_field_mappings());
}

ORSF BaseAdapter::flat_to_orsf(const FlatSetup& flat, const ORSF& template_orsf) const {
    return MappingEngine::map_to_orsf(flat, get_field_mappings(), template_orsf);
}

// ============================================================================
// Example Adapter Implementation
// ============================================================================

ExampleAdapter::ExampleAdapter()
    : BaseAdapter("example", "1.0", "generic", "Example adapter for demonstration", "ORSF Team") {}

std::vector<uint8_t> ExampleAdapter::orsf_to_native(const ORSF& orsf) const {
    // Example: Convert to JSON bytes
    std::string json_str = orsf.to_json_string(2);
    return std::vector<uint8_t>(json_str.begin(), json_str.end());
}

ORSF ExampleAdapter::native_to_orsf(const std::vector<uint8_t>& data) const {
    // Example: Parse from JSON bytes
    std::string json_str(data.begin(), data.end());
    return ORSF::from_json(json_str);
}

std::string ExampleAdapter::get_suggested_filename() const {
    return "setup_example.json";
}

std::string ExampleAdapter::get_file_extension() const {
    return "json";
}

std::optional<std::string> ExampleAdapter::get_install_path() const {
    return std::nullopt;
}

std::vector<FieldMapping> ExampleAdapter::get_field_mappings() const {
    // Example mappings (identity mappings for demonstration)
    return {
        FieldMapping("setup.aero.front_wing", "aero_front", std::nullopt, std::nullopt, false),
        FieldMapping("setup.aero.rear_wing", "aero_rear", std::nullopt, std::nullopt, false),
        FieldMapping("setup.tires.pressure_fl_kpa", "tire_fl_pressure",
            Transform::unit_convert(Unit::KPA, Unit::PSI),  // to native (PSI)
            Transform::unit_convert(Unit::PSI, Unit::KPA),  // to ORSF (kPa)
            false),
        FieldMapping("setup.brakes.brake_bias_pct", "brake_balance", std::nullopt, std::nullopt, false),
    };
}

} // namespace orsf
