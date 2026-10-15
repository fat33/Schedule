cmake_minimum_required(VERSION 3.16)

project(Schedule
    VERSION 1.1.0
    DESCRIPTION "Class timetable editor"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(SCHEDULE_BUILD_TESTS "Build unit tests" ON)
option(SCHEDULE_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

# Qt 6 is preferred, Qt 5.15 is supported as a fallback.
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core)
if(QT_VERSION_MAJOR EQUAL 5 AND QT_VERSION VERSION_LESS 5.15)
    message(FATAL_ERROR "Qt 5.15 or newer is required, found ${QT_VERSION}")
endif()
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets PrintSupport LinguistTools)
message(STATUS "Using Qt ${QT_VERSION}")

# --- Common compile options --------------------------------------------------
add_library(schedule_options INTERFACE)
target_compile_definitions(schedule_options INTERFACE
    QT_NO_CAST_FROM_ASCII
    QT_NO_CAST_TO_ASCII
    QT_NO_URL_CAST_FROM_STRING
    QT_USE_QSTRINGBUILDER
    QT_DISABLE_DEPRECATED_BEFORE=0x050F00)
if(MSVC)
    target_compile_options(schedule_options INTERFACE /W4 /permissive- /utf-8)
    if(SCHEDULE_WARNINGS_AS_ERRORS)
        target_compile_options(schedule_options INTERFACE /WX)
    endif()
else()
    target_compile_options(schedule_options INTERFACE -Wall -Wextra -Wpedantic -Wshadow)
    if(SCHEDULE_WARNINGS_AS_ERRORS)
        target_compile_options(schedule_options INTERFACE -Werror)
    endif()
endif()

# --- Third-party: hierarchical header view -----------------------------------
add_library(hierarchicalheaderview STATIC
    third_party/hierarchicalheaderview/hierarchicalheaderview.cpp
    third_party/hierarchicalheaderview/hierarchicalheaderview.h)
target_include_directories(hierarchicalheaderview PUBLIC third_party/hierarchicalheaderview)
target_link_libraries(hierarchicalheaderview PUBLIC Qt${QT_VERSION_MAJOR}::Widgets)

# --- Core: model and export (no main window, unit-tested) --------------------
add_library(schedule_core STATIC
    src/htmlexport.cpp
    src/htmlexport.h
    src/schedulemodel.cpp
    src/schedulemodel.h
    src/timetable.h)
target_include_directories(schedule_core PUBLIC src)
target_link_libraries(schedule_core
    PUBLIC Qt${QT_VERSION_MAJOR}::Gui hierarchicalheaderview
    PRIVATE schedule_options)

# --- Application -------------------------------------------------------------
set(APP_SOURCES
    src/comboboxdelegate.cpp
    src/comboboxdelegate.h
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/mainwindow.ui
    resources/resources.qrc)

if(WIN32)
    configure_file(resources/schedule.rc.in ${CMAKE_CURRENT_BINARY_DIR}/schedule.rc @ONLY)
    list(APPEND APP_SOURCES ${CMAKE_CURRENT_BINARY_DIR}/schedule.rc)
endif()

add_executable(schedule WIN32 MACOSX_BUNDLE ${APP_SOURCES})
target_compile_definitions(schedule PRIVATE SCHEDULE_VERSION="${PROJECT_VERSION}")
target_link_libraries(schedule PRIVATE
    schedule_core
    schedule_options
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::PrintSupport)
set_target_properties(schedule PROPERTIES
    MACOSX_BUNDLE_BUNDLE_NAME "Schedule"
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})

# --- Translations ------------------------------------------------------------
set(TS_FILES translations/schedule_ru.ts)
if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_translations(schedule
        TS_FILES ${TS_FILES}
        SOURCES ${APP_SOURCES} src/htmlexport.cpp src/schedulemodel.cpp
        RESOURCE_PREFIX "/i18n")
else()
    qt5_add_translation(QM_FILES ${TS_FILES})
    set(I18N_QRC ${CMAKE_CURRENT_BINARY_DIR}/i18n.qrc)
    file(WRITE ${I18N_QRC} "<RCC><qresource prefix=\"/i18n\">\n")
    foreach(qm ${QM_FILES})
        get_filename_component(qm_name ${qm} NAME)
        file(APPEND ${I18N_QRC} "<file alias=\"${qm_name}\">${qm}</file>\n")
    endforeach()
    file(APPEND ${I18N_QRC} "</qresource></RCC>\n")
    target_sources(schedule PRIVATE ${I18N_QRC} ${QM_FILES})
endif()

# --- Install -----------------------------------------------------------------
include(GNUInstallDirs)
install(TARGETS schedule
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

# --- Tests -------------------------------------------------------------------
if(SCHEDULE_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
