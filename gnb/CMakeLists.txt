cmake_minimum_required(VERSION 3.16)
project(GnbModule LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

add_library(gnb_lib STATIC
    include/gnb_logic.hpp
    src/gnb_logic.cpp
)

target_include_directories(gnb_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}"
)

find_package(Qt6 REQUIRED COMPONENTS
    Core
)

target_link_libraries(gnb_lib PUBLIC
    Qt6::Core
    common_lib
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/gnb_main.cpp")
    add_executable(gnb_app src/gnb_main.cpp)
    target_link_libraries(gnb_app PRIVATE gnb_lib)
endif()

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
