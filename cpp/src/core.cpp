#include "orsf/core.hpp"
#include <stdexcept>

namespace orsf {

ORSF ORSF::from_json(const std::string& json_str) {
    try {
        json j = json::parse(json_str);
        return from_json(j);
    } catch (const json::exception& e) {
        throw std::runtime_error(std::string("Failed to parse JSON: ") + e.what());
    }
}

ORSF ORSF::from_json(const json& j) {
    try {
        ORSF orsf;
        j.get_to(orsf);

        // Validate schema version
        if (orsf.schema != "orsf://v1") {
            throw std::runtime_error("Invalid schema version: " + orsf.schema + " (expected orsf://v1)");
        }

        return orsf;
    } catch (const json::exception& e) {
        throw std::runtime_error(std::string("Failed to deserialize ORSF: ") + e.what());
    }
}

std::string ORSF::to_json_string(int indent) const {
    try {
        json j = to_json();
        return j.dump(indent);
    } catch (const json::exception& e) {
        throw std::runtime_error(std::string("Failed to serialize ORSF: ") + e.what());
    }
}

json ORSF::to_json() const {
    try {
        json j;
        nlohmann::to_json(j, *this);
        return j;
    } catch (const json::exception& e) {
        throw std::runtime_error(std::string("Failed to convert ORSF to JSON: ") + e.what());
    }
}

} // namespace orsf
