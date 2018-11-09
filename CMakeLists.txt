project(Knights)

cmake_minimum_required (VERSION 3.2.0 FATAL_ERROR)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set (QT_MIN_VERSION "5.7.0")
set (KF5_MIN_VERSION "5.30.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED CONFIG)

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

find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
  DBusAddons
  ConfigWidgets
  Crash
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
include(KDEFrameworkCompilerSettings NO_POLICY_SCOPE)
include(KDECMakeSettings)

add_definitions(-DQT_USE_FAST_CONCATENATION -DQT_USE_FAST_OPERATOR_PLUS)
add_definitions(-DQT_NO_CAST_FROM_ASCII -DQT_NO_CAST_TO_ASCII)
add_definitions(-DQT_DISABLE_DEPRECATED_BEFORE=0x50600)

add_subdirectory(doc)
add_subdirectory(icons)
add_subdirectory(sounds)
add_subdirectory(src)
add_subdirectory(themes)

install( FILES knights.categories DESTINATION ${KDE_INSTALL_CONFDIR} )

feature_summary(WHAT ALL
                INCLUDE_QUIET_PACKAGES
                FATAL_ON_MISSING_REQUIRED_PACKAGES)

