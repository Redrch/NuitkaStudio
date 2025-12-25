cmake_minimum_required(VERSION 3.31)
project(NuitkaStudio)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(VERSION 1.0.0)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/forms)

set(CMAKE_PREFIX_PATH "D:/Develop/QtDev/Qt5.14.2/5.14.2/mingw73_64")

# not Windows
if (NOT WIN32)
    message(FATAL_ERROR "Error: The project can only be uses on Windows OS now.")
endif ()

# include spdlog lib
if(NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "x64-mingw-dynamic" CACHE STRING "vcpkg triplet")
endif()

set(_vcpkg_installed "${CMAKE_BINARY_DIR}/vcpkg_installed")

string(REPLACE "\\" "/" _vcpkg_installed "${_vcpkg_installed}/${VCPKG_TARGET_TRIPLET}")
set(spdlog_DIR "${_vcpkg_installed}/lib/cmake/spdlog" "${_vcpkg_installed}/${VCPKG_TARGET_TRIPLET}")
message(STATUS "spdlog_DIR -> ${spdlog_DIR}")

list(APPEND CMAKE_PREFIX_PATH
        "${_vcpkg_installed}/lib/cmake"
        "${_vcpkg_installed}/share"
)
message(STATUS "CMAKE_PREFIX_PATH -> ${CMAKE_PREFIX_PATH}")

find_package(spdlog CONFIG REQUIRED HINTS
        "${_vcpkg_installed}/lib/cmake/spdlog"
        "${_vcpkg_installed}/share/spdlog"
        "${_vcpkg_installed}/cmake"
)

find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

set(SOURCES
        src/main.cpp
        src/mainwindow.cpp
        src/mainwindow.h
        forms/mainwindow.ui
        src/config.cpp
        src/config.h
        src/export_datalist_window.cpp
        src/export_datalist_window.h
        forms/export_datalist_window.ui
        src/logger.cpp
        src/logger.h
)
set(RESOURCES
        resource.qrc
        logo.rc
)

add_executable(NuitkaStudio ${SOURCES} ${RESOURCES})

if (WIN32)
        # For multi-config generators (Visual Studio, etc.) set the WIN32 flag
        # only for the Release configuration using a generator expression.
        set_target_properties(NuitkaStudio PROPERTIES WIN32_EXECUTABLE "$<$<CONFIG:Release>:TRUE>")

        # For single-config generators that use CMAKE_BUILD_TYPE, enable immediately
        # when CMAKE_BUILD_TYPE is Release.
        if (DEFINED CMAKE_BUILD_TYPE AND CMAKE_BUILD_TYPE STREQUAL "Release")
                set_target_properties(NuitkaStudio PROPERTIES WIN32_EXECUTABLE TRUE)
        endif()
endif()

target_link_libraries(NuitkaStudio
        PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Widgets
        spdlog::spdlog
)

target_include_directories(NuitkaStudio PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}
)

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
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
                "${QT_INSTALL_PATH}/bin/Qt5${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()