//
//  ORSFCore.swift
//  raceday
//
//  ORSF (Open Racing Setup Format) - Core Data Models
//

import Foundation

// MARK: - ORSF Core Data Structure

public struct ORSF: Codable {
    let schema: String
    var metadata: Metadata
    var car: Car
    var context: Context
    var setup: Setup
    var compat: [String: AnyCodable]?

    public struct Metadata: Codable {
        var id: String
        var name: String?
        var notes: String?
        var created_at: Date
        var updated_at: Date?
        var created_by: String
        var tags: [String]?
        var source: String?
        var origin_sim: String?
    }

    public struct Car: Codable {
        var make: String?
        var model: String?
        var variant: String?
        var `class`: String?
        var bop_id: String?
    }

    public struct Context: Codable {
        var track: String?
        var layout: String?
        var fuel_rule: String?
        var ambient_temp_c: Double?
        var track_temp_c: Double?
        var wetness: Double?
        var rubber: String?
        var session_type: String?
    }

    public struct Setup: Codable {
        var aero: [String: AnyCodable]?
        var suspension: [String: AnyCodable]?
        var tyres: [String: AnyCodable]?
        var drivetrain: [String: AnyCodable]?
        var gearing: [String: AnyCodable]?
        var brakes: [String: AnyCodable]?
        var electronics: [String: AnyCodable]?
        var fuel: [String: AnyCodable]?
        var strategy: [String: AnyCodable]?
    }
}

// MARK: - AnyCodable Wrapper

public struct AnyCodable: Codable {
    public let value: Any

    public init(_ value: Any) {
        self.value = value
    }

    public init(from decoder: Decoder) throws {
        let container = try decoder.singleValueContainer()

        if let value = try? container.decode(Double.self) {
            self.value = value
            return
        }
        if let value = try? container.decode(Int.self) {
            self.value = value
            return
        }
        if let value = try? container.decode(String.self) {
            self.value = value
            return
        }
        if let value = try? container.decode(Bool.self) {
            self.value = value
            return
        }
        if let value = try? container.decode([String: AnyCodable].self) {
            self.value = value
            return
        }
        if let value = try? container.decode([AnyCodable].self) {
            self.value = value
            return
        }

        throw DecodingError.typeMismatch(
            AnyCodable.self,
            DecodingError.Context(
                codingPath: decoder.codingPath,
                debugDescription: "Unsupported type for AnyCodable"
            )
        )
    }

    public func encode(to encoder: Encoder) throws {
        var container = encoder.singleValueContainer()

        switch value {
        case let v as Double:
            try container.encode(v)
        case let v as Int:
            try container.encode(v)
        case let v as String:
            try container.encode(v)
        case let v as Bool:
            try container.encode(v)
        case let v as [String: AnyCodable]:
            try container.encode(v)
        case let v as [AnyCodable]:
            try container.encode(v)
        default:
            throw EncodingError.invalidValue(
                value,
                EncodingError.Context(
                    codingPath: encoder.codingPath,
                    debugDescription: "Unsupported type for AnyCodable: \(type(of: value))"
                )
            )
        }
    }
}

// MARK: - Helper Extensions

extension AnyCodable {
    public var doubleValue: Double? {
        switch value {
        case let d as Double: return d
        case let i as Int: return Double(i)
        case let f as Float: return Double(f)
        default: return nil
        }
    }

    public var intValue: Int? {
        switch value {
        case let i as Int: return i
        case let d as Double: return Int(d)
        case let f as Float: return Int(f)
        default: return nil
        }
    }

    public var stringValue: String? {
        switch value {
        case let s as String: return s
        default: return nil
        }
    }

    public var boolValue: Bool? {
        switch value {
        case let b as Bool: return b
        default: return nil
        }
    }
}

// MARK: - ORSF Factory

public struct ORSFFactory {
    static func createFromCarSetup(_ carSetup: CarSetup) -> ORSF {
        return ORSF(
            schema: "orsf://v1",
            metadata: ORSF.Metadata(
                id: carSetup.id.uuidString,
                name: carSetup.name,
                notes: nil,
                created_at: carSetup.dateCreated,
                updated_at: carSetup.lastModified,
                created_by: carSetup.teamId,
                tags: carSetup.tags,
                source: "manual",
                origin_sim: "raceday"
            ),
            car: ORSF.Car(
                make: carSetup.car?.manufacturer ?? carSetup.car?.name,
                model: carSetup.car?.name,
                variant: carSetup.car?.year,
                class: carSetup.car?.category?.name,
                bop_id: carSetup.car?.code
            ),
            context: ORSF.Context(
                track: carSetup.track,
                layout: nil,
                fuel_rule: nil,
                ambient_temp_c: nil,
                track_temp_c: nil,
                wetness: carSetup.weatherCondition == .heavyRain ? 1.0 : (carSetup.weatherCondition == .lightRain ? 0.5 : 0.0),
                rubber: "medium",
                session_type: carSetup.setupType == .qualifying ? "qualifying" : "race"
            ),
            setup: convertCarSetupToORSF(carSetup),
            compat: [
                "raceday": AnyCodable([
                    "version": AnyCodable(carSetup.currentVersion),
                    "setupType": AnyCodable(carSetup.setupType.rawValue),
                    "weatherCondition": AnyCodable(carSetup.weatherCondition.rawValue)
                ])
            ]
        )
    }

    private static func convertCarSetupToORSF(_ carSetup: CarSetup) -> ORSF.Setup {
        var setup = ORSF.Setup()

        // Convert suspension data
        let suspension = carSetup.suspension
        setup.suspension = [
            "corner": AnyCodable([
                "fl": AnyCodable([
                    "camber_deg": AnyCodable(suspension.frontCamberLeft),
                    "toe_deg": AnyCodable(suspension.frontToeLeft),
                    "spring_n_per_mm": AnyCodable(suspension.frontSpringRate),
                    "ride_height_mm": AnyCodable(suspension.frontRideHeight),
                    "damper_compression": AnyCodable(suspension.frontDamperCompression),
                    "damper_rebound": AnyCodable(suspension.frontDamperRebound),
                    "caster_deg": AnyCodable(suspension.frontCaster)
                ]),
                "fr": AnyCodable([
                    "camber_deg": AnyCodable(suspension.frontCamberRight),
                    "toe_deg": AnyCodable(suspension.frontToeRight),
                    "spring_n_per_mm": AnyCodable(suspension.frontSpringRate),
                    "ride_height_mm": AnyCodable(suspension.frontRideHeight),
                    "damper_compression": AnyCodable(suspension.frontDamperCompression),
                    "damper_rebound": AnyCodable(suspension.frontDamperRebound),
                    "caster_deg": AnyCodable(suspension.frontCaster)
                ]),
                "rl": AnyCodable([
                    "camber_deg": AnyCodable(suspension.rearCamberLeft),
                    "toe_deg": AnyCodable(0.0), // LFS doesn't have rear toe
                    "spring_n_per_mm": AnyCodable(suspension.rearSpringRate),
                    "ride_height_mm": AnyCodable(suspension.rearRideHeight),
                    "damper_compression": AnyCodable(suspension.rearDamperCompression),
                    "damper_rebound": AnyCodable(suspension.rearDamperRebound)
                ]),
                "rr": AnyCodable([
                    "camber_deg": AnyCodable(suspension.rearCamberRight),
                    "toe_deg": AnyCodable(0.0), // LFS doesn't have rear toe
                    "spring_n_per_mm": AnyCodable(suspension.rearSpringRate),
                    "ride_height_mm": AnyCodable(suspension.rearRideHeight),
                    "damper_compression": AnyCodable(suspension.rearDamperCompression),
                    "damper_rebound": AnyCodable(suspension.rearDamperRebound)
                ])
            ]),
            "anti_roll_bar": AnyCodable([
                "front": AnyCodable(suspension.frontAntiRollBar),
                "rear": AnyCodable(suspension.rearAntiRollBar)
            ])
        ]

        // Convert tire/tyre data
        let tires = carSetup.tires
        setup.tyres = [
            "compound": AnyCodable([
                "front": AnyCodable(tires.frontCompound.displayName),
                "rear": AnyCodable(tires.rearCompound.displayName)
            ]),
            "pressure_kpa": AnyCodable([
                "fl": AnyCodable(tires.frontPressureLeft),
                "fr": AnyCodable(tires.frontPressureRight),
                "rl": AnyCodable(tires.rearPressureLeft),
                "rr": AnyCodable(tires.rearPressureRight)
            ])
        ]

        // Convert drivetrain data
        let drivetrain = carSetup.drivetrain

        // Build gearing dictionary, handling optional sixth gear
        var gearingDict: [String: AnyCodable] = [
            "gear_1": AnyCodable(drivetrain.firstGear),
            "gear_2": AnyCodable(drivetrain.secondGear),
            "gear_3": AnyCodable(drivetrain.thirdGear),
            "gear_4": AnyCodable(drivetrain.fourthGear),
            "gear_5": AnyCodable(drivetrain.fifthGear),
            "final_drive": AnyCodable(drivetrain.finalDriveRatio)
        ]

        // Only add sixth gear if it exists (not nil)
        if let sixthGear = drivetrain.sixthGear {
            gearingDict["gear_6"] = AnyCodable(sixthGear)
        }

        setup.drivetrain = [
            "gearing": AnyCodable(gearingDict),
            "differential": AnyCodable([
                "preload": AnyCodable(drivetrain.differentialPreload),
                "power_locking": AnyCodable(drivetrain.differentialPowerLocking),
                "coast_locking": AnyCodable(drivetrain.differentialCoastLocking)
            ])
        ]

        // Convert brake data
        setup.brakes = [
            "balance": AnyCodable(drivetrain.brakeBalance),
            "max_force": AnyCodable(drivetrain.maximumBrakeForce)
        ]

        // Convert aerodynamics if available
        if carSetup.car?.hasAerodynamics == true {
            let aero = carSetup.aerodynamics
            setup.aero = [
                "front_wing": AnyCodable(aero.frontSplitter),
                "rear_wing": AnyCodable(aero.rearWing),
                "brake_ducts": AnyCodable([
                    "front": AnyCodable(aero.frontBrakeDucts),
                    "rear": AnyCodable(aero.rearBrakeDucts)
                ])
            ]
        }

        // Convert electronics if available
        if carSetup.car?.hasAdvancedElectronics == true {
            let electronics = carSetup.electronics
            setup.electronics = [
                "traction_control": AnyCodable([
                    "tc1": AnyCodable(electronics.tractionControl1),
                    "tc2": AnyCodable(electronics.tractionControl2)
                ]),
                "abs": AnyCodable(electronics.abs),
                "engine_braking": AnyCodable(electronics.engineBraking)
            ]
        }

        // Convert engine data if available
        if carSetup.car?.hasTurbo == true {
            let engine = carSetup.engine
            setup.fuel = [
                "ecu_map": AnyCodable(engine.ecuMap),
                "fuel_mixture": AnyCodable(engine.fuelMixture)
            ]
        }

        return setup
    }

}