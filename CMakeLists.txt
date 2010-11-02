project(Knights)

find_package(KDE4 REQUIRED)
find_package(LibKDEGames REQUIRED)
include (KDE4Defaults)
# TODO: check for gnuchess

set(CMAKE_CXX_FLAGS_PROFILE "-fprofile-arcs -ftest-coverage -pg")
add_definitions( -DQT_NO_CAST_FROM_ASCII )
add_definitions( -DQT_NO_CAST_TO_ASCII )

include_directories( ${KDE4_INCLUDES} ${QT_INCLUDES} ${KDEGAMES_INCLUDE_DIRS} )


add_subdirectory( src )
add_subdirectory( icons )
add_subdirectory( themes )
add_subdirectory( doc )
