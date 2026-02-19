cmake_minimum_required(VERSION 3.16)
project(UeModule LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

add_library(ue_lib STATIC
    src/ue_logic.cpp
    include/ue_logic.hpp
)

target_include_directories(ue_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}"
)

find_package(Qt6 REQUIRED COMPONENTS
    Core
)

target_link_libraries(ue_lib PUBLIC
    Qt6::Core
    common_lib
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/ue_main.cpp")
    add_executable(ue_app src/ue_main.cpp)
    target_link_libraries(ue_app PRIVATE ue_lib)
endif()
