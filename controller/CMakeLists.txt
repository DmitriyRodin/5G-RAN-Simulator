cmake_minimum_required(VERSION 3.16)
project(ControllerModule LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(controller_lib STATIC
    simulation_controller.hpp
    simulation_controller.cpp
)

target_include_directories(controller_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}"
)

find_package(Qt6 REQUIRED COMPONENTS
    Core
)

target_link_libraries(controller_lib PUBLIC
    Qt6::Core
    gnb_lib
    ue_lib
    radiohub_lib
)
