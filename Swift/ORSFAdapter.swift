//
//  ORSFAdapter.swift
//  raceday
//
//  ORSF Adapter Protocol and Registry System
//

import Foundation

// MARK: - Adapter Protocol

public protocol ORSFAdapter {
    var id: String { get }                  // e.g., "iracing", "lfs"
    var version: String { get }             // game build version
    var carKey: String { get }              // normalized per car
    var suggestedFilename: String { get }   // for export dialogs

    func orsfToNative(_ orsf: ORSF) throws -> Data
    func nativeToORSF(_ data: Data) throws -> ORSF
    func validateORSF(_ orsf: ORSF) -> [ORSFValidationError]
    func getFileExtension() -> String
    func getInstallPath() -> String?
}

// MARK: - Adapter Errors

public enum ORSFAdapterError: Error, LocalizedError {
    case mappingNotFound(String)
    case unsupportedField(String)
    case parseFailure(String)
    case validation(String)
    case unsupportedGameVersion(String)
    case fileFormatError(String)

    public var errorDescription: String? {
        switch self {
        case .mappingNotFound(let field):
            return "Mapping not found for field: \(field)"
        case .unsupportedField(let field):
            return "Unsupported field: \(field)"
        case .parseFailure(let reason):
            return "Parse failure: \(reason)"
        case .validation(let reason):
            return "Validation error: \(reason)"
        case .unsupportedGameVersion(let version):
            return "Unsupported game version: \(version)"
        case .fileFormatError(let reason):
            return "File format error: \(reason)"
        }
    }
}

public struct ORSFValidationError {
    let field: String
    let message: String
    let severity: Severity
    let code: ValidationCode?
    let actualValue: Any?
    let expectedRange: String?

    enum Severity {
        case error, warning, info
    }

    enum ValidationCode {
        case required, outOfRange, invalidFormat, incompatible, deprecated
    }
}

// MARK: - Adapter Registry

public final class ORSFAdapterRegistry {
    public static let shared = ORSFAdapterRegistry()

    private var adapters: [String: ORSFAdapter] = [:]
    private let queue = DispatchQueue(label: "orsf.adapter.registry", attributes: .concurrent)

    private init() {}

    private func key(for adapter: ORSFAdapter) -> String {
        return "\(adapter.id):\(adapter.version):\(adapter.carKey)"
    }

    public func register(_ adapter: ORSFAdapter) {
        queue.async(flags: .barrier) {
            self.adapters[self.key(for: adapter)] = adapter
        }
    }

    public func resolve(id: String, version: String, carKey: String) -> ORSFAdapter? {
        return queue.sync {
            let key = "\(id):\(version):\(carKey)"
            return adapters[key]
        }
    }

    public func getAllAdapters() -> [ORSFAdapter] {
        return queue.sync {
            Array(adapters.values)
        }
    }

    public func getAdapters(for gameId: String) -> [ORSFAdapter] {
        return queue.sync {
            adapters.values.filter { $0.id == gameId }
        }
    }
}

// MARK: - Field Mapping System

public struct FieldMapping: Codable {
    public struct Field: Codable {
        let orsf: String
        let nativeKey: String  // "sto" for iRacing, "set" for LFS
        let transform: String?
        let from: String?
        let to: String?
        let scale: Double?
        let codomain: Codomain?
        let lut: [LUTPoint]?

        public struct Codomain: Codable {
            let min: Double?
            let max: Double?
            let step: Double?
        }

        public struct LUTPoint: Codable {
            let x: Double
            let y: Double
        }
    }

    public let adapter: String
    public let version: String
    public let carKey: String
    public let filenameTemplates: FilenameTemplates?
    public let installPaths: InstallPaths?
    public let fields: [Field]
    public let capabilities: [String: Bool]?

    public struct FilenameTemplates: Codable {
        let export: String      // "{CAR3}_{TRACK}_{NAME}.set"
        let importGlob: String  // "{CAR3}_*.set"

        enum CodingKeys: String, CodingKey {
            case export
            case importGlob = "import_glob"
        }
    }

    public struct InstallPaths: Codable {
        let windows: String?
        let macos: String?
        let linux: String?
    }
}

// MARK: - Mapping Engine

public class MappingEngine {
    private let mapping: FieldMapping

    public init(mapping: FieldMapping) {
        self.mapping = mapping
    }

    public func flattenORSF(_ orsf: ORSF) throws -> [String: Double] {
        var flatValues: [String: Double] = [:]

        for field in mapping.fields {
            guard let value = try lookupDouble(in: orsf, path: field.orsf) else {
                continue
            }

            var transformedValue = value

            // Apply transformations
            if let transform = field.transform {
                switch transform {
                case "unit":
                    if let from = field.from, let to = field.to {
                        transformedValue = UnitConverter.convert(value, from: from, to: to)
                    }
                case "scale":
                    if let scale = field.scale {
                        transformedValue = value * scale
                    }
                case "lut":
                    if let lut = field.lut {
                        transformedValue = LookupTableConverter.interpolate(value: value, lut: lut)
                    }
                default:
                    break
                }
            }

            // Apply codomain constraints
            if let codomain = field.codomain {
                transformedValue = UnitConverter.clamp(
                    transformedValue,
                    min: codomain.min ?? transformedValue,
                    max: codomain.max ?? transformedValue,
                    step: codomain.step
                )
            }

            flatValues[field.nativeKey] = transformedValue
        }

        return flatValues
    }

    public func inflateORSF(from flat: [String: Double], template: ORSF) throws -> ORSF {
        var orsf = template

        for field in mapping.fields {
            guard let value = flat[field.nativeKey] else { continue }

            var transformedValue = value

            // Apply inverse transformations
            if let transform = field.transform {
                switch transform {
                case "unit":
                    if let from = field.from, let to = field.to {
                        // Reverse the unit conversion
                        transformedValue = UnitConverter.convert(value, from: to, to: from)
                    }
                case "scale":
                    if let scale = field.scale, scale != 0 {
                        transformedValue = value / scale
                    }
                case "lut":
                    if let lut = field.lut {
                        // Reverse lookup in LUT
                        transformedValue = LookupTableConverter.reverseLookup(value: value, lut: lut)
                    }
                default:
                    break
                }
            }

            // Set value back into ORSF structure
            try setValue(in: &orsf, path: field.orsf, value: transformedValue)
        }

        return orsf
    }

    // MARK: - Helper Methods

    private func lookupDouble(in orsf: ORSF, path: String) throws -> Double? {
        let components = path.split(separator: ".").map(String.init)

        guard components.count >= 2 else {
            throw ORSFAdapterError.parseFailure("Invalid path format: \(path)")
        }

        let section = components[0]
        let remainingPath = components.dropFirst()

        guard let sectionData = getSectionData(from: orsf.setup, section: section) else {
            return nil
        }

        return extractValue(from: sectionData, path: Array(remainingPath))?.doubleValue
    }

    private func getSectionData(from setup: ORSF.Setup, section: String) -> [String: AnyCodable]? {
        switch section {
        case "aero": return setup.aero
        case "suspension": return setup.suspension
        case "tyres": return setup.tyres
        case "drivetrain": return setup.drivetrain
        case "gearing": return setup.gearing
        case "brakes": return setup.brakes
        case "electronics": return setup.electronics
        case "fuel": return setup.fuel
        case "strategy": return setup.strategy
        default: return nil
        }
    }

    private func extractValue(from data: [String: AnyCodable], path: [String]) -> AnyCodable? {
        guard !path.isEmpty else { return nil }

        let key = path[0]
        guard let value = data[key] else { return nil }

        if path.count == 1 {
            return value
        }

        // Navigate deeper into nested structure
        if let nestedDict = value.value as? [String: AnyCodable] {
            return extractValue(from: nestedDict, path: Array(path.dropFirst()))
        }

        return nil
    }

    private func setValue(in orsf: inout ORSF, path: String, value: Double) throws {
        let components = path.split(separator: ".").map(String.init)

        guard components.count >= 2 else {
            throw ORSFAdapterError.parseFailure("Invalid path format: \(path)")
        }

        let section = components[0]
        let remainingPath = Array(components.dropFirst())

        // This is a simplified implementation - in practice, you'd need
        // more sophisticated path setting for nested structures
        switch section {
        case "tyres":
            if orsf.setup.tyres == nil {
                orsf.setup.tyres = [:]
            }
            setNestedValue(in: &orsf.setup.tyres!, path: remainingPath, value: AnyCodable(value))
        case "suspension":
            if orsf.setup.suspension == nil {
                orsf.setup.suspension = [:]
            }
            setNestedValue(in: &orsf.setup.suspension!, path: remainingPath, value: AnyCodable(value))
        case "drivetrain":
            if orsf.setup.drivetrain == nil {
                orsf.setup.drivetrain = [:]
            }
            setNestedValue(in: &orsf.setup.drivetrain!, path: remainingPath, value: AnyCodable(value))
        default:
            break
        }
    }

    private func setNestedValue(in dict: inout [String: AnyCodable], path: [String], value: AnyCodable) {
        guard !path.isEmpty else { return }

        if path.count == 1 {
            dict[path[0]] = value
            return
        }

        let key = path[0]
        let remainingPath = Array(path.dropFirst())

        if var nestedDict = dict[key]?.value as? [String: AnyCodable] {
            setNestedValue(in: &nestedDict, path: remainingPath, value: value)
            dict[key] = AnyCodable(nestedDict)
        } else {
            // Create nested structure if it doesn't exist
            var newDict: [String: AnyCodable] = [:]
            setNestedValue(in: &newDict, path: remainingPath, value: value)
            dict[key] = AnyCodable(newDict)
        }
    }
}

// MARK: - Unit Conversion

public struct UnitConverter {
    public static func convert(_ value: Double, from: String, to: String) -> Double {
        switch (from.lowercased(), to.lowercased()) {
        // Pressure conversions
        case ("kpa", "psi"):
            return value * 0.1450377377
        case ("psi", "kpa"):
            return value / 0.1450377377
        case ("kpa", "bar"):
            return value / 100.0
        case ("bar", "kpa"):
            return value * 100.0
        case ("psi", "bar"):
            return value * 0.0689475729
        case ("bar", "psi"):
            return value / 0.0689475729

        // Spring rate conversions
        case ("n/mm", "lb/in"):
            return value * 5.71014715
        case ("lb/in", "n/mm"):
            return value / 5.71014715

        // Distance conversions
        case ("mm", "in"):
            return value / 25.4
        case ("in", "mm"):
            return value * 25.4

        // Temperature conversions
        case ("c", "f"):
            return value * 9.0/5.0 + 32.0
        case ("f", "c"):
            return (value - 32.0) * 5.0/9.0

        // Same unit or unknown - pass through
        default:
            return value
        }
    }

    public static func clamp(_ value: Double, min: Double, max: Double, step: Double?) -> Double {
        let clamped = Swift.max(min, Swift.min(max, value))

        guard let step = step, step > 0 else {
            return clamped
        }

        return (clamped / step).rounded() * step
    }
}

// MARK: - Lookup Table Converter

public struct LookupTableConverter {
    public static func interpolate(value: Double, lut: [FieldMapping.Field.LUTPoint]) -> Double {
        // Handle edge cases
        guard !lut.isEmpty else { return value }

        let sortedLUT = lut.sorted { $0.x < $1.x }

        // Value before first point
        if value <= sortedLUT.first!.x {
            return sortedLUT.first!.y
        }

        // Value after last point
        if value >= sortedLUT.last!.x {
            return sortedLUT.last!.y
        }

        // Find surrounding points for interpolation
        for i in 0..<(sortedLUT.count - 1) {
            let p1 = sortedLUT[i]
            let p2 = sortedLUT[i + 1]

            if value >= p1.x && value <= p2.x {
                // Linear interpolation
                let ratio = (value - p1.x) / (p2.x - p1.x)
                return p1.y + ratio * (p2.y - p1.y)
            }
        }

        return value
    }

    public static func reverseLookup(value: Double, lut: [FieldMapping.Field.LUTPoint]) -> Double {
        // Reverse lookup - find x for given y
        guard !lut.isEmpty else { return value }

        let sortedByY = lut.sorted { $0.y < $1.y }

        // Value before first point
        if value <= sortedByY.first!.y {
            return sortedByY.first!.x
        }

        // Value after last point
        if value >= sortedByY.last!.y {
            return sortedByY.last!.x
        }

        // Find surrounding points for interpolation
        for i in 0..<(sortedByY.count - 1) {
            let p1 = sortedByY[i]
            let p2 = sortedByY[i + 1]

            if value >= p1.y && value <= p2.y {
                // Linear interpolation on reversed axis
                let ratio = (value - p1.y) / (p2.y - p1.y)
                return p1.x + ratio * (p2.x - p1.x)
            }
        }

        return value
    }
}

// MARK: - ORSF Validation System

public class ORSFValidator {
    private let schema: ORSFSchemaValidation

    public init() {
        self.schema = ORSFSchemaValidation()
    }

    public func validateORSF(_ orsf: ORSF) -> [ORSFValidationError] {
        var errors: [ORSFValidationError] = []

        // Schema validation
        errors.append(contentsOf: schema.validateSchema(orsf))

        // Metadata validation
        errors.append(contentsOf: validateMetadata(orsf.metadata))

        // Car validation
        errors.append(contentsOf: validateCar(orsf.car))

        // Context validation
        errors.append(contentsOf: validateContext(orsf.context))

        // Setup validation
        errors.append(contentsOf: validateSetup(orsf.setup))

        // Cross-field validation
        errors.append(contentsOf: validateCrossFields(orsf))

        return errors
    }

    private func validateMetadata(_ metadata: ORSF.Metadata) -> [ORSFValidationError] {
        var errors: [ORSFValidationError] = []

        // ID validation
        if metadata.id.isEmpty {
            errors.append(ORSFValidationError(
                field: "metadata.id",
                message: "ID cannot be empty",
                severity: .error,
                code: .required,
                actualValue: metadata.id,
                expectedRange: "Non-empty string"
            ))
        }

        // Created by validation
        if metadata.created_by.isEmpty {
            errors.append(ORSFValidationError(
                field: "metadata.created_by",
                message: "Created by cannot be empty",
                severity: .error,
                code: .required,
                actualValue: metadata.created_by,
                expectedRange: "Non-empty string"
            ))
        }

        // Date validation
        if let updatedAt = metadata.updated_at, updatedAt < metadata.created_at {
            errors.append(ORSFValidationError(
                field: "metadata.updated_at",
                message: "Updated date cannot be before created date",
                severity: .error,
                code: .invalidFormat,
                actualValue: updatedAt,
                expectedRange: "Date >= created_at"
            ))
        }

        return errors
    }

    private func validateCar(_ car: ORSF.Car) -> [ORSFValidationError] {
        let errors: [ORSFValidationError] = []

        // Basic required fields should be handled by schema
        // Add specific car validation logic here

        return errors
    }

    private func validateContext(_ context: ORSF.Context) -> [ORSFValidationError] {
        var errors: [ORSFValidationError] = []

        // Temperature validation
        if let ambientTemp = context.ambient_temp_c {
            if ambientTemp < -50 || ambientTemp > 70 {
                errors.append(ORSFValidationError(
                    field: "context.ambient_temp_c",
                    message: "Ambient temperature outside realistic range",
                    severity: .warning,
                    code: .outOfRange,
                    actualValue: ambientTemp,
                    expectedRange: "-50°C to 70°C"
                ))
            }
        }

        if let trackTemp = context.track_temp_c {
            if trackTemp < -20 || trackTemp > 80 {
                errors.append(ORSFValidationError(
                    field: "context.track_temp_c",
                    message: "Track temperature outside realistic range",
                    severity: .warning,
                    code: .outOfRange,
                    actualValue: trackTemp,
                    expectedRange: "-20°C to 80°C"
                ))
            }
        }

        // Wetness validation
        if let wetness = context.wetness {
            if wetness < 0 || wetness > 1 {
                errors.append(ORSFValidationError(
                    field: "context.wetness",
                    message: "Wetness must be between 0 and 1",
                    severity: .error,
                    code: .outOfRange,
                    actualValue: wetness,
                    expectedRange: "0.0 to 1.0"
                ))
            }
        }

        return errors
    }

    private func validateSetup(_ setup: ORSF.Setup) -> [ORSFValidationError] {
        var errors: [ORSFValidationError] = []

        // Suspension validation
        if let suspension = setup.suspension {
            errors.append(contentsOf: validateSuspension(suspension))
        }

        // Tyre validation
        if let tyres = setup.tyres {
            errors.append(contentsOf: validateTyres(tyres))
        }

        // Drivetrain validation
        if let drivetrain = setup.drivetrain {
            errors.append(contentsOf: validateDrivetrain(drivetrain))
        }

        // Brake validation
        if let brakes = setup.brakes {
            errors.append(contentsOf: validateBrakes(brakes))
        }

        return errors
    }

    private func validateSuspension(_ suspension: [String: AnyCodable]) -> [ORSFValidationError] {
        var errors: [ORSFValidationError] = []

        // Validate corner setups
        if let cornerData = suspension["corner"]?.value as? [String: AnyCodable] {
            for (corner, data) in cornerData {
                if let cornerSetup = data.value as? [String: AnyCodable] {
                    errors.append(contentsOf: validateCornerSetup(cornerSetup, corner: corner))
                }
            }
        }

        // Validate anti-roll bars
        if let frontARB = suspension["arb_front_n_per_deg"]?.doubleValue {
            if frontARB < 0 {
                errors.append(ORSFValidationError(
                    field: "suspension.arb_front_n_per_deg",
                    message: "Front anti-roll bar rate cannot be negative",
                    severity: .error,
                    code: .outOfRange,
                    actualValue: frontARB,
                    expectedRange: ">= 0"
                ))
            }
        }

        return errors
    }

    private func validateCornerSetup(_ setup: [String: AnyCodable], corner: String) -> [ORSFValidationError] {
        var errors: [ORSFValidationError] = []

        let fieldPrefix = "suspension.corner.\(corner)"

        // Camber validation
        if let camber = setup["camber_deg"]?.doubleValue {
            if camber < -10 || camber > 5 {
                errors.append(ORSFValidationError(
                    field: "\(fieldPrefix).camber_deg",
                    message: "Camber angle outside typical range",
                    severity: .warning,
                    code: .outOfRange,
                    actualValue: camber,
                    expectedRange: "-10° to 5°"
                ))
            }
        }

        // Spring rate validation
        if let springRate = setup["spring_n_per_mm"]?.doubleValue {
            if springRate <= 0 {
                errors.append(ORSFValidationError(
                    field: "\(fieldPrefix).spring_n_per_mm",
                    message: "Spring rate must be positive",
                    severity: .error,
                    code: .outOfRange,
                    actualValue: springRate,
                    expectedRange: "> 0"
                ))
            }
        }

        return errors
    }

    private func validateTyres(_ tyres: [String: AnyCodable]) -> [ORSFValidationError] {
        var errors: [ORSFValidationError] = []

        // Pressure validation
        if let pressures = tyres["pressure_kpa"]?.value as? [String: AnyCodable] {
            for (wheel, pressure) in pressures {
                if let pressureValue = pressure.doubleValue {
                    if pressureValue < 50 || pressureValue > 400 {
                        errors.append(ORSFValidationError(
                            field: "tyres.pressure_kpa.\(wheel)",
                            message: "Tyre pressure outside realistic range",
                            severity: .warning,
                            code: .outOfRange,
                            actualValue: pressureValue,
                            expectedRange: "50-400 kPa"
                        ))
                    }
                }
            }
        }

        return errors
    }

    private func validateDrivetrain(_ drivetrain: [String: AnyCodable]) -> [ORSFValidationError] {
        var errors: [ORSFValidationError] = []

        // Differential validation
        if let preload = drivetrain["diff_preload_n_m"]?.doubleValue {
            if preload < 0 {
                errors.append(ORSFValidationError(
                    field: "drivetrain.diff_preload_n_m",
                    message: "Differential preload cannot be negative",
                    severity: .error,
                    code: .outOfRange,
                    actualValue: preload,
                    expectedRange: ">= 0"
                ))
            }
        }

        // Percentage validation
        if let powerRamp = drivetrain["power_ramp_pct"]?.doubleValue {
            if powerRamp < 0 || powerRamp > 100 {
                errors.append(ORSFValidationError(
                    field: "drivetrain.power_ramp_pct",
                    message: "Power ramp percentage must be 0-100",
                    severity: .error,
                    code: .outOfRange,
                    actualValue: powerRamp,
                    expectedRange: "0-100%"
                ))
            }
        }

        return errors
    }

    private func validateBrakes(_ brakes: [String: AnyCodable]) -> [ORSFValidationError] {
        var errors: [ORSFValidationError] = []

        // Brake bias validation
        if let brakeBias = brakes["brake_bias_pct"]?.doubleValue {
            if brakeBias < 0 || brakeBias > 100 {
                errors.append(ORSFValidationError(
                    field: "brakes.brake_bias_pct",
                    message: "Brake bias percentage must be 0-100",
                    severity: .error,
                    code: .outOfRange,
                    actualValue: brakeBias,
                    expectedRange: "0-100%"
                ))
            }
        }

        return errors
    }

    private func validateCrossFields(_ orsf: ORSF) -> [ORSFValidationError] {
        var errors: [ORSFValidationError] = []

        // Check temperature consistency
        if let ambientTemp = orsf.context.ambient_temp_c,
           let trackTemp = orsf.context.track_temp_c {
            if trackTemp < ambientTemp - 10 {
                errors.append(ORSFValidationError(
                    field: "context.track_temp_c",
                    message: "Track temperature significantly lower than ambient",
                    severity: .warning,
                    code: .incompatible,
                    actualValue: trackTemp,
                    expectedRange: "Within 10°C of ambient"
                ))
            }
        }

        return errors
    }
}

// MARK: - Schema Validation

public class ORSFSchemaValidation {

    public func validateSchema(_ orsf: ORSF) -> [ORSFValidationError] {
        var errors: [ORSFValidationError] = []

        // Schema version validation
        if orsf.schema != "orsf://v1" {
            errors.append(ORSFValidationError(
                field: "schema",
                message: "Unsupported schema version",
                severity: .error,
                code: .invalidFormat,
                actualValue: orsf.schema,
                expectedRange: "orsf://v1"
            ))
        }

        // Required field validation would go here
        // This is a simplified version - in practice you'd validate against the full JSON schema

        return errors
    }
}