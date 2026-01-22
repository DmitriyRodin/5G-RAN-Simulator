cmake_minimum_required(VERSION 3.16)

project(5G-RAN-Simulator VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(5G-RAN-Simulator
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        common/types.hpp
        common/types.cpp
        common/udp_transport.hpp
        common/udp_transport.cpp
        common/base_entity.hpp
        common/base_entity.cpp
        gnb/gnb_logic.hpp
        gnb/gnb_logic.cpp
        ue/src/ue_logic.hpp
        ue/src/ue_logic.cpp
    )
# Define target properties for Android with Qt 6 as:
#    set_property(TARGET 5G-RAN-Simulator APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 ${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
else()
    if(ANDROID)
        add_library(5G-RAN-Simulator SHARED
            ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
    else()
        add_executable(5G-RAN-Simulator
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(5G-RAN-Simulator PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt6::Network
)

target_include_directories(5G-RAN-Simulator PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
)

# Qt for iOS sets MACOSX_BUNDLE_GUI_IDENTIFIER automatically since Qt 6.1.
# If you are developing for iOS or macOS you should consider setting an
# explicit, fixed bundle identifier manually though.
if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.5G-RAN-Simulator)
endif()
set_target_properties(5G-RAN-Simulator PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS 5G-RAN-Simulator
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(5G-RAN-Simulator)
endif()
