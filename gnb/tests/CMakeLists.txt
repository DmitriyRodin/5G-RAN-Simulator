find_package(GTest REQUIRED)
find_package(Qt6 REQUIRED COMPONENTS Test)

if(NOT TARGET GTest::gmock)
    message(STATUS "GMock target not found, let's searching it manually")
    find_library(GMOCK_LIB gmock HINTS /usr/local/lib /usr/lib/x86_64-linux-gnu)
    add_library(GTest::gmock UNKNOWN IMPORTED)
    set_target_properties(GTest::gmock PROPERTIES
        IMPORTED_LOCATION "${GMOCK_LIB}"
        INTERFACE_LINK_LIBRARIES "GTest::gtest"
    )
endif()

add_executable(gnb_tests
    test_runner.cpp
    gnb_logic_test.cpp
)

target_compile_definitions(gnb_tests PRIVATE UNIT_TESTS)

target_link_libraries(gnb_tests PRIVATE
    gnb_lib
    GTest::gtest
    GTest::gmock
    Qt6::Test
)

add_test(NAME GnbLogicTests COMMAND gnb_tests)
