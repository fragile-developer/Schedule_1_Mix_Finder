cmake_minimum_required(VERSION 3.16) # safer default than 3.10
project(MixFinder LANGUAGES CXX C)

message (STATUS "Compiler path: ${CMAKE_CXX_COMPILER}")
message (STATUS "Compiler ID: ${CMAKE_CXX_COMPILER_ID}")

# statically link runtime for Windows binary
if (WIN32)
    if (MSVC)
        message(STATUS "Static linking for MSVC")
        set(CMAKE_MSVC_RUNTIME_LIBRARY
            "MultiThreaded$<$<CONFIG:Debug>:Debug>"
            CACHE STRING "" FORCE)
    else()
        message(STATUS "Static linking for MinGW")
        set(CMAKE_CXX_FLAGS        "${CMAKE_CXX_FLAGS}        -static -static-libgcc -static-libstdc++")
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static -static-libgcc -static-libstdc++")
    endif()
endif()

# ---------------------------------------------------------
# Configurable options
# ---------------------------------------------------------
set(EXE_NAME "MixFinder" CACHE STRING "Name of the final executable")

# C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default build type (Release if none given)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the build type" FORCE)
endif()

# ---------------------------------------------------------
# Dependencies
# ---------------------------------------------------------
include(FetchContent)

# FTXUI
set(FTXUI_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    ftxui
    GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI
    GIT_TAG v6.1.9
)
FetchContent_MakeAvailable(ftxui)

# Ankler
FetchContent_Declare(
    unordered_dense
    GIT_REPOSITORY https://github.com/martinus/unordered_dense
    GIT_TAG v4.8.1
)
FetchContent_MakeAvailable(unordered_dense)

# ---------------------------------------------------------
# Sources & executable
# ---------------------------------------------------------
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS src/*.cpp)

add_executable(${EXE_NAME} ${SOURCES})

# Compiler warnings
if (MSVC)
    add_compile_options(${EXE_NAME} PRIVATE /W4 /permissive-)
else()
    add_compile_options(${EXE_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

target_include_directories(${EXE_NAME}
    PRIVATE SYSTEM
        ${CMAKE_SOURCE_DIR}/include
        ${unordered_dense_SOURCE_DIR}/include
)

target_link_libraries(${EXE_NAME}
    PRIVATE
        ftxui::component
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
