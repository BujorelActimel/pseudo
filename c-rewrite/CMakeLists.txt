cmake_minimum_required(VERSION 3.20)
project(pseudo C)

# Project version
set(PSEUDO_VERSION_MAJOR 1)
set(PSEUDO_VERSION_MINOR 0)
set(PSEUDO_VERSION_PATCH 0)
set(PSEUDO_VERSION "${PSEUDO_VERSION_MAJOR}.${PSEUDO_VERSION_MINOR}.${PSEUDO_VERSION_PATCH}")

# C standard
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C compiler: ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")

# Options
option(BUILD_TESTING "Build tests" ON)
option(BUILD_WASM "Build for WebAssembly" OFF)
option(BUILD_SHARED_LIBS "Build shared libraries" OFF)
option(ENABLE_SANITIZERS "Enable sanitizers (debug builds)" OFF)
option(ENABLE_DAP "Build DAP server support" ON)

# Include CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Load CMake modules (will be created in TASK-002)
# include(CompilerWarnings)
# include(Sanitizers)
# include(TreeSitter)

# Conditional Emscripten setup
if(BUILD_WASM)
    # include(Emscripten)
    message(STATUS "Building for WebAssembly with Emscripten")
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Subdirectories (will be uncommented as modules are implemented)
add_subdirectory(src/common)
add_subdirectory(src/linter)
# add_subdirectory(src/parser)
# add_subdirectory(src/runtime)

# if(ENABLE_DAP)
#     add_subdirectory(src/debugger)
#     add_subdirectory(src/dap)
# endif()

# add_subdirectory(src/transpiler)

# if(BUILD_WASM)
#     add_subdirectory(src/wasm)
# else()
#     add_subdirectory(src/cli)
# endif()

# Testing
if(BUILD_TESTING)
    enable_testing()
    # add_subdirectory(test)
    # add_subdirectory(integration_tests)
endif()

# Print configuration summary
message(STATUS "")
message(STATUS "Configuration Summary:")
message(STATUS "  Version:           ${PSEUDO_VERSION}")
message(STATUS "  Build type:        ${CMAKE_BUILD_TYPE}")
message(STATUS "  C standard:        C${CMAKE_C_STANDARD}")
message(STATUS "  Build testing:     ${BUILD_TESTING}")
message(STATUS "  Build WASM:        ${BUILD_WASM}")
message(STATUS "  Shared libraries:  ${BUILD_SHARED_LIBS}")
message(STATUS "  Enable sanitizers: ${ENABLE_SANITIZERS}")
message(STATUS "  Enable DAP:        ${ENABLE_DAP}")
message(STATUS "")
