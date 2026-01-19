cmake_minimum_required(VERSION 3.21)

project(vibemodule
    VERSION 1.0.0
    DESCRIPTION "Clouds Reverb port for JUCE, Percussa SSP, and Daisy Patch"
    LANGUAGES CXX
)

# Require C++17 or later
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Export compile commands for IDE support
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "Release" CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo" "MinSizeRel")
endif()

# CPM.cmake - Package Manager
include(cmake/CPM.cmake)

# Project options
option(VIBEMODULE_BUILD_JUCE "Build JUCE VST plugin" ON)
option(VIBEMODULE_BUILD_SSP "Build Percussa SSP module" OFF)
option(VIBEMODULE_BUILD_DAISY "Build Daisy Patch firmware" OFF)
option(VIBEMODULE_BUILD_TESTS "Build unit tests" ON)

# Add subdirectories
# Core DSP library (always built)
add_subdirectory(libs/clouds-dsp)

# Platform-specific builds
if(VIBEMODULE_BUILD_JUCE)
    add_subdirectory(platforms/juce)
endif()

# if(VIBEMODULE_BUILD_SSP)
#     add_subdirectory(platforms/ssp)
# endif()

# if(VIBEMODULE_BUILD_DAISY)
#     add_subdirectory(platforms/daisy)
# endif()

# Tests
# if(VIBEMODULE_BUILD_TESTS)
#     enable_testing()
#     add_subdirectory(tests)
# endif()

# Print configuration summary
message(STATUS "")
message(STATUS "vibemodule Configuration Summary:")
message(STATUS "  Version:      ${PROJECT_VERSION}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  Build Type:   ${CMAKE_BUILD_TYPE}")
message(STATUS "  JUCE Plugin:  ${VIBEMODULE_BUILD_JUCE}")
message(STATUS "  SSP Module:   ${VIBEMODULE_BUILD_SSP}")
message(STATUS "  Daisy Patch:  ${VIBEMODULE_BUILD_DAISY}")
message(STATUS "  Tests:        ${VIBEMODULE_BUILD_TESTS}")
message(STATUS "")
