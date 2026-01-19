#!/bin/bash
# Project: Clouds Reverb Port (VibeModule)
# Generated: 2026-01-19
# Targets: JUCE VST (Win/Mac), Percussa SSP, Daisy Patch

set -e

# Helper function to create a bead and capture its ID
create_bead() {
    local output
    output=$(bd create "$1" -p "$2" --label "$3" 2>&1)
    echo "$output" | grep -oP 'Created issue: \K[a-z0-9-]+' || echo "$output" >&2
}

# Initialize beads if needed
if [ ! -d ".beads" ]; then
    bd init
fi

echo "Creating Clouds Reverb Port task graph..."
echo ""

# ========================================
# Phase 1: Project Setup & Infrastructure
# ========================================
echo "Phase 1: Project Setup & Infrastructure"

SETUP_CMAKE=$(create_bead "Initialize CMake project with CPM package manager and configure minimum C++17 standard" 0 setup)
echo "  Created: $SETUP_CMAKE"

SETUP_DIRS=$(create_bead "Create project directory structure: libs/clouds-dsp/, platforms/{juce,ssp,daisy}/, tests/, cmake/" 0 setup)
echo "  Created: $SETUP_DIRS"
bd dep add $SETUP_DIRS $SETUP_CMAKE

SETUP_CPM=$(create_bead "Configure CPM.cmake module and add to cmake/ directory with proper cache settings" 1 setup)
echo "  Created: $SETUP_CPM"
bd dep add $SETUP_CPM $SETUP_CMAKE

SETUP_LINT=$(create_bead "Set up .clang-format and .clang-tidy for consistent C++ code style across platforms" 2 setup)
echo "  Created: $SETUP_LINT"
bd dep add $SETUP_LINT $SETUP_DIRS

SETUP_CI=$(create_bead "Create GitHub Actions CI workflow for multi-platform builds (Windows, macOS, Linux/ARM)" 2 deploy)
echo "  Created: $SETUP_CI"
bd dep add $SETUP_CI $SETUP_CMAKE

SETUP_PRESETS=$(create_bead "Configure CMake presets for Debug/Release and each target platform (juce, ssp, daisy)" 1 setup)
echo "  Created: $SETUP_PRESETS"
bd dep add $SETUP_PRESETS $SETUP_CPM

# ========================================
# Phase 2: Core DSP Library Port
# ========================================
echo ""
echo "Phase 2: Core DSP Library Port"

PORT_STMLIB=$(create_bead "Port Clouds stmlib utilities: basic DSP math, SIMD helpers, and type definitions to libs/clouds-dsp/stmlib/" 0 core)
echo "  Created: $PORT_STMLIB"
bd dep add $PORT_STMLIB $SETUP_DIRS

PORT_FXENGINE=$(create_bead "Port Clouds FxEngine template class with buffer management and delay line operations" 0 core)
echo "  Created: $PORT_FXENGINE"
bd dep add $PORT_FXENGINE $PORT_STMLIB

PORT_ALLPASS=$(create_bead "Port Clouds allpass filter implementation used in reverb diffusers" 0 core)
echo "  Created: $PORT_ALLPASS"
bd dep add $PORT_ALLPASS $PORT_STMLIB

PORT_REVERB=$(create_bead "Port Clouds Reverb class with Griesinger topology (4 input diffusers, nested delay loops)" 0 core)
echo "  Created: $PORT_REVERB"
bd dep add $PORT_REVERB $PORT_FXENGINE
bd dep add $PORT_REVERB $PORT_ALLPASS

DSP_DECOUPLE=$(create_bead "Remove hardware-specific dependencies from DSP code (STM32 references, hardware buffers)" 0 core)
echo "  Created: $DSP_DECOUPLE"
bd dep add $DSP_DECOUPLE $PORT_REVERB

DSP_WRAPPER=$(create_bead "Create platform-agnostic CloudsReverb wrapper class with clean public API" 0 core)
echo "  Created: $DSP_WRAPPER"
bd dep add $DSP_WRAPPER $DSP_DECOUPLE

DSP_SAMPLERATE=$(create_bead "Implement configurable sample rate support (44.1kHz, 48kHz, 96kHz) with proper coefficient scaling" 1 core)
echo "  Created: $DSP_SAMPLERATE"
bd dep add $DSP_SAMPLERATE $DSP_WRAPPER

DSP_CHANNELS=$(create_bead "Add mono and stereo processing modes to CloudsReverb wrapper" 1 core)
echo "  Created: $DSP_CHANNELS"
bd dep add $DSP_CHANNELS $DSP_WRAPPER

DSP_CMAKE=$(create_bead "Create CMakeLists.txt for clouds-dsp library as a static library target" 1 core)
echo "  Created: $DSP_CMAKE"
bd dep add $DSP_CMAKE $DSP_WRAPPER

# ========================================
# Phase 3: DSP Testing
# ========================================
echo ""
echo "Phase 3: DSP Testing"

SETUP_TEST=$(create_bead "Set up Catch2 or GoogleTest via CPM for unit testing framework" 1 testing)
echo "  Created: $SETUP_TEST"
bd dep add $SETUP_TEST $SETUP_CPM

TEST_STMLIB=$(create_bead "Write unit tests for stmlib math utilities and type conversions" 2 testing)
echo "  Created: $TEST_STMLIB"
bd dep add $TEST_STMLIB $SETUP_TEST
bd dep add $TEST_STMLIB $PORT_STMLIB

TEST_FXENGINE=$(create_bead "Write unit tests for FxEngine delay line operations and buffer management" 2 testing)
echo "  Created: $TEST_FXENGINE"
bd dep add $TEST_FXENGINE $SETUP_TEST
bd dep add $TEST_FXENGINE $PORT_FXENGINE

TEST_ALLPASS=$(create_bead "Write unit tests for allpass filter frequency response and phase characteristics" 2 testing)
echo "  Created: $TEST_ALLPASS"
bd dep add $TEST_ALLPASS $SETUP_TEST
bd dep add $TEST_ALLPASS $PORT_ALLPASS

TEST_REVERB=$(create_bead "Write integration tests for CloudsReverb: impulse response, decay time, wet/dry mix" 1 testing)
echo "  Created: $TEST_REVERB"
bd dep add $TEST_REVERB $SETUP_TEST
bd dep add $TEST_REVERB $DSP_WRAPPER

TEST_REFERENCE=$(create_bead "Create audio file comparison tests using reference recordings from original Clouds" 2 testing)
echo "  Created: $TEST_REFERENCE"
bd dep add $TEST_REFERENCE $TEST_REVERB

TEST_PERF=$(create_bead "Add performance benchmarks for DSP processing (measure CPU cycles per sample)" 3 testing)
echo "  Created: $TEST_PERF"
bd dep add $TEST_PERF $TEST_REVERB

# ========================================
# Phase 4: JUCE Platform (VST3)
# ========================================
echo ""
echo "Phase 4: JUCE Platform (VST3)"

JUCE_CPM=$(create_bead "Add JUCE framework via CPM with AudioPlugin module enabled" 0 juce)
echo "  Created: $JUCE_CPM"
bd dep add $JUCE_CPM $SETUP_CPM
bd dep add $JUCE_CPM $DSP_WRAPPER

JUCE_PROCESSOR=$(create_bead "Create JUCE AudioProcessor subclass for CloudsReverb plugin" 0 juce)
echo "  Created: $JUCE_PROCESSOR"
bd dep add $JUCE_PROCESSOR $JUCE_CPM

JUCE_PARAMS=$(create_bead "Implement JUCE AudioProcessorValueTreeState for parameter management (amount, time, diffusion, lp, input_gain)" 0 juce)
echo "  Created: $JUCE_PARAMS"
bd dep add $JUCE_PARAMS $JUCE_PROCESSOR

JUCE_SMOOTHING=$(create_bead "Create parameter smoothing for real-time safe parameter changes" 1 juce)
echo "  Created: $JUCE_SMOOTHING"
bd dep add $JUCE_SMOOTHING $JUCE_PARAMS

JUCE_PROCESS=$(create_bead "Implement processBlock() bridging JUCE audio buffers to CloudsReverb" 0 juce)
echo "  Created: $JUCE_PROCESS"
bd dep add $JUCE_PROCESS $JUCE_PARAMS
bd dep add $JUCE_PROCESS $DSP_WRAPPER

JUCE_PRESETS=$(create_bead "Add preset management system with factory presets (Hall, Plate, Shimmer, etc.)" 2 juce)
echo "  Created: $JUCE_PRESETS"
bd dep add $JUCE_PRESETS $JUCE_PARAMS

JUCE_EDITOR=$(create_bead "Create minimal JUCE AudioProcessorEditor with parameter sliders and preset selector" 1 juce)
echo "  Created: $JUCE_EDITOR"
bd dep add $JUCE_EDITOR $JUCE_PARAMS

JUCE_STYLE=$(create_bead "Style JUCE UI with custom LookAndFeel inspired by Mutable Instruments aesthetic" 3 juce)
echo "  Created: $JUCE_STYLE"
bd dep add $JUCE_STYLE $JUCE_EDITOR

JUCE_CMAKE=$(create_bead "Configure JUCE CMakeLists.txt for VST3 and AU format builds on Windows and macOS" 1 juce)
echo "  Created: $JUCE_CMAKE"
bd dep add $JUCE_CMAKE $JUCE_PROCESS
bd dep add $JUCE_CMAKE $DSP_CMAKE

JUCE_TEST=$(create_bead "Test JUCE plugin in DAW hosts (Ableton, Logic, Reaper) on Windows and macOS" 1 juce)
echo "  Created: $JUCE_TEST"
bd dep add $JUCE_TEST $JUCE_CMAKE

JUCE_AAX=$(create_bead "Add AAX format support for Pro Tools compatibility" 3 juce)
echo "  Created: $JUCE_AAX"
bd dep add $JUCE_AAX $JUCE_TEST

# ========================================
# Phase 5: Percussa SSP Platform
# ========================================
echo ""
echo "Phase 5: Percussa SSP Platform"

SSP_SDK=$(create_bead "Set up Percussa SSP SDK cross-compilation toolchain for ARM Linux" 0 ssp)
echo "  Created: $SSP_SDK"
bd dep add $SSP_SDK $SETUP_CMAKE
bd dep add $SSP_SDK $DSP_WRAPPER

SSP_WRAPPER=$(create_bead "Create SSP VST3 plugin wrapper class inheriting from SSP SDK base" 0 ssp)
echo "  Created: $SSP_WRAPPER"
bd dep add $SSP_WRAPPER $SSP_SDK

SSP_PORTS=$(create_bead "Implement SSP input/output port configuration (stereo in, stereo out, CV inputs)" 0 ssp)
echo "  Created: $SSP_PORTS"
bd dep add $SSP_PORTS $SSP_WRAPPER

SSP_CV=$(create_bead "Map SSP CV inputs to reverb parameters (amount, time, diffusion, lp)" 1 ssp)
echo "  Created: $SSP_CV"
bd dep add $SSP_CV $SSP_PORTS

SSP_GUI=$(create_bead "Implement SSP GUI panel with parameter display and CV indicators" 1 ssp)
echo "  Created: $SSP_GUI"
bd dep add $SSP_GUI $SSP_PORTS

SSP_CMAKE=$(create_bead "Configure CMake cross-compilation for SSP ARM target with proper SDK paths" 1 ssp)
echo "  Created: $SSP_CMAKE"
bd dep add $SSP_CMAKE $SSP_WRAPPER
bd dep add $SSP_CMAKE $DSP_CMAKE

SSP_META=$(create_bead "Create SSP module metadata (icon, description, category) for module browser" 2 ssp)
echo "  Created: $SSP_META"
bd dep add $SSP_META $SSP_GUI

SSP_TEST=$(create_bead "Test SSP module on actual hardware or SSP emulator" 1 ssp)
echo "  Created: $SSP_TEST"
bd dep add $SSP_TEST $SSP_CMAKE

SSP_NEON=$(create_bead "Optimize SSP build for ARM NEON SIMD if available" 3 ssp)
echo "  Created: $SSP_NEON"
bd dep add $SSP_NEON $SSP_TEST

# ========================================
# Phase 6: Daisy Patch Platform
# ========================================
echo ""
echo "Phase 6: Daisy Patch Platform"

DAISY_SDK=$(create_bead "Set up Daisy toolchain with libDaisy and DaisySP libraries" 0 daisy)
echo "  Created: $DAISY_SDK"
bd dep add $DAISY_SDK $SETUP_CMAKE
bd dep add $DAISY_SDK $DSP_WRAPPER

DAISY_MAIN=$(create_bead "Create Daisy Patch main.cpp with AudioCallback setup" 0 daisy)
echo "  Created: $DAISY_MAIN"
bd dep add $DAISY_MAIN $DAISY_SDK

DAISY_KNOBS=$(create_bead "Implement hardware control mapping: 4 knobs to reverb parameters" 0 daisy)
echo "  Created: $DAISY_KNOBS"
bd dep add $DAISY_KNOBS $DAISY_MAIN

DAISY_CV=$(create_bead "Map Daisy Patch CV inputs to modulate reverb parameters" 1 daisy)
echo "  Created: $DAISY_CV"
bd dep add $DAISY_CV $DAISY_KNOBS

DAISY_OLED=$(create_bead "Implement OLED display showing parameter values and mode" 1 daisy)
echo "  Created: $DAISY_OLED"
bd dep add $DAISY_OLED $DAISY_KNOBS

DAISY_ENCODER=$(create_bead "Add encoder button modes for switching between parameter pages" 2 daisy)
echo "  Created: $DAISY_ENCODER"
bd dep add $DAISY_ENCODER $DAISY_OLED

DAISY_PRESETS=$(create_bead "Implement preset save/load to SD card or flash storage" 2 daisy)
echo "  Created: $DAISY_PRESETS"
bd dep add $DAISY_PRESETS $DAISY_ENCODER

DAISY_MAKE=$(create_bead "Configure Makefile for Daisy Patch with proper linker script and ARM flags" 1 daisy)
echo "  Created: $DAISY_MAKE"
bd dep add $DAISY_MAKE $DAISY_MAIN
bd dep add $DAISY_MAKE $DSP_CMAKE

DAISY_CMAKE=$(create_bead "Integrate Daisy build into CMake as ExternalProject or custom target" 2 daisy)
echo "  Created: $DAISY_CMAKE"
bd dep add $DAISY_CMAKE $DAISY_MAKE

DAISY_OPTIMIZE=$(create_bead "Optimize DSP for Cortex-M7: fixed-point math where beneficial, cache-friendly memory access" 2 daisy)
echo "  Created: $DAISY_OPTIMIZE"
bd dep add $DAISY_OPTIMIZE $DAISY_MAKE

DAISY_TEST=$(create_bead "Test Daisy Patch firmware on actual hardware with audio I/O verification" 1 daisy)
echo "  Created: $DAISY_TEST"
bd dep add $DAISY_TEST $DAISY_MAKE

DAISY_DFU=$(create_bead "Add DFU flashing instructions and helper script for firmware upload" 2 daisy)
echo "  Created: $DAISY_DFU"
bd dep add $DAISY_DFU $DAISY_TEST

# ========================================
# Phase 7: Integration & Documentation
# ========================================
echo ""
echo "Phase 7: Integration & Documentation"

DOCS_README=$(create_bead "Create comprehensive README.md with build instructions for all platforms" 1 docs)
echo "  Created: $DOCS_README"
bd dep add $DOCS_README $JUCE_CMAKE
bd dep add $DOCS_README $SSP_CMAKE
bd dep add $DOCS_README $DAISY_MAKE

DOCS_PARAMS=$(create_bead "Document reverb parameters with ranges, units, and musical descriptions" 2 docs)
echo "  Created: $DOCS_PARAMS"
bd dep add $DOCS_PARAMS $DSP_WRAPPER

DOCS_ARCH=$(create_bead "Write architecture documentation explaining DSP signal flow and Griesinger topology" 2 docs)
echo "  Created: $DOCS_ARCH"
bd dep add $DOCS_ARCH $DSP_WRAPPER

DOCS_CONTRIB=$(create_bead "Create CONTRIBUTING.md with code style guide and PR process" 3 docs)
echo "  Created: $DOCS_CONTRIB"
bd dep add $DOCS_CONTRIB $SETUP_LINT

DOCS_LICENSE=$(create_bead "Add LICENSE file (check Clouds MIT license compatibility)" 1 docs)
echo "  Created: $DOCS_LICENSE"
bd dep add $DOCS_LICENSE $PORT_REVERB

CI_TEST=$(create_bead "Configure GitHub Actions for automated testing on all platforms" 1 deploy)
echo "  Created: $CI_TEST"
bd dep add $CI_TEST $SETUP_CI
bd dep add $CI_TEST $TEST_REVERB

CI_RELEASE=$(create_bead "Set up GitHub Actions release workflow for building binaries" 2 deploy)
echo "  Created: $CI_RELEASE"
bd dep add $CI_RELEASE $CI_TEST
bd dep add $CI_RELEASE $JUCE_TEST
bd dep add $CI_RELEASE $SSP_TEST
bd dep add $CI_RELEASE $DAISY_TEST

RELEASE=$(create_bead "Create GitHub release with pre-built VST3, AU, SSP, and Daisy firmware" 2 deploy)
echo "  Created: $RELEASE"
bd dep add $RELEASE $CI_RELEASE

MACOS_SIGN=$(create_bead "Add codesigning for macOS VST3/AU bundles (Developer ID)" 3 deploy)
echo "  Created: $MACOS_SIGN"
bd dep add $MACOS_SIGN $CI_RELEASE

WIN_INSTALLER=$(create_bead "Add Windows installer using NSIS or WiX for VST3 distribution" 3 deploy)
echo "  Created: $WIN_INSTALLER"
bd dep add $WIN_INSTALLER $CI_RELEASE

echo ""
echo "============================================"
echo "Bead graph created successfully!"
echo "============================================"
echo ""
echo "View the task graph:"
echo "  bv                    # Interactive TUI"
echo "  bv --robot-triage     # AI-friendly recommendations"
echo "  bd ready              # List unblocked tasks"
echo "  bd list --label setup # View setup tasks only"
echo ""
echo "Summary of labels:"
echo "  setup   - Project initialization and configuration"
echo "  core    - Core DSP library port"
echo "  testing - Unit and integration tests"
echo "  juce    - JUCE VST3 plugin"
echo "  ssp     - Percussa SSP module"
echo "  daisy   - Daisy Patch firmware"
echo "  docs    - Documentation"
echo "  deploy  - CI/CD and releases"
