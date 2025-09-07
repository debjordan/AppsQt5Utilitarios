cmake_minimum_required(VERSION 3.10)

project(SSHTerminal
    VERSION 1.0.0
    DESCRIPTION "SSH Terminal integrado com interface Qt5"
    LANGUAGES CXX)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt5 Configuration
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Find Qt5 packages
find_package(Qt5 REQUIRED COMPONENTS Core Widgets)

# Source files
set(SOURCES
    main.cpp
    sshterminal.cpp
)

set(HEADERS
    sshterminal.h
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Link Qt5 libraries
target_link_libraries(${PROJECT_NAME}
    Qt5::Core
    Qt5::Widgets
)

# Compiler specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -pedantic
    )
endif()

# Install configuration
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Create desktop entry
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/sshterminal.desktop.in
    ${CMAKE_CURRENT_BINARY_DIR}/sshterminal.desktop
    @ONLY
)

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/sshterminal.desktop
    DESTINATION share/applications
)

# Print build information
message(STATUS "Building ${PROJECT_NAME} v${PROJECT_VERSION}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Package configuration
set(CPACK_PACKAGE_NAME "sshterminal")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "SSH Terminal integrado com Qt5")
set(CPACK_PACKAGE_VENDOR "AppsQt5Utilitarios")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "AppsQt5Utilitarios")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5core5a, libqt5widgets5, sshpass")

include(CPack)
