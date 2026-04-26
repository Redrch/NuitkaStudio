cmake_minimum_required(VERSION 3.31)
project(NuitkaStudio VERSION 1.4.0.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOUIC_SEARCH_PATHS ${CMAKE_CURRENT_SOURCE_DIR}/forms)

# not Windows
if (NOT WIN32)
    message(FATAL_ERROR "Error: The project can only be uses on Windows OS now.")
endif ()

# set app info
string(TIMESTAMP BUILD_TIME "%Y-%m-%d %H:%M:%S")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/app_config.h.in
        ${CMAKE_CURRENT_BINARY_DIR}/app_config.h
        @ONLY)

# find packages
find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

find_package(BZip2 REQUIRED)
find_package(spdlog CONFIG REQUIRED)

set(THIRD_PARTY_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
if(NOT EXISTS "${THIRD_PARTY_INCLUDE_DIR}")
    file(MAKE_DIRECTORY "${THIRD_PARTY_INCLUDE_DIR}")
    MESSAGE(WARNING "Not found include dir.")
endif()
# Quazip
add_library(QuaZip::Quazip SHARED IMPORTED)
set_target_properties(QuaZip::Quazip PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${THIRD_PARTY_INCLUDE_DIR}"
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/libquazip1-qt5.dll"
        IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/libquazip1-qt5.dll"
)
# ElaWidgetTools
add_library(ElaWidgetTools::ElaWidgetTools SHARED IMPORTED)
set_target_properties(ElaWidgetTools::ElaWidgetTools PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${THIRD_PARTY_INCLUDE_DIR}/ElaWidgetTools"
        IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/ElaWidgetTools.dll"
        IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/lib/libElaWidgetTools.dll.a"
)

set(SOURCES
        src/main.cpp
        src/ui/mainwindow.cpp
        src/ui/mainwindow.h
        src/utils/config.cpp
        src/utils/config.h
        src/utils/logger.cpp
        src/utils/logger.h
        src/ui/about_window.cpp
        src/ui/about_window.h
        src/app_config.h.in
        src/utils/project_config.cpp
        src/utils/project_config.h
        src/types/data_structs.h
        src/utils/utils.cpp
        src/utils/utils.h
        src/types/project_config_type.h
        src/global/project_config_manager.cpp
        src/global/project_config_manager.h
        src/utils/compress.cpp
        src/utils/compress.h
        src/types/simname.h
        src/types/global_data.cpp
        src/types/global_data.h
        src/types/constants.h
        src/types/color.h
        src/ui/float_button.cpp
        src/ui/float_button.h
        src/utils/event_bus.cpp
        src/utils/event_bus.h
        src/ui/pack_page.cpp
        src/ui/pack_page.h
        src/ui/settings_page.cpp
        src/ui/settings_page.h
        src/global/application.cpp
        src/global/application.h
        src/global/update_clock.cpp
        src/global/update_clock.h
)
set(RESOURCES
        resource.qrc
        translations.qrc
        logo.rc
        forms/mainwindow.ui
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
        BZip2::BZip2
        QuaZip::Quazip
        ElaWidgetTools::ElaWidgetTools
        dwmapi
)

target_include_directories(NuitkaStudio PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_BINARY_DIR}
)

option(ENABLE_ASAN "Enable AddressSanitizer" OFF)

if (ENABLE_ASAN)
    if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        # try to find libasan in the toolchain link dirs
        find_library(ASAN_LIB asan
                PATHS ${CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES}
                NO_DEFAULT_PATH
        )

        # try common clang runtime name on mingw/clang
        if (NOT ASAN_LIB AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            find_library(ASAN_LIB clang_rt.asan-x86_64
                    PATHS ${CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES}
                    NO_DEFAULT_PATH
            )
        endif ()

        if (ASAN_LIB)
            message(STATUS "Enabling AddressSanitizer (found ${ASAN_LIB})")
            set(ASAN_COMMON_FLAGS -fsanitize=address -fno-omit-frame-pointer -g -O1)
            target_compile_options(NuitkaStudio PRIVATE ${ASAN_COMMON_FLAGS})
            target_link_options(NuitkaStudio PRIVATE -fsanitize=address)
        else ()
            message(WARNING "ASan requested but no libasan found in the toolchain. Disable ENABLE_ASAN or switch to a clang toolchain / WSL.")
        endif ()
    else ()
        message(WARNING "ASan requires gcc/clang. On MSVC use clang-cl or other tools.")
    endif ()
endif ()

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

# install
# clear install dir
install(CODE "
    message(STATUS \"Cleaning install directory: \${CMAKE_INSTALL_PREFIX}\")
    file(REMOVE_RECURSE \"\${CMAKE_INSTALL_PREFIX}\")
    file(MAKE_DIRECTORY \"\${CMAKE_INSTALL_PREFIX}\")
")
# set install dir
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/Install" CACHE PATH "Install Prefix" FORCE)
endif()

# install main file
install(TARGETS NuitkaStudio
        RUNTIME DESTINATION .
        COMPONENT Runtime
)

# install third-party libs
install(FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/ElaWidgetTools.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/libquazip1-qt5.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/bz2.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/lib/zlib1.dll"
        DESTINATION .
        COMPONENT Runtime
)
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/libspdlogd.dll"
        "${CMAKE_CURRENT_BINARY_DIR}/libfmtd.dll"
        DESTINATION .
        COMPONENT Runtime
    )
else ()
    install(FILES
            "${CMAKE_CURRENT_BINARY_DIR}/libspdlog.dll"
            "${CMAKE_CURRENT_BINARY_DIR}/libfmt.dll"
            DESTINATION .
            COMPONENT Runtime
    )
endif ()

# windeployqt
if(WIN32)
    get_target_property(_qt_core_path Qt5::Core LOCATION)
    get_filename_component(_qt_bin_dir "${_qt_core_path}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")

    if(WINDEPLOYQT_EXECUTABLE)
        install(CODE "
            message(STATUS \"Running windeployqt...\")
            execute_process(
                COMMAND \"${WINDEPLOYQT_EXECUTABLE}\"
                --no-compiler-runtime
                --no-svg
                --no-opengl-sw
                --no-translations
                \"\${CMAKE_INSTALL_PREFIX}/NuitkaStudio.exe\"
            )
        " COMPONENT Runtime)
    endif()
endif()

# CPack
# Config
set(CPACK_PACKAGE_NAME "NuitkaStudio")
set(CPACK_PACKAGE_VENDOR "Redrch")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Nuitka Studio Setup")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "NuitkaStudio")
set(CPACK_MONOLITHIC_INSTALL 1)
set(CPACK_GENERATOR "ZIP;NSIS")

# NSIS
if(WIN32)
     set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/NuitkaStudioSetupLogo.ico")

    set(CPACK_NSIS_PACKAGE_NAME "Nuitka Studio")
    set(CPACK_NSIS_DISPLAY_NAME "Nuitka Studio ${PROJECT_VERSION}")
    set(CPACK_NSIS_HELP_LINK "https://github.com/Redrch/NuitkaStudio")

    # Shortcut
    set(CPACK_NSIS_CREATE_ICONS_EXTRA "
        CreateShortCut '$DESKTOP\\Nuitka Studio.lnk' '$INSTDIR\\NuitkaStudio.exe'
    ")
endif()

# include CPack module
include(CPack)