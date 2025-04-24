cmake_minimum_required(VERSION 3.20)

# Set compilers before project() command
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)

project(PIVOT)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find OpenMP package
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    message(STATUS "Found OpenMP ${OpenMP_CXX_VERSION}")
else()
    message(FATAL_ERROR "OpenMP not found!")
endif()

# Build type (Debug or Release)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

add_compile_options(-fPIC)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

file(GLOB ALP_CORE_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/fastlanes/*.cpp")

set(ENCODE_APP "pivot-encode")
set(EXP_APP "pivot-exp")
set(SERVER_APP "pivot-server")

# Executable: encode
set(ENCODE_FILES
    ${CMAKE_SOURCE_DIR}/src/om3_encode.cpp
    ${CMAKE_SOURCE_DIR}/src/Om3Encoder.cpp
)

add_executable(${ENCODE_APP} ${ENCODE_FILES})
set_target_properties(${ENCODE_APP} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build"
)

# Executable: experiment
set(EXPERIMENT_FILES
    ${CMAKE_SOURCE_DIR}/src/main.cpp
    ${CMAKE_SOURCE_DIR}/src/MemoryManager.cpp
    ${CMAKE_SOURCE_DIR}/src/utils.cpp
    ${ALP_CORE_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/AlpEncoder.cpp
    ${CMAKE_SOURCE_DIR}/src/AlpDecoder.cpp
    ${CMAKE_SOURCE_DIR}/src/CompressedData.cpp
)

add_executable(${EXP_APP} ${EXPERIMENT_FILES})

# Set compiler flags for OpenMP
target_link_libraries(${EXP_APP} PRIVATE OpenMP::OpenMP_CXX)
target_compile_options(${EXP_APP} PRIVATE ${OpenMP_CXX_FLAGS})

set_target_properties(${EXP_APP} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build"
)

# Executable: server

# 定义ASIO_STANDALONE宏
add_definitions(-DASIO_STANDALONE)

include_directories(
    ${CMAKE_SOURCE_DIR}/crow
    ${CMAKE_SOURCE_DIR}/asio/include
    ${CMAKE_SOURCE_DIR}/include
)

set(SERVER_FILES
    ${CMAKE_SOURCE_DIR}/src/server.cpp
    ${CMAKE_SOURCE_DIR}/src/MemoryManager.cpp
    ${CMAKE_SOURCE_DIR}/src/utils.cpp
    ${ALP_CORE_SOURCES}
    ${CMAKE_SOURCE_DIR}/src/AlpEncoder.cpp
    ${CMAKE_SOURCE_DIR}/src/AlpDecoder.cpp
    ${CMAKE_SOURCE_DIR}/src/CompressedData.cpp
)

add_executable(${SERVER_APP} ${SERVER_FILES})

# Set compiler flags for OpenMP
target_link_libraries(${SERVER_APP} PRIVATE OpenMP::OpenMP_CXX)
target_compile_options(${SERVER_APP} PRIVATE ${OpenMP_CXX_FLAGS})

set_target_properties(${SERVER_APP} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/build"
)

