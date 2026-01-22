cmake_minimum_required(VERSION 3.16) # safer default than 3.10
project(MixFinder LANGUAGES CXX)

# ---------------------------------------------------------
# Configurable options
# ---------------------------------------------------------
set(EXE_NAME "MixFinder" CACHE STRING "Name of the final executable")

# C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default build type (Debug if none given)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the build type" FORCE)
endif()

message (STATUS "Compiler path: ${CMAKE_CXX_COMPILER}")

# statically link runtime for Windows binary
if (WIN32)
    if (MSVC)
        set(CMAKE_MSVC_RUNTIME_LIBRARY
            "MultiThreaded$<$<CONFIG:Debug>:Debug>"
            CACHE STRING "" FORCE)
        set(ABSL_MSVC_STATIC_RUNTIME ON CACHE BOOL "" FORCE)
    else()
        set(CMAKE_CXX_FLAGS        "${CMAKE_CXX_FLAGS}        -static -static-libgcc -static-libstdc++")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
    endif()
endif()

# Compiler warnings
if (MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------
# Dependencies
# ---------------------------------------------------------
include(FetchContent)

# FTXUI
FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
    GIT_TAG v6.1.9
)
FetchContent_MakeAvailable(ftxui)

# Abseil
FetchContent_Declare(
    abseil
    GIT_REPOSITORY https://github.com/abseil/abseil-cpp.git
    GIT_TAG 20250512.1
)
FetchContent_MakeAvailable(abseil)

# Boost (commented out until needed)
# find_package(Boost REQUIRED COMPONENTS container)

# ---------------------------------------------------------
# Sources & executable
# ---------------------------------------------------------
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(${EXE_NAME} ${SOURCES})

target_include_directories(${EXE_NAME}
    PRIVATE
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(${EXE_NAME}
    PRIVATE
        absl::flat_hash_map
        ftxui::component
        # Boost::container
)

# ---------------------------------------------------------
# Output directories
# ---------------------------------------------------------
set_target_properties(${EXE_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib
)

# ---------------------------------------------------------
# Install (optional)
# ---------------------------------------------------------
install(TARGETS ${EXE_NAME} DESTINATION bin)
