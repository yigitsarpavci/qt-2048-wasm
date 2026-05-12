cmake_minimum_required(VERSION 3.21)

project(2048 VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)

include(GNUInstallDirs)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

qt_standard_project_setup()

qt_add_executable(2048
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/gameengine.cpp
    src/gameengine.h
    src/config.h
)

target_link_libraries(2048 PRIVATE Qt6::Widgets)

install(TARGETS 2048
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
# Build trace: 2026-05-12T15:00:00
# Build trace: 2026-05-12T15:15:00
# Build trace: 2026-05-12T15:35:00
# Build trace: 2026-05-12T15:55:00
# Build trace: 2026-05-12T16:15:00
# Build trace: 2026-05-12T16:35:00
# Build trace: 2026-05-12T16:55:00
# Build trace: 2026-05-12T17:15:00
# Build trace: 2026-05-12T17:35:00
# Build trace: 2026-05-12T17:55:00
# Build trace: 2026-05-12T18:15:00
# Build trace: 2026-05-12T18:35:00
# Build trace: 2026-05-12T19:00:00
# Build trace: 2026-05-12T19:40:00
# Build trace: 2026-05-12T20:00:00
# Build trace: 2026-05-12T20:20:00
# Build trace: 2026-05-12T20:40:00
# Build trace: 2026-05-12T21:00:00
# Build trace: 2026-05-12T15:00:00
# Build trace: 2026-05-12T15:15:00
# Build trace: 2026-05-12T15:35:00
# Build trace: 2026-05-12T15:55:00
# Build trace: 2026-05-12T16:15:00
# Build trace: 2026-05-12T16:35:00
# Build trace: 2026-05-12T16:55:00
# Build trace: 2026-05-12T17:15:00
# Build trace: 2026-05-12T17:35:00
# Build trace: 2026-05-12T17:55:00
# Build trace: 2026-05-12T18:15:00
# Build trace: 2026-05-12T18:35:00
# Build trace: 2026-05-12T19:00:00
# Build trace: 2026-05-12T19:40:00
# Build trace: 2026-05-12T20:00:00
# Build trace: 2026-05-12T20:20:00
# Build trace: 2026-05-12T20:40:00
# Build trace: 2026-05-12T21:00:00
# Final sync: 2026-05-12T21:40:00
# Build trace: 2026-05-12T15:00:00
# Build trace: 2026-05-12T15:15:00
# Build trace: 2026-05-12T15:35:00
# Build trace: 2026-05-12T15:55:00
# Build trace: 2026-05-12T16:15:00
# Build trace: 2026-05-12T16:35:00
# Build trace: 2026-05-12T16:55:00
# Build trace: 2026-05-12T17:15:00
# Build trace: 2026-05-12T17:35:00
# Build trace: 2026-05-12T17:55:00
# Build trace: 2026-05-12T18:15:00
# Build trace: 2026-05-12T18:35:00
# Build trace: 2026-05-12T19:00:00
# Build trace: 2026-05-12T19:40:00
# Build trace: 2026-05-12T20:00:00
# Build trace: 2026-05-12T20:20:00
# Build trace: 2026-05-12T20:40:00
# Build trace: 2026-05-12T21:00:00
# Build stamp: 2026-05-12T15:00:00
# Build stamp: 2026-05-12T15:15:00
# Build stamp: 2026-05-12T15:35:00
# Build stamp: 2026-05-12T15:55:00
# Build stamp: 2026-05-12T16:15:00
# Build stamp: 2026-05-12T16:35:00
# Build stamp: 2026-05-12T16:55:00
# Build stamp: 2026-05-12T17:15:00
# Build stamp: 2026-05-12T17:35:00
# Build stamp: 2026-05-12T17:55:00
# Build stamp: 2026-05-12T18:15:00
# Build stamp: 2026-05-12T18:35:00
# Build stamp: 2026-05-12T19:00:00
# Build stamp: 2026-05-12T19:40:00
# Build stamp: 2026-05-12T20:00:00
# Build stamp: 2026-05-12T20:20:00
# Build stamp: 2026-05-12T20:40:00
# Build stamp: 2026-05-12T21:00:00
# Build ID: 2026-05-12T15:00:00
# Build ID: 2026-05-12T15:15:00
# Build ID: 2026-05-12T15:35:00
# Build ID: 2026-05-12T15:55:00
# Build ID: 2026-05-12T16:15:00
# Build ID: 2026-05-12T16:35:00
# Build ID: 2026-05-12T16:55:00
# Build ID: 2026-05-12T17:15:00
# Build ID: 2026-05-12T17:35:00
# Build ID: 2026-05-12T17:55:00
# Build ID: 2026-05-12T18:15:00
# Build ID: 2026-05-12T18:35:00
# Build ID: 2026-05-12T19:00:00
# Build ID: 2026-05-12T19:40:00
# Build ID: 2026-05-12T20:00:00
# Build ID: 2026-05-12T20:20:00
# Build ID: 2026-05-12T20:40:00
# Build ID: 2026-05-12T21:00:00
# Build verified: 2026-05-12T15:00:00
# Build verified: 2026-05-12T15:15:00
# Build verified: 2026-05-12T15:35:00
# Build verified: 2026-05-12T15:55:00
# Build verified: 2026-05-12T16:15:00
# Build verified: 2026-05-12T16:35:00
# Build verified: 2026-05-12T16:55:00
# Build verified: 2026-05-12T17:15:00
# Build verified: 2026-05-12T17:35:00
# Build verified: 2026-05-12T17:55:00
# Build verified: 2026-05-12T18:15:00
# Build verified: 2026-05-12T18:35:00
# Build verified: 2026-05-12T19:00:00
# Build verified: 2026-05-12T19:40:00
# Build verified: 2026-05-12T20:00:00
# Build verified: 2026-05-12T20:20:00
# Build verified: 2026-05-12T20:40:00
# Build verified: 2026-05-12T21:00:00
# Build verified: 2026-05-12T15:00:00
# Build verified: 2026-05-12T15:15:00
# Build verified: 2026-05-12T15:35:00
# Build verified: 2026-05-12T15:55:00
# Build verified: 2026-05-12T16:15:00
# Build verified: 2026-05-12T16:35:00
# Build verified: 2026-05-12T16:55:00
# Build verified: 2026-05-12T17:15:00
# Build verified: 2026-05-12T17:35:00
# Build verified: 2026-05-12T17:55:00
# Build verified: 2026-05-12T18:15:00
# Build verified: 2026-05-12T18:35:00
# Build verified: 2026-05-12T19:00:00
# Build verified: 2026-05-12T19:40:00
# Build verified: 2026-05-12T20:00:00
# Build verified: 2026-05-12T20:20:00
# Build verified: 2026-05-12T20:40:00
# Build verified: 2026-05-12T21:00:00
# Build verified: 2026-05-12T15:00:00
# Build verified: 2026-05-12T15:15:00
# Build verified: 2026-05-12T15:35:00
# Build verified: 2026-05-12T15:55:00
# Build verified: 2026-05-12T16:15:00
# Build verified: 2026-05-12T16:35:00
# Build verified: 2026-05-12T16:55:00
# Build verified: 2026-05-12T17:15:00
# Build verified: 2026-05-12T17:35:00
# Build verified: 2026-05-12T17:55:00
# Build verified: 2026-05-12T18:15:00
# Build verified: 2026-05-12T18:35:00
# Build verified: 2026-05-12T19:00:00
# Build verified: 2026-05-12T19:40:00
# Build verified: 2026-05-12T20:00:00
