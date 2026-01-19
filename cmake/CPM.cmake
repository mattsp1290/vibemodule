# CPM.cmake - CMake Package Manager
# This file provides CPM.cmake configuration for the vibemodule project.
#
# Usage:
#   include(cmake/CPM.cmake)
#
# Then use CPMAddPackage to add dependencies:
#   CPMAddPackage("gh:catchorg/Catch2@3.4.0")

# CPM version to download
set(CPM_DOWNLOAD_VERSION 0.40.2)

# Compute the CPM download location
if(NOT DEFINED CPM_DOWNLOAD_LOCATION)
    set(CPM_DOWNLOAD_LOCATION "${CMAKE_BINARY_DIR}/cmake/CPM_${CPM_DOWNLOAD_VERSION}.cmake")
endif()

# Set the source cache for faster subsequent builds
if(NOT DEFINED CPM_SOURCE_CACHE)
    set(CPM_SOURCE_CACHE "${CMAKE_SOURCE_DIR}/.cpm_cache" CACHE PATH "CPM source cache directory")
endif()

# Download CPM.cmake if not present
if(NOT EXISTS ${CPM_DOWNLOAD_LOCATION})
    message(STATUS "Downloading CPM.cmake v${CPM_DOWNLOAD_VERSION}")
    file(DOWNLOAD
        https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake
        ${CPM_DOWNLOAD_LOCATION}
        EXPECTED_HASH SHA256=c8cdc32c03816538ce22781ed72964dc864b2a34a310d3b7104812a5ca2d835d
        STATUS download_status
    )
    list(GET download_status 0 status_code)
    if(NOT status_code EQUAL 0)
        message(FATAL_ERROR "Failed to download CPM.cmake: ${download_status}")
    endif()
endif()

include(${CPM_DOWNLOAD_LOCATION})

# Optionally silence CPM output
# set(CPM_USE_NAMED_CACHE_DIRECTORIES ON)

message(STATUS "CPM.cmake v${CPM_DOWNLOAD_VERSION} loaded")
message(STATUS "CPM source cache: ${CPM_SOURCE_CACHE}")
