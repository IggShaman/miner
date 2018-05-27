cmake_minimum_required(VERSION 3.10)
SET(CMAKE_CXX_STANDARD 14)
project(MINER)

OPTION(ENABLE_GLPK_SOLVER "Build glpk-based solver" OFF)
OPTION(ENABLE_SOPLEX_SOLVER "Build soplex-based solver" OFF)

include_directories(${MINER_SOURCE_DIR})

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra -fPIC")

SET(QT_COMPONENTS Core Widgets)
add_definitions(-DQT_NO_DEBUG -DQT)
find_package(Qt5 COMPONENTS ${QT_COMPONENTS})
include_directories(${Qt5Widgets_INCLUDE_DIRS})
add_definitions(${Qt5Core_DEFINITIONS} ${Qt5Widgets_DEFINITIONS})

include(${MINER_SOURCE_DIR}/cmake/pch.cmake)

SET(MINER_CXX_FILES
  solver.cc
  field.cc
  board.cc
  game_board_widget.cc
  main_window.cc
  util.cc
  lp_problem.cc
  main.cc
)

IF (${ENABLE_GLPK_SOLVER})
  LIST(APPEND MINER_CXX_FILES glpk_solver.cc)
  add_definitions(-DENABLE_GLPK_SOLVER)
ENDIF()

SET(MINER_UI_FORMS
  main_window.ui
  configure_field_dialog.ui
)

SET(MINER_MOC_HEADERS
  game_board_widget.h
  main_window.h
)

qt5_add_resources(MINER_RC_SRCS miner.qrc)
qt5_wrap_ui(MINER_UIS_H ${MINER_UI_FORMS})
qt5_wrap_cpp(MINER_MOC_SRCS ${MINER_MOC_HEADERS})

include_directories(${CMAKE_BINARY_DIR})

create_precompiled_header(stable stable.h)
add_executable(miner ${MINER_CXX_FILES} ${MINER_UIS_H} ${MINER_MOC_SRCS} ${MINER_RC_SRCS})
target_link_libraries(miner glpk ${Qt5Core_LIBRARIES} ${Qt5Widgets_LIBRARIES} -lpthread)
use_precompiled_header(miner stable)
