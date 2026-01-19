cmake_minimum_required(VERSION 3.21)

project(clouds-dsp
    VERSION 1.0.0
    DESCRIPTION "Clouds Reverb DSP library - ported from Mutable Instruments"
    LANGUAGES CXX
)

# Create a header-only interface library (no source files to compile)
add_library(clouds-dsp INTERFACE)
add_library(clouds::dsp ALIAS clouds-dsp)

target_include_directories(clouds-dsp
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)

target_compile_features(clouds-dsp INTERFACE cxx_std_17)

# Add compiler warnings for consumers
target_compile_options(clouds-dsp
    INTERFACE
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra>
        $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra>
        $<$<CXX_COMPILER_ID:AppleClang>:-Wall -Wextra>
        $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

# Installation rules
include(GNUInstallDirs)

install(TARGETS clouds-dsp
    EXPORT clouds-dsp-targets
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(EXPORT clouds-dsp-targets
    FILE clouds-dsp-config.cmake
    NAMESPACE clouds::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/clouds-dsp
)
