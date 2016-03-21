project(Knights)

cmake_minimum_required(VERSION 2.8.12)
set (QT_MIN_VERSION "5.5.0")

find_package(ECM 1.7.0 REQUIRED CONFIG)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wundef -Wpointer-arith -Wcast-align -Wunreachable-code -fno-omit-frame-pointer -fstack-protector -fno-exceptions")

#set(CMAKE_CXX_FLAGS_PROFILE "-fprofile-arcs -ftest-coverage -pg")

include(FeatureSummary)
find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS
   Concurrent
   Core
   Gui
   Widgets
   Svg
)

find_package(Qt5TextToSpeech ${QT_REQUIRED_VERSION} QUIET)
set_package_properties(Qt5TextToSpeech PROPERTIES
   PURPOSE "Speaking of opponents moves")
if (Qt5TextToSpeech_FOUND)
   add_definitions(-DHAVE_SPEECH)
endif ()

find_package(KF5 REQUIRED COMPONENTS
  DBusAddons
  ConfigWidgets
  TextWidgets
  XmlGui
  KIO
  Plotting
  DocTools
  Wallet
  Plasma
)

find_package(KF5KDEGames 4.9.0 REQUIRED)

include(FeatureSummary)
include(ECMAddAppIcon)
include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)
include(KDEInstallDirs)
include(KDECompilerSettings)
include(KDECMakeSettings)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII)


add_subdirectory( src )
add_subdirectory( icons )
add_subdirectory( themes )
add_subdirectory( doc )

