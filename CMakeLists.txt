cmake_minimum_required(VERSION 2.8.9)
project(Knights)

find_package(KDE4 REQUIRED)
find_package(KDEGames REQUIRED)
include (KDE4Defaults)

set(CMAKE_CXX_FLAGS_PROFILE "-fprofile-arcs -ftest-coverage -pg")
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )
add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)

include_directories( ${KDE4_INCLUDES} ${QT_INCLUDES} ${KDEGAMES_INCLUDE_DIRS} )

add_subdirectory( src )
add_subdirectory( icons )
add_subdirectory( themes )
add_subdirectory( doc )

macro_display_feature_log()
