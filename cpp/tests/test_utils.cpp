#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "orsf/utils.hpp"

using namespace orsf;
using Catch::Approx;

TEST_CASE("UnitConverter converts pressure correctly", "[utils]") {
    SECTION("kPa to PSI") {
        double kpa = 200.0;
        double psi = UnitConverter::convert(kpa, Unit::KPA, Unit::PSI);
        REQUIRE(psi == Approx(29.0076).margin(0.001));
    }

    SECTION("PSI to kPa") {
        double psi = 30.0;
        double kpa = UnitConverter::convert(psi, Unit::PSI, Unit::KPA);
        REQUIRE(kpa == Approx(206.843).margin(0.001));
    }

    SECTION("kPa to Bar") {
        double kpa = 200.0;
        double bar = UnitConverter::convert(kpa, Unit::KPA, Unit::BAR);
        REQUIRE(bar == Approx(2.0).margin(0.001));
    }
}

TEST_CASE("UnitConverter converts length correctly", "[utils]") {
    SECTION("mm to inches") {
        double mm = 25.4;
        double inches = UnitConverter::convert(mm, Unit::MM, Unit::INCHES);
        REQUIRE(inches == Approx(1.0).margin(0.001));
    }

    SECTION("inches to mm") {
        double inches = 2.0;
        double mm = UnitConverter::convert(inches, Unit::INCHES, Unit::MM);
        REQUIRE(mm == Approx(50.8).margin(0.001));
    }
}

TEST_CASE("UnitConverter converts temperature correctly", "[utils]") {
    SECTION("Celsius to Fahrenheit") {
        double celsius = 20.0;
        double fahrenheit = UnitConverter::convert(celsius, Unit::CELSIUS, Unit::FAHRENHEIT);
        REQUIRE(fahrenheit == Approx(68.0).margin(0.001));
    }

    SECTION("Fahrenheit to Celsius") {
        double fahrenheit = 32.0;
        double celsius = UnitConverter::convert(fahrenheit, Unit::FAHRENHEIT, Unit::CELSIUS);
        REQUIRE(celsius == Approx(0.0).margin(0.001));
    }

    SECTION("Celsius to Kelvin") {
        double celsius = 0.0;
        double kelvin = UnitConverter::convert(celsius, Unit::CELSIUS, Unit::KELVIN);
        REQUIRE(kelvin == Approx(273.15).margin(0.001));
    }
}

TEST_CASE("UnitConverter converts spring rate correctly", "[utils]") {
    SECTION("N/mm to lb/in") {
        double n_mm = 100.0;
        double lb_in = UnitConverter::convert(n_mm, Unit::N_MM, Unit::LB_IN);
        REQUIRE(lb_in == Approx(571.015).margin(0.01));
    }
}

TEST_CASE("UnitConverter clamps values correctly", "[utils]") {
    SECTION("Clamp value within range") {
        double result = UnitConverter::clamp(50.0, 0.0, 100.0);
        REQUIRE(result == 50.0);
    }

    SECTION("Clamp value below minimum") {
        double result = UnitConverter::clamp(-10.0, 0.0, 100.0);
        REQUIRE(result == 0.0);
    }

    SECTION("Clamp value above maximum") {
        double result = UnitConverter::clamp(150.0, 0.0, 100.0);
        REQUIRE(result == 100.0);
    }

    SECTION("Clamp with step precision") {
        double result = UnitConverter::clamp(52.3, 0.0, 100.0, 5.0);
        REQUIRE(result == Approx(50.0).margin(0.001));
    }
}

TEST_CASE("UnitConverter rounds to step correctly", "[utils]") {
    REQUIRE(UnitConverter::round_to_step(52.3, 5.0) == Approx(50.0).margin(0.001));
    REQUIRE(UnitConverter::round_to_step(53.0, 5.0) == Approx(55.0).margin(0.001));
    REQUIRE(UnitConverter::round_to_step(14.7, 0.5) == Approx(14.5).margin(0.001));
}

TEST_CASE("LookupTableConverter interpolates correctly", "[utils]") {
    std::vector<LUTEntry> table = {
        {0.0, 0.0},
        {50.0, 25.0},
        {100.0, 75.0}
    };

    LookupTableConverter lut(table);

    SECTION("Interpolate at exact points") {
        REQUIRE(lut.interpolate(0.0) == Approx(0.0).margin(0.001));
        REQUIRE(lut.interpolate(50.0) == Approx(25.0).margin(0.001));
        REQUIRE(lut.interpolate(100.0) == Approx(75.0).margin(0.001));
    }

    SECTION("Interpolate between points") {
        REQUIRE(lut.interpolate(25.0) == Approx(12.5).margin(0.001));
        REQUIRE(lut.interpolate(75.0) == Approx(50.0).margin(0.001));
    }

    SECTION("Clamp to bounds") {
        REQUIRE(lut.interpolate(-10.0) == Approx(0.0).margin(0.001));
        REQUIRE(lut.interpolate(150.0) == Approx(75.0).margin(0.001));
    }
}

TEST_CASE("LookupTableConverter reverse lookup works", "[utils]") {
    std::vector<LUTEntry> table = {
        {0.0, 0.0},
        {50.0, 25.0},
        {100.0, 75.0}
    };

    LookupTableConverter lut(table);

    SECTION("Reverse lookup at exact points") {
        REQUIRE(lut.reverse_lookup(0.0) == Approx(0.0).margin(0.001));
        REQUIRE(lut.reverse_lookup(25.0) == Approx(50.0).margin(0.001));
        REQUIRE(lut.reverse_lookup(75.0) == Approx(100.0).margin(0.001));
    }
}

TEST_CASE("Transform functions work correctly", "[utils]") {
    SECTION("Identity transform") {
        auto f = Transform::identity();
        REQUIRE(f(42.0) == 42.0);
    }

    SECTION("Scale transform") {
        auto f = Transform::scale(2.0);
        REQUIRE(f(10.0) == 20.0);
    }

    SECTION("Offset transform") {
        auto f = Transform::offset(5.0);
        REQUIRE(f(10.0) == 15.0);
    }

    SECTION("Linear transform") {
        auto f = Transform::linear(2.0, 3.0);
        REQUIRE(f(10.0) == 23.0); // 10 * 2 + 3
    }

    SECTION("Negate transform") {
        auto f = Transform::negate();
        REQUIRE(f(10.0) == -10.0);
        REQUIRE(f(-5.0) == 5.0);
    }

    SECTION("Percent to ratio") {
        auto f = Transform::percent_to_ratio();
        REQUIRE(f(50.0) == Approx(0.5).margin(0.001));
        REQUIRE(f(100.0) == Approx(1.0).margin(0.001));
    }

    SECTION("Ratio to percent") {
        auto f = Transform::ratio_to_percent();
        REQUIRE(f(0.5) == Approx(50.0).margin(0.001));
        REQUIRE(f(1.0) == Approx(100.0).margin(0.001));
    }

    SECTION("Compose transforms") {
        auto scale = Transform::scale(2.0);
        auto offset = Transform::offset(5.0);
        auto composed = Transform::compose({scale, offset});

        REQUIRE(composed(10.0) == 25.0); // (10 * 2) + 5
    }
}

TEST_CASE("StringUtils trims whitespace", "[utils]") {
    REQUIRE(StringUtils::trim("  hello  ") == "hello");
    REQUIRE(StringUtils::trim("\thello\n") == "hello");
    REQUIRE(StringUtils::trim("hello") == "hello");
    REQUIRE(StringUtils::trim("   ") == "");
}

TEST_CASE("StringUtils converts case", "[utils]") {
    REQUIRE(StringUtils::to_lower("HELLO") == "hello");
    REQUIRE(StringUtils::to_lower("Hello World") == "hello world");
    REQUIRE(StringUtils::to_upper("hello") == "HELLO");
    REQUIRE(StringUtils::to_upper("Hello World") == "HELLO WORLD");
}

TEST_CASE("StringUtils splits strings", "[utils]") {
    auto parts = StringUtils::split("a,b,c", ',');
    REQUIRE(parts.size() == 3);
    REQUIRE(parts[0] == "a");
    REQUIRE(parts[1] == "b");
    REQUIRE(parts[2] == "c");

    auto path_parts = StringUtils::split("setup.aero.front_wing", '.');
    REQUIRE(path_parts.size() == 3);
    REQUIRE(path_parts[0] == "setup");
    REQUIRE(path_parts[1] == "aero");
    REQUIRE(path_parts[2] == "front_wing");
}

TEST_CASE("StringUtils joins strings", "[utils]") {
    std::vector<std::string> parts = {"a", "b", "c"};
    REQUIRE(StringUtils::join(parts, ",") == "a,b,c");
    REQUIRE(StringUtils::join(parts, " - ") == "a - b - c");

    std::vector<std::string> empty;
    REQUIRE(StringUtils::join(empty, ",") == "");
}

TEST_CASE("StringUtils checks string prefixes and suffixes", "[utils]") {
    REQUIRE(StringUtils::starts_with("hello world", "hello"));
    REQUIRE_FALSE(StringUtils::starts_with("hello world", "world"));

    REQUIRE(StringUtils::ends_with("hello world", "world"));
    REQUIRE_FALSE(StringUtils::ends_with("hello world", "hello"));
}

TEST_CASE("StringUtils replaces substrings", "[utils]") {
    std::string text = "hello world, hello universe";
    REQUIRE(StringUtils::replace_all(text, "hello", "hi") == "hi world, hi universe");
    REQUIRE(StringUtils::replace_all(text, "world", "earth") == "hello earth, hello universe");
}

TEST_CASE("DateTimeUtils validates ISO8601", "[utils]") {
    REQUIRE(DateTimeUtils::is_valid_iso8601("2024-01-15T10:30:00Z"));
    REQUIRE(DateTimeUtils::is_valid_iso8601("2024-01-15T10:30:00.123Z"));
    REQUIRE(DateTimeUtils::is_valid_iso8601("2024-01-15T10:30:00+02:00"));

    REQUIRE_FALSE(DateTimeUtils::is_valid_iso8601("2024-01-15"));
    REQUIRE_FALSE(DateTimeUtils::is_valid_iso8601("not a date"));
}

TEST_CASE("DateTimeUtils generates current timestamp", "[utils]") {
    std::string now = DateTimeUtils::now_iso8601();

    REQUIRE_FALSE(now.empty());
    REQUIRE(DateTimeUtils::is_valid_iso8601(now));
    REQUIRE(now.find("T") != std::string::npos);
    REQUIRE(now.find("Z") != std::string::npos);
}
