cmake_minimum_required(VERSION 3.16)
project(ChessAI VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Try to find Qt6 first, fallback to Qt5
find_package(Qt6 COMPONENTS Core Widgets Gui)
if(NOT Qt6_FOUND)
    message(STATUS "Qt6 not found, trying Qt5...")
    find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui)
    set(QT_VERSION_MAJOR 5)
else()
    set(QT_VERSION_MAJOR 6)
endif()

message(STATUS "Using Qt${QT_VERSION_MAJOR}")

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/include)

# Core library
set(CORE_SOURCES
    src/core/Board.cpp
    src/core/Piece.cpp
    src/core/Move.cpp
    src/core/Position.cpp
    src/core/MoveGenerator.cpp
    src/core/MoveValidator.cpp
)

set(CORE_HEADERS
    include/core/Board.h
    include/core/Piece.h
    include/core/Move.h
    include/core/Position.h
    include/core/MoveGenerator.h
    include/core/MoveValidator.h
    include/core/Types.h
)

# AI library
set(AI_SOURCES
    src/ai/Engine.cpp
    src/ai/Evaluator.cpp
)

set(AI_HEADERS
    include/ai/Engine.h
    include/ai/Evaluator.h
)

# UI sources
set(UI_SOURCES
    src/ui/MainWindow.cpp
    src/ui/ChessBoard.cpp
    src/ui/PieceWidget.cpp
    src/main.cpp
)

set(UI_HEADERS
    include/ui/MainWindow.h
    include/ui/ChessBoard.h
    include/ui/PieceWidget.h
)

# Resources
set(RESOURCES
    resources/resources.qrc
)

# Create libraries
add_library(chess_core STATIC ${CORE_SOURCES} ${CORE_HEADERS})
add_library(chess_ai STATIC ${AI_SOURCES} ${AI_HEADERS})
target_link_libraries(chess_ai chess_core)

# Main executable
add_executable(chess-ai 
    ${UI_SOURCES} 
    ${UI_HEADERS}
    ${RESOURCES}
)

if(QT_VERSION_MAJOR EQUAL 6)
    target_link_libraries(chess-ai
        chess_core
        chess_ai
        Qt6::Core
        Qt6::Widgets
        Qt6::Gui
    )
else()
    target_link_libraries(chess-ai
        chess_core
        chess_ai
        Qt5::Core
        Qt5::Widgets
        Qt5::Gui
    )
endif()

# Enable warnings
if(MSVC)
    target_compile_options(chess-ai PRIVATE /W4)
else()
    target_compile_options(chess-ai PRIVATE -Wall -Wextra -pedantic)
endif()

