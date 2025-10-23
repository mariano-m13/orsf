cmake_minimum_required(VERSION 3.15)
project(orsf VERSION 1.0.0 LANGUAGES CXX)

# C++17 required
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build options
option(ORSF_BUILD_TESTS "Build ORSF tests" ON)
option(ORSF_BUILD_EXAMPLES "Build ORSF examples" ON)
option(ORSF_HEADER_ONLY "Build ORSF as header-only library" OFF)

# Include FetchContent for dependencies
include(FetchContent)

# nlohmann/json for JSON handling
FetchContent_Declare(
    json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.3
)
FetchContent_MakeAvailable(json)

# Main library
if(ORSF_HEADER_ONLY)
    add_library(orsf INTERFACE)
    target_include_directories(orsf INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
    target_compile_definitions(orsf INTERFACE ORSF_HEADER_ONLY)
else()
    add_library(orsf
        src/core.cpp
        src/validator.cpp
        src/mapping.cpp
        src/utils.cpp
        src/adapter.cpp
    )
    target_include_directories(orsf PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    )
endif()

# Link nlohmann/json
target_link_libraries(orsf ${ORSF_LIB_TYPE} nlohmann_json::nlohmann_json)

# Compiler warnings
if(MSVC)
    target_compile_options(orsf ${ORSF_LIB_TYPE} /W4)
else()
    target_compile_options(orsf ${ORSF_LIB_TYPE} -Wall -Wextra -Wpedantic)
endif()

# Tests
if(ORSF_BUILD_TESTS)
    enable_testing()

    # Catch2 for testing
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.5.2
    )
    FetchContent_MakeAvailable(Catch2)

    add_subdirectory(tests)
endif()

# Examples
if(ORSF_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()

# Installation
install(TARGETS orsf
    EXPORT orsfTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(DIRECTORY include/orsf
    DESTINATION include
)

# Export targets
install(EXPORT orsfTargets
    FILE orsfTargets.cmake
    NAMESPACE orsf::
    DESTINATION lib/cmake/orsf
)

# Package config
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/orsfConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/orsfConfigVersion.cmake"
    DESTINATION lib/cmake/orsf
)
