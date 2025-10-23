#pragma once

/// ORSF - Open Racing Setup Format
/// A universal JSON-based car setup format for racing simulations
///
/// Version: 1.0.0
/// Schema: orsf://v1
/// License: MIT

// Core data structures
#include "core.hpp"

// Validation framework
#include "validator.hpp"

// Utilities (unit conversion, transformations)
#include "utils.hpp"

// Mapping engine
#include "mapping.hpp"

// Adapter system
#include "adapter.hpp"

/// Main ORSF namespace
namespace orsf {

/// Library version information
constexpr const char* VERSION = "1.0.0";
constexpr const char* SCHEMA_VERSION = "orsf://v1";

} // namespace orsf
