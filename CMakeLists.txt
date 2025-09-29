cmake_minimum_required(VERSION 3.16)

project(UnityTemplateBuilder VERSION 0.1 LANGUAGES CXX)

set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "")
set(ENABLE_TAR OFF CACHE INTERNAL "")
set(ENABLE_CPIO OFF CACHE INTERNAL "")
set(ENABLE_CAT OFF CACHE INTERNAL "")
set(ENABLE_TEST OFF CACHE INTERNAL "")

add_subdirectory(thirdparty/libarchive)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Core)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Core)

set(PROJECT_SOURCES
        main.cpp
        mainwindow.cpp
        mainwindow.h
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(UnityTemplateBuilder
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    if(ANDROID)
        add_library(UnityTemplateBuilder SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(UnityTemplateBuilder
            ${PROJECT_SOURCES}
        )
    endif()
endif()

target_link_libraries(UnityTemplateBuilder
    PRIVATE
        Qt${QT_VERSION_MAJOR}::Widgets
        Qt${QT_VERSION_MAJOR}::Core
        archive_static
)

target_include_directories(UnityTemplateBuilder PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libarchive/libarchive
    ${CMAKE_CURRENT_SOURCE_DIR}/thirdparty/libarchive/libarchive/include
)

if(${QT_VERSION} VERSION_LESS 6.1.0)
  set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.UnityTemplateInstaller)
endif()
set_target_properties(UnityTemplateBuilder PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)
install(TARGETS UnityTemplateBuilder
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(UnityTemplateBuilder)
endif()
