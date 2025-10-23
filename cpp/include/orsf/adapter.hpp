#pragma once

#include "core.hpp"
#include "validator.hpp"
#include "mapping.hpp"
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <mutex>

namespace orsf {

// ============================================================================
// Adapter Interface
// ============================================================================

/// Base interface for game-specific adapters
class Adapter {
public:
    virtual ~Adapter() = default;

    /// Get game identifier (e.g., "iracing", "lfs", "acc")
    virtual std::string get_id() const = 0;

    /// Get game version supported by this adapter
    virtual std::string get_version() const = 0;

    /// Get car key (normalized car identifier)
    virtual std::string get_car_key() const = 0;

    /// Get suggested filename for exported setup
    virtual std::string get_suggested_filename() const = 0;

    /// Convert ORSF to native game format
    /// @param orsf ORSF setup to convert
    /// @return Binary data in native format
    /// @throws std::runtime_error if conversion fails
    virtual std::vector<uint8_t> orsf_to_native(const ORSF& orsf) const = 0;

    /// Convert native game format to ORSF
    /// @param data Binary data in native format
    /// @return ORSF setup
    /// @throws std::runtime_error if conversion fails
    virtual ORSF native_to_orsf(const std::vector<uint8_t>& data) const = 0;

    /// Validate ORSF for this specific game
    /// @param orsf ORSF setup to validate
    /// @return Vector of validation errors (empty if valid)
    virtual std::vector<ValidationError> validate_orsf(const ORSF& orsf) const = 0;

    /// Get file extension for native format (e.g., "sto", "set", "ini")
    virtual std::string get_file_extension() const = 0;

    /// Get game installation path (if applicable)
    /// @return Installation path or nullopt if not found
    virtual std::optional<std::string> get_install_path() const = 0;

    /// Get field mappings for this adapter
    virtual std::vector<FieldMapping> get_field_mappings() const = 0;

    /// Get adapter metadata
    struct Metadata {
        std::string id;
        std::string version;
        std::string car_key;
        std::string description;
        std::string author;
    };

    virtual Metadata get_metadata() const = 0;
};

// ============================================================================
// Adapter Registry
// ============================================================================

/// Thread-safe registry for game adapters
class AdapterRegistry {
public:
    /// Get singleton instance
    static AdapterRegistry& instance();

    /// Register an adapter
    /// @param adapter Shared pointer to adapter
    void register_adapter(std::shared_ptr<Adapter> adapter);

    /// Resolve adapter by game ID, version, and car key
    /// @param id Game identifier
    /// @param version Game version (empty for any version)
    /// @param car_key Car identifier (empty for any car)
    /// @return Shared pointer to adapter or nullptr if not found
    std::shared_ptr<Adapter> resolve(
        const std::string& id,
        const std::string& version = "",
        const std::string& car_key = ""
    ) const;

    /// Get all registered adapters
    std::vector<std::shared_ptr<Adapter>> get_all_adapters() const;

    /// Get all adapters for a specific game
    std::vector<std::shared_ptr<Adapter>> get_adapters_for_game(const std::string& id) const;

    /// Unregister adapter
    void unregister_adapter(const std::string& id, const std::string& version, const std::string& car_key);

    /// Clear all registered adapters
    void clear();

private:
    AdapterRegistry() = default;
    AdapterRegistry(const AdapterRegistry&) = delete;
    AdapterRegistry& operator=(const AdapterRegistry&) = delete;

    mutable std::mutex mutex_;
    std::vector<std::shared_ptr<Adapter>> adapters_;

    // Helper to create unique key for adapter
    static std::string make_key(const std::string& id, const std::string& version, const std::string& car_key);
};

// ============================================================================
// Base Adapter Implementation
// ============================================================================

/// Base class providing common adapter functionality
class BaseAdapter : public Adapter {
public:
    BaseAdapter(
        const std::string& id,
        const std::string& version,
        const std::string& car_key,
        const std::string& description = "",
        const std::string& author = ""
    );

    std::string get_id() const override { return metadata_.id; }
    std::string get_version() const override { return metadata_.version; }
    std::string get_car_key() const override { return metadata_.car_key; }
    Metadata get_metadata() const override { return metadata_; }

    /// Default validation uses standard ORSF validator
    std::vector<ValidationError> validate_orsf(const ORSF& orsf) const override;

protected:
    Metadata metadata_;

    /// Helper: Convert ORSF to flat key-value using field mappings
    FlatSetup orsf_to_flat(const ORSF& orsf) const;

    /// Helper: Convert flat key-value to ORSF using field mappings
    ORSF flat_to_orsf(const FlatSetup& flat, const ORSF& template_orsf) const;
};

// ============================================================================
// Example Adapter (for reference)
// ============================================================================

/// Example adapter demonstrating the interface
class ExampleAdapter : public BaseAdapter {
public:
    ExampleAdapter();

    std::vector<uint8_t> orsf_to_native(const ORSF& orsf) const override;
    ORSF native_to_orsf(const std::vector<uint8_t>& data) const override;
    std::string get_suggested_filename() const override;
    std::string get_file_extension() const override;
    std::optional<std::string> get_install_path() const override;
    std::vector<FieldMapping> get_field_mappings() const override;
};

} // namespace orsf
