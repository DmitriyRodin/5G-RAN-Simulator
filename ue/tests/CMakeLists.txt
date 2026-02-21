find_package(GTest REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Test)

add_executable(ue_tests
    test_runner.cpp
    ue_logic_test.cpp
)

target_compile_definitions(ue_tests PRIVATE UNIT_TESTS)

target_link_libraries(ue_tests PRIVATE
    ue_lib
    GTest::GTest
    Qt6::Test
)

add_test(NAME UeLogicTests COMMAND ue_tests)
