cmake_minimum_required(VERSION 3.8)

project(tinydi)

option(TINYDI_BUILD_EXAMPLES "Configure CMake to build examples (or not)" ON)
option(TINYDI_BUILD_TESTS "Configure CMake to build tests (or not)" ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(TINYDI_INCLUDE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(TINYDI_HEADER_FILES ${TINYDI_INCLUDE_PATH}/tinydi/tinydi.h)

add_library(tinydi INTERFACE)
add_library(tinydi::tinydi ALIAS tinydi) # To match export
target_compile_features(tinydi INTERFACE cxx_std_11)
add_compile_options(-Wall)
add_compile_options(-Wextra)
add_compile_options(-Werror)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Weffc++ -Wold-style-cast" CACHE STRING "" FORCE)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)
target_include_directories(tinydi INTERFACE $<BUILD_INTERFACE:${TINYDI_INCLUDE_PATH}>
                                            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)

set(TINYDI_CMAKE_FILES_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/cmake/tinydi)
configure_package_config_file(
  ${CMAKE_CURRENT_LIST_DIR}/cmake/tinydi-config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/tinydi-config.cmake
  INSTALL_DESTINATION ${TINYDI_CMAKE_FILES_INSTALL_DIR})
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/tinydi-config-version.cmake VERSION 0.1.0
                                 COMPATIBILITY ExactVersion)

install(TARGETS tinydi EXPORT tinydi-targets PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT tinydi-targets NAMESPACE tinydi:: FILE tinydi-targets.cmake
        DESTINATION ${TINYDI_CMAKE_FILES_INSTALL_DIR})
install(FILES ${TINYDI_HEADER_FILES} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tinydi)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tinydi-config.cmake ${CMAKE_CURRENT_BINARY_DIR}/tinydi-config-version.cmake
        DESTINATION ${TINYDI_CMAKE_FILES_INSTALL_DIR})

if(TINYDI_BUILD_EXAMPLES)
  add_subdirectory(example)
endif()

if(TINYDI_BUILD_TESTS)
  add_subdirectory(test)
endif()

set(CPACK_CMAKE_GENERATOR "Unix Makefiles")
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "libtinydi")
set(CPACK_PACKAGE_VENDOR "Dominik Thalhammer")
set(CPACK_PACKAGE_CONTACT "dominik@thalhammer.it")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "TinyDI is a tiny single header dependency injection for c++")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/Thalhammer/tinydi")
set(CPACK_PACKAGE_VERSION_MAJOR 0)
set(CPACK_PACKAGE_VERSION_MINOR 1)
set(CPACK_PACKAGE_VERSION_PATCH 0)
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Dominik Thalhammer <dominik@thalhammer.it>")
include(CPack)