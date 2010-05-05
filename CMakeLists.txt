project(Knights)

find_package(KDE4 REQUIRED)
include (KDE4Defaults)

set(CMAKE_CXX_FLAGS_PROFILE "-fprofile-arcs -ftest-coverage -pg")

include_directories( ${KDE4_INCLUDES} ${QT_INCLUDES} )


add_subdirectory( src )
add_subdirectory( icons )
add_subdirectory( themes )
