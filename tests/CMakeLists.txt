cmake_minimum_required(VERSION 3.21)

# Fetch Catch2 via CPM
CPMAddPackage(
    NAME Catch2
    GIT_TAG v3.5.2
    GITHUB_REPOSITORY catchorg/Catch2
)

# Add Catch2 cmake module path for catch_discover_tests
list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)

# Include Catch2 test discovery
include(CTest)
include(Catch)

# Create test executable
add_executable(vibemodule_tests
    test_clouds_reverb.cpp
    test_stmlib_dsp.cpp
)

target_link_libraries(vibemodule_tests
    PRIVATE
        Catch2::Catch2WithMain
        clouds::dsp
)

# Auto-discover tests
catch_discover_tests(vibemodule_tests)
