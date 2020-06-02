cmake_minimum_required(VERSION 3.4)

project(bridge)
set(CMAKE_BUILD_TYPE Debug)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)

include_directories(.)
include_directories(${GIO_UNIX_INCLUDE_DIRS})
#include_directories(${GLIB_INCLUDE_DIRS})
# 定义一个变量
set(GATTLIB_ROOT_DIR ${CMAKE_SOURCE_DIR}/bridge/gattlib)

#include_directories(. ${GATTLIB_ROOT_DIR}/common ${CMAKE_CURRENT_BINARY_DIR} ${GIO_UNIX_INCLUDE_DIRS} ${BLUEZ_INCLUDE_DIRS})

# include_directories(${GATTLIB_ROOT_DIR}/common ${CMAKE_CURRENT_BINARY_DIR} ${GIO_UNIX_INCLUDE_DIRS} ${BLUEZ_INCLUDE_DIRS})


add_subdirectory("bridge")