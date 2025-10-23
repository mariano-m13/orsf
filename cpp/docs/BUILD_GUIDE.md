# ORSF C++ Build Guide

Complete guide for building and installing the ORSF C++ library.

## Requirements

### Minimum Requirements

- **C++ Compiler**: C++17 compliant
  - GCC 7+
  - Clang 5+
  - MSVC 2017+
  - AppleClang 10+ (Xcode 10+)
- **CMake**: 3.15 or higher
- **Git**: For fetching dependencies

### Dependencies

All dependencies are automatically fetched by CMake:

- **nlohmann/json** (v3.11.3) - JSON parsing
- **Catch2** (v3.5.2) - Testing framework (optional, if tests enabled)

## Quick Start

### Linux / macOS

```bash
# Clone repository
git clone https://github.com/mariano-m13/orsf.git
cd orsf/cpp

# Build
mkdir build && cd build
cmake ..
cmake --build . -j$(nproc)

# Run tests
ctest --verbose

# Install (optional)
sudo cmake --install .
```

### Windows (Visual Studio)

```cmd
# Clone repository
git clone https://github.com/mariano-m13/orsf.git
cd orsf\cpp

# Generate Visual Studio solution
mkdir build
cd build
cmake ..

# Build with Visual Studio
cmake --build . --config Release

# Run tests
ctest -C Release --verbose

# Install (optional, run as Administrator)
cmake --install . --config Release
```

### Windows (MinGW)

```bash
mkdir build && cd build
cmake -G "MinGW Makefiles" ..
cmake --build .
ctest --verbose
```

## Build Options

### Standard Build (Recommended)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

### Debug Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

### Header-Only Mode

Build as header-only library (no compiled library):

```bash
cmake -B build -DORSF_HEADER_ONLY=ON
cmake --build build
```

**Note**: In header-only mode, all implementation is in headers. This increases compilation time but allows for better optimization.

### Disable Tests

Skip building tests (faster build):

```bash
cmake -B build -DORSF_BUILD_TESTS=OFF
cmake --build build
```

### Disable Examples

Skip building examples:

```bash
cmake -B build -DORSF_BUILD_EXAMPLES=OFF
cmake --build build
```

### Custom Install Prefix

Install to custom location:

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=/opt/orsf
cmake --build build
cmake --install build
```

### Enable Warnings as Errors

```bash
cmake -B build -DCMAKE_CXX_FLAGS="-Werror"
cmake --build build
```

## Platform-Specific Instructions

### macOS

#### Using Homebrew

```bash
# Install dependencies (if needed)
brew install cmake

# Build
mkdir build && cd build
cmake ..
make -j$(sysctl -n hw.ncpu)
```

#### Xcode

Generate Xcode project:

```bash
cmake -B build -G Xcode
open build/orsf.xcodeproj
```

### Linux

#### Ubuntu / Debian

```bash
# Install build tools
sudo apt update
sudo apt install build-essential cmake git

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)
```

#### Fedora / RHEL

```bash
# Install build tools
sudo dnf install gcc-c++ cmake git

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)
```

#### Arch Linux

```bash
# Install build tools
sudo pacman -S base-devel cmake git

# Build
mkdir build && cd build
cmake ..
make -j$(nproc)
```

### Windows

#### Visual Studio 2022

```cmd
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

#### Visual Studio 2019

```cmd
cmake -B build -G "Visual Studio 16 2019" -A x64
cmake --build build --config Release
```

#### Ninja

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Integration Methods

### Method 1: FetchContent (Recommended)

Add to your `CMakeLists.txt`:

```cmake
include(FetchContent)

FetchContent_Declare(
    orsf
    GIT_REPOSITORY https://github.com/mariano-m13/orsf.git
    GIT_TAG v1.0.0
)
FetchContent_MakeAvailable(orsf)

target_link_libraries(your_target PRIVATE orsf)
```

### Method 2: Add as Subdirectory

```bash
# Clone ORSF into your project
cd your_project
git clone https://github.com/mariano-m13/orsf.git external/orsf
```

```cmake
# In your CMakeLists.txt
add_subdirectory(external/orsf/cpp)
target_link_libraries(your_target PRIVATE orsf)
```

### Method 3: System-Wide Installation

```bash
# Build and install
cd orsf/cpp
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local
cmake --build build
sudo cmake --install build
```

```cmake
# In your CMakeLists.txt
find_package(orsf REQUIRED)
target_link_libraries(your_target PRIVATE orsf::orsf)
```

### Method 4: Manual Integration

Copy `include/orsf/` to your include path and link against nlohmann/json.

## Testing

### Run All Tests

```bash
cd build
ctest --verbose
```

### Run Specific Test

```bash
./tests/orsf_tests "[core]"              # Core tests only
./tests/orsf_tests "[validator]"         # Validator tests
./tests/orsf_tests "[utils]"             # Utilities tests
```

### Test Coverage (Linux/macOS)

```bash
# Build with coverage
cmake -B build -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="--coverage"
cmake --build build

# Run tests
cd build
ctest

# Generate coverage report
gcov ../src/*.cpp
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_html
```

## Examples

Build and run examples:

```bash
# Build
cd build
cmake --build .

# Run examples
./examples/basic_usage
./examples/validation_example
./examples/conversion_example
./examples/adapter_example
```

## Troubleshooting

### CMake can't find nlohmann/json

**Solution**: Ensure you have internet connection. CMake will automatically download it via FetchContent.

### Compiler version too old

**Error**: `C++17 required`

**Solution**: Update your compiler:
- **Ubuntu**: `sudo apt install g++-9`
- **macOS**: `brew install gcc`
- **Windows**: Download latest Visual Studio or MinGW-w64

### "No such file or directory: orsf/orsf.hpp"

**Solution**: Ensure you're using correct include path:
```cpp
#include <orsf/orsf.hpp>  // Correct
// NOT: #include "orsf.hpp"
```

### Tests failing

**Solution**:
1. Ensure you built with tests enabled: `-DORSF_BUILD_TESTS=ON`
2. Run verbose tests: `ctest --verbose`
3. Check test output for specific failures

### Linking errors on Windows

**Solution**: Ensure you're building with same configuration (Debug/Release) as your project.

### Header-only mode compilation errors

**Solution**: Ensure `ORSF_HEADER_ONLY` is defined:
```cpp
#define ORSF_HEADER_ONLY
#include <orsf/orsf.hpp>
```

## Performance Optimization

### Release Build with LTO

```bash
cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=ON
cmake --build build
```

### Native CPU Optimization

```bash
cmake -B build \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_CXX_FLAGS="-march=native"
cmake --build build
```

**Warning**: Binaries may not be portable to other CPUs.

## Static Analysis

### Clang-Tidy

```bash
cmake -B build -DCMAKE_CXX_CLANG_TIDY="clang-tidy"
cmake --build build
```

### Cppcheck

```bash
cppcheck --enable=all --std=c++17 src/ include/
```

## Packaging

### Create Debian Package

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr
cd build
cpack -G DEB
```

### Create RPM Package

```bash
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr
cd build
cpack -G RPM
```

### Create Archive

```bash
cd build
cpack -G TGZ
```

## Cross-Compilation

### Android (NDK)

```bash
cmake -B build \
    -DCMAKE_TOOLCHAIN_FILE=$ANDROID_NDK/build/cmake/android.toolchain.cmake \
    -DANDROID_ABI=arm64-v8a \
    -DANDROID_PLATFORM=android-21
cmake --build build
```

### iOS

```bash
cmake -B build -G Xcode \
    -DCMAKE_SYSTEM_NAME=iOS \
    -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build --config Release
```

### WebAssembly (Emscripten)

```bash
emcmake cmake -B build
cmake --build build
```

## Continuous Integration

### GitHub Actions Example

```yaml
name: Build

on: [push, pull_request]

jobs:
  build:
    runs-on: ${{ matrix.os }}
    strategy:
      matrix:
        os: [ubuntu-latest, macos-latest, windows-latest]
        build_type: [Debug, Release]

    steps:
    - uses: actions/checkout@v3

    - name: Configure
      run: cmake -B build -DCMAKE_BUILD_TYPE=${{ matrix.build_type }}

    - name: Build
      run: cmake --build build

    - name: Test
      run: cd build && ctest --verbose
```

## Clean Build

Remove build artifacts:

```bash
# Unix
rm -rf build/

# Windows
rmdir /s /q build
```

## Support

For build issues:
1. Check this guide
2. Search [GitHub Issues](https://github.com/mariano-m13/orsf/issues)
3. Open a new issue with:
   - OS and version
   - Compiler and version
   - CMake version
   - Full build log

---

**Last updated**: 2024-01-15
