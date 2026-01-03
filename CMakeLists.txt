cmake_minimum_required (VERSION 3.16 FATAL_ERROR)

# Release Service version, managed by release script
set (RELEASE_SERVICE_VERSION_MAJOR "25")
set (RELEASE_SERVICE_VERSION_MINOR "12")
set (RELEASE_SERVICE_VERSION_MICRO "1")
set (RELEASE_SERVICE_COMPACT_VERSION "${RELEASE_SERVICE_VERSION_MAJOR}${RELEASE_SERVICE_VERSION_MINOR}${RELEASE_SERVICE_VERSION_MICRO}")

# Bump KNIGHTS_BASE_VERSION once new features are added
set(KNIGHTS_BASE_VERSION "2.6")
set(KNIGHTS_VERSION "${KNIGHTS_BASE_VERSION}.${RELEASE_SERVICE_COMPACT_VERSION}")

project(Knights VERSION ${KNIGHTS_VERSION})

set(QT_MIN_VERSION "6.5.0")
set(KF_MIN_VERSION "6.0.0")
set(PLASMA_MIN_VERSION "6.0.0")

find_package(ECM ${KF_MIN_VERSION} REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake ${ECM_MODULE_PATH})

if(CMAKE_COMPILER_IS_GNUCXX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wpointer-arith -Wunreachable-code -fno-omit-frame-pointer -fstack-protector")
endif(CMAKE_COMPILER_IS_GNUCXX)

#set(CMAKE_CXX_FLAGS_PROFILE "-fprofile-arcs -ftest-coverage -pg")

include(KDEInstallDirs)
include(KDECMakeSettings)
include(KDECompilerSettings NO_POLICY_SCOPE)

include(ECMAddAppIcon)
include(ECMInstallIcons)
include(ECMQtDeclareLoggingCategory)
include(ECMSetupVersion)
include(ECMDeprecationSettings)
include(FeatureSummary)

include(InternalMacros)

find_package(Qt6 ${QT_MIN_VERSION} REQUIRED COMPONENTS
    Concurrent
    Core
    Gui
    Widgets
    Svg
)

find_package(Qt6TextToSpeech ${QT_MIN_VERSION} QUIET)
set_package_properties(Qt6TextToSpeech PROPERTIES
    PURPOSE "Speaking of opponents moves"
)
if (Qt6TextToSpeech_FOUND)
    add_definitions(-DHAVE_SPEECH)
endif ()

find_package(KF6 ${KF_MIN_VERSION} REQUIRED COMPONENTS
    CoreAddons
    DBusAddons
    ConfigWidgets
    Crash
    I18n
    TextWidgets
    XmlGui
    KIO
    Plotting
    DocTools
    Wallet
    Svg
)

find_package(Plasma ${PLASMA_MIN_VERSION} REQUIRED)
set_package_properties(Plasma PROPERTIES
    TYPE RUNTIME
    PURPOSE "clock svg (part of the desktop theme)"
)

find_package(KDEGames6 6.0.0 REQUIRED)

ecm_set_disabled_deprecation_versions(
    QT 6.9.0
    KF 6.5
    KDEGAMES 6.0
)

add_subdirectory(doc)
add_subdirectory(icons)
add_subdirectory(sounds)
add_subdirectory(src)
add_subdirectory(themes)

ecm_qt_install_logging_categories(
    EXPORT KNIGHTS
    FILE knights.categories
    DESTINATION ${KDE_INSTALL_LOGGINGCATEGORIESDIR}
)

ki18n_install(po)
kdoctools_install(po)

feature_summary(WHAT ALL FATAL_ON_MISSING_REQUIRED_PACKAGES)
