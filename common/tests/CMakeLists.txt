find_package(GTest REQUIRED)

find_package(Qt6 REQUIRED COMPONENTS
    Core
)

add_executable(common_tests
    sim_protocol_test.cpp
)

target_compile_definitions(common_tests PRIVATE UNIT_TESTS)

target_link_libraries(common_tests PRIVATE
    common_lib
    Qt6::Core
    GTest::gtest
    GTest::gtest_main
)

add_test(NAME CommonTests COMMAND common_tests)
