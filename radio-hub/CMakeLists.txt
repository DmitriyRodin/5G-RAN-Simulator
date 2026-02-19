cmake_minimum_required(VERSION 3.16)
project(RadiohubModule LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

add_library(radiohub_lib STATIC
    include/radio_hub.hpp
    src/radio_hub.cpp
)

target_include_directories(radiohub_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}"
)

find_package(Qt6 REQUIRED COMPONENTS
    Core
)

target_link_libraries(radiohub_lib PUBLIC
    Qt6::Core
    common_lib
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/radio_hub_main.cpp")
    add_executable(radiohub_app src/radio_hub_main.cpp)
    target_link_libraries(radiohub_app PRIVATE radiohub_lib)
endif()
