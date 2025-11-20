cmake_minimum_required(VERSION 3.16)

# Set Version, Release need cmake
set(PROJECT_VERSION_MAJOR 0)
set(PROJECT_VERSION_MINOR 0)

# use git version as library version
find_package(Git QUIET)
if (Git_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../.git")
    execute_process(
            COMMAND ${GIT_EXECUTABLE} rev-list --count HEAD
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            OUTPUT_VARIABLE _git_version
            OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    execute_process(
            COMMAND ${GIT_EXECUTABLE} log -1 --pretty=format:%h
            OUTPUT_VARIABLE PROJECT_GIT_HASH
            OUTPUT_STRIP_TRAILING_WHITESPACE
            ERROR_QUIET
            WORKING_DIRECTORY
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
else ()
    set(_git_version 0)
    set(PROJECT_GIT_HASH "")
endif ()

project(YFEL VERSION 0.0.${_git_version} LANGUAGES CXX)

option(CMAKE_QT_CLION "Using CLion develop" OFF)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Use modern CMake with Qt
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

# Enable automatic processing of Qt files
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR})

# Use folders for IDEs that support them
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

configure_file(
        "${PROJECT_SOURCE_DIR}/config.h.in"
        "${PROJECT_BINARY_DIR}/yfel_config.h"
)

if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    enable_language("RC")
    configure_file(
            "${PROJECT_SOURCE_DIR}/assets/YFEL.rc.in"
            "${PROJECT_BINARY_DIR}/YFEL.rc"
    )
    set(WIN32_RESOURCES ${PROJECT_BINARY_DIR}/YFEL.rc)
endif ()

# Find Qt libraries
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets LinguistTools Concurrent)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Widgets LinguistTools Concurrent)

set(I18N_TS_FILES "${PROJECT_SOURCE_DIR}/i18n/YFEL_zh_CN.ts")
set_source_files_properties(${I18N_TS_FILES} PROPERTIES OUTPUT_LOCATION "${CMAKE_CURRENT_BINARY_DIR}/i18n")

# Explicitly list all source files to ensure they are included
set(PROJECT_SOURCES
    # Main files
    main.cpp
    mainwindow.cpp
    mainwindow.h
    
    # Core functionality
    chipop.cpp
    chipop.h
    fel.cpp
    fel.h
    feldevice.cpp
    feldevice.h
    usb.cpp
    usb.h
    chip_status.cpp
    chip_status.h
    
    # SPI functionality
    spi.cpp
    spi.h
    spi_nand.cpp
    spi_nand.h
    spi_nor.cpp
    spi_nor.h
    
    # Utilities
    sha256.cpp
    sha256.h
    tpr13dialog.cpp
    tpr13dialog.h
    
    # Headers
    chips/chip_db.h
    chips/chip_type.h
    chips/chip_version.h
    chips/dram_header.h
    chips/d1.h
    chips/f1c100s.h
    chips/r528.h
    chips/v853.h
    exceptions.h
    macros.h
    spi_nand_info.h
    spi_nor_info.h
    spi_nor_sfdp.h
    utils.h
    x.h
    
    # Chip implementations
    chips/d1.cpp
    chips/f1c100s.cpp
    chips/r528.cpp
    chips/v853.cpp
    
    # QHexView library files
    lib/QHexView/QHexView.cpp
    lib/QHexView/QHexView.h
)

# UI files
set(UI_FILES
    mainwindow.ui
    tpr13dialog.ui
)

# Resource files
set(RESOURCE_FILES
    icon.qrc
)

# Note: ALL_SOURCES is kept for backward compatibility with Qt5
set(ALL_SOURCES
    ${PROJECT_SOURCES}
    ${UI_FILES}
    ${RESOURCE_FILES}
    ${I18N_TS_FILES}
    ${PROJECT_BINARY_DIR}/yfel_config.h
    ${WIN32_RESOURCES})

if (${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(YFEL ${ALL_SOURCES})
    
    # Translation handling for Qt6
    if(COMMAND qt6_create_translation)
        qt6_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${I18N_TS_FILES})
        set(qm_files ${QM_FILES})
    else()
        find_package(Qt${QT_VERSION_MAJOR} REQUIRED LinguistTools)
        qt6_add_lrelease(YFEL
                TS_FILES ${I18N_TS_FILES}
                QM_FILES_OUTPUT_VARIABLE qm_files)
    endif()
    
    if(qm_files)
        qt6_add_resources(YFEL "translations"
                PREFIX "/"
                BASE "${CMAKE_CURRENT_BINARY_DIR}"
                FILES "${qm_files}")
    endif()
else ()
    # Qt5 specific configuration
    if (ANDROID)
        add_library(YFEL SHARED ${ALL_SOURCES})
    else ()
        add_executable(YFEL ${ALL_SOURCES})
    endif ()
    qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR} ${I18N_TS_FILES})
endif ()

if (WIN32)
    # Windows platform: Use precompiled MinGW libusb static library
    target_link_libraries(YFEL PRIVATE 
        Qt${QT_VERSION_MAJOR}::Core 
        Qt${QT_VERSION_MAJOR}::Widgets 
        Qt${QT_VERSION_MAJOR}::Concurrent
        ${CMAKE_SOURCE_DIR}/lib/libusb/libusb-MinGW-x64/lib/libusb-1.0.a)
    target_include_directories(YFEL PRIVATE 
        ${CMAKE_SOURCE_DIR}/lib/libusb/libusb-MinGW-x64/include
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/QHexView
        ${CMAKE_CURRENT_BINARY_DIR})
elseif (UNIX)
    # Linux/Unix/macOS platform: Use pkg-config to find libusb-1.0
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
    
    target_link_libraries(YFEL PRIVATE 
        Qt${QT_VERSION_MAJOR}::Core 
        Qt${QT_VERSION_MAJOR}::Widgets 
        Qt${QT_VERSION_MAJOR}::Concurrent
        ${LIBUSB_LIBRARIES})
    target_include_directories(YFEL PRIVATE 
        ${LIBUSB_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/QHexView
        ${CMAKE_CURRENT_BINARY_DIR})
    target_link_directories(YFEL PRIVATE ${LIBUSB_LIBRARY_DIRS})
    target_compile_options(YFEL PRIVATE ${LIBUSB_CFLAGS_OTHER})
else()
    message(FATAL_ERROR "Unsupported platform")
endif ()

# Set target properties with modern CMake syntax
set_target_properties(YFEL PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER yfel.gloomyghost.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# Remove the qt_finalize_executable call since we're no longer using MANUAL_FINALIZATION

if (WIN32)
    if (CMAKE_QT_CLION)
        set(DEBUG_SUFFIX)
        if (MSVC AND CMAKE_BUILD_TYPE MATCHES "Debug")
            set(DEBUG_SUFFIX "d")
        endif ()
        set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
            if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
                set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
            endif ()
        endif ()
        if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E make_directory
                    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy
                    "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                    "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        endif ()
        foreach (QT_LIB Core Gui Widgets)
            add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy
                    "${QT_INSTALL_PATH}/bin/Qt${QT_VERSION_MAJOR}${QT_LIB}${DEBUG_SUFFIX}.dll"
                    "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
        endforeach (QT_LIB)
    endif ()
endif ()

# Add support for CMake 4.0 features and modern practices
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.15")
    # Enable CMake testing if needed
    include(CTest)
endif()

# Better organize source files in IDEs
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "Sources" FILES ${PROJECT_SOURCES})
source_group("UI Files" FILES mainwindow.ui tpr13dialog.ui)
source_group("Resources" FILES icon.qrc)
source_group("QHexView" FILES ${QHEXVIEW_SOURCES})

install(TARGETS YFEL
        RUNTIME DESTINATION ${CMAKE_INSTALL_PREFIX}/bin
        LIBRARY DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib
        BUNDLE DESTINATION .)

install(FILES ${PROJECT_BINARY_DIR}/yfel_config.h
        DESTINATION ${CMAKE_INSTALL_PREFIX}/include)

if(qm_files)
    install(FILES ${qm_files}
            DESTINATION ${CMAKE_INSTALL_PREFIX}/share/locale)
endif()

install(FILES ${PROJECT_SOURCE_DIR}/assets/applications/io.github.yuzukitsuru.yfel.desktop
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/applications)

install(FILES ${PROJECT_SOURCE_DIR}/assets/img/yfel.png
        DESTINATION ${CMAKE_INSTALL_PREFIX}/share/pixmaps)