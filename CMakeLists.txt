cmake_minimum_required(VERSION 3.31)
project(NuitkaStudio VERSION 1.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/forms)

set(CMAKE_PREFIX_PATH "D:/Develop/QtDev/Qt5.14.2/5.14.2/mingw73_64")

# not Windows
if (NOT WIN32)
    message(FATAL_ERROR "Error: The project can only be uses on Windows OS now.")
endif ()

# set app info
set(AUTHOR "Redrch")
string(TIMESTAMP BUILD_TIME "%Y-%m-%d %H:%M:%S")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/app_config.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/app_config.h
        @ONLY)

include_directories(${CMAKE_CURRENT_BINARY_DIR})

# include spdlog lib
if (NOT DEFINED VCPKG_TARGET_TRIPLET)
    set(VCPKG_TARGET_TRIPLET "x64-mingw-dynamic" CACHE STRING "vcpkg triplet")
endif ()

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
        src/ui/mainwindow.cpp
        src/ui/mainwindow.h
        src/utils/config.cpp
        src/utils/config.h
        src/ui/export_datalist_window.cpp
        src/ui/export_datalist_window.h
        src/utils/logger.cpp
        src/utils/logger.h
        src/ui/about_window.cpp
        src/ui/about_window.h
        src/app_config.h.in
        src/ui/new_project_window.cpp
        src/ui/new_project_window.h
        src/utils/project_config.cpp
        src/utils/project_config.h
        src/utils/data_structs.h
        src/utils/utils.cpp
        src/utils/utils.h
)
set(RESOURCES
        resource.qrc
        logo.rc
        forms/export_datalist_window.ui
        forms/mainwindow.ui
        forms/about_window.ui
        forms/new_project_window.ui
        licenses/FmtLicense
        licenses/SpdlogLicense
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
    endif ()
endif ()

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

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

if(ENABLE_ASAN)
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # try to find libasan in the toolchain link dirs
        find_library(ASAN_LIB asan
                PATHS ${CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES}
                NO_DEFAULT_PATH
        )

        # try common clang runtime name on mingw/clang
        if(NOT ASAN_LIB AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            find_library(ASAN_LIB clang_rt.asan-x86_64
                    PATHS ${CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES}
                    NO_DEFAULT_PATH
            )
        endif()

        if(ASAN_LIB)
            message(STATUS "Enabling AddressSanitizer (found ${ASAN_LIB})")
            set(ASAN_COMMON_FLAGS -fsanitize=address -fno-omit-frame-pointer -g -O1)
            target_compile_options(NuitkaStudio PRIVATE ${ASAN_COMMON_FLAGS})
            target_link_options(NuitkaStudio PRIVATE -fsanitize=address)
        else()
            message(WARNING "ASan requested but no libasan found in the toolchain. Disable ENABLE_ASAN or switch to a clang toolchain / WSL.")
        endif()
    else()
        message(WARNING "ASan requires gcc/clang. On MSVC use clang-cl or other tools.")
    endif()
endif()

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