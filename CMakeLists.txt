cmake_minimum_required(VERSION 3.13...3.14)
project(VRAC_SDK)

add_library(${PROJECT_NAME} INTERFACE)
add_library(VRAC::sdk ALIAS ${PROJECT_NAME})

set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_EXTENSIONS          OFF
        CXX_STANDARD            23
        CXX_STANDARD_REQUIRED   ON
)
target_include_directories(${PROJECT_NAME}
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/fmt/include
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/range-v3/include
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/nlohmann-json/include
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/spdlog/include
)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_23)