cmake_minimum_required(VERSION 3.16)
project(GnbModule LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

add_library(gnb_lib STATIC
    include/gnb_logic.hpp
    include/sys_info_service.hpp
    src/gnb_logic.cpp
    src/sys_info_service.cpp
)

target_include_directories(gnb_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}"
)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Network
)

target_link_libraries(gnb_lib PUBLIC
    Qt6::Core
    Qt6::Network
    common_lib
    common_proto_lib
)

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/gnb_main.cpp")
    add_executable(gnb_app src/gnb_main.cpp)
    target_link_libraries(gnb_app PRIVATE gnb_lib)
endif()

if(BUILD_TESTS)
    add_subdirectory(tests)
endif()
