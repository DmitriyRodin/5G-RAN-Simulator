cmake_minimum_required(VERSION 3.16)
project(CommonModule LANGUAGES CXX)

set(CMAKE_AUTOMOC ON)

add_library(common_lib STATIC
    include/base_entity.hpp
    include/sim_protocol.hpp
    include/types.hpp
    include/udp_transport.hpp
    include/flow_logger.hpp
    src/base_entity.cpp
    src/sim_protocol.cpp
    src/types.cpp
    src/udp_transport.cpp
    src/flow_logger.cpp
)

target_include_directories(common_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_SOURCE_DIR}"
)

find_package(Qt6 REQUIRED COMPONENTS
    Core
    Network
)

target_link_libraries(common_lib PUBLIC
    Qt6::Core
    Qt6::Network
)


