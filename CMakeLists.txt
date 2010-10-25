project(Knights)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules ${CMAKE_MODULE_PATH})
find_package(KDE4 REQUIRED)
find_package(KdeGames REQUIRED)
include (KDE4Defaults)

set(CMAKE_CXX_FLAGS_PROFILE "-fprofile-arcs -ftest-coverage -pg")
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

include_directories( ${KDE4_INCLUDES} ${QT_INCLUDES} )


add_subdirectory( src )
add_subdirectory( icons )
add_subdirectory( themes )
add_subdirectory( doc )
