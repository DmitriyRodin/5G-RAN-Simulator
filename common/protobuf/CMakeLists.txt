cmake_minimum_required(VERSION 3.16)
project(CommonProtobuf LANGUAGES CXX)

find_package(Protobuf REQUIRED)

set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/proto")
set(PROTO_FILE "${PROTO_DIR}/ran_messages.proto")

set(GEN_SRC "${CMAKE_CURRENT_BINARY_DIR}/ran_messages.pb.cc")
set(GEN_HDR "${CMAKE_CURRENT_BINARY_DIR}/ran_messages.pb.h")

add_custom_command(
    OUTPUT "${GEN_SRC}" "${GEN_HDR}"
    COMMAND ${Protobuf_PROTOC_EXECUTABLE}
    ARGS --cpp_out=${CMAKE_CURRENT_BINARY_DIR}
         --proto_path=${PROTO_DIR}
         --experimental_allow_proto3_optional
         ${PROTO_FILE}
    DEPENDS "${PROTO_FILE}"
    COMMENT "Running C++ protocol buffer compiler manually with optional support!"
    VERBATIM
)

add_library(common_proto_lib STATIC
    include/protobuf_serializer.hpp
    src/protobuf_serializer.cpp
    "${GEN_SRC}"
    "${GEN_HDR}"
)

target_include_directories(common_proto_lib PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_BINARY_DIR}"
)

find_package(Qt6 REQUIRED COMPONENTS Core)
target_link_libraries(common_proto_lib PUBLIC
    common_lib
    protobuf::libprotobuf
    Qt6::Core
)
