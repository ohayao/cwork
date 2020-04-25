# 为了gattlib的相关变量
include(CrossCompilation.cmake)

# project(gattlib)

option(GATTLIB_SHARED_LIB "Build GattLib as a shared library" YES)
option(GATTLIB_BUILD_DOCS "Build GattLib docs" NO)

find_package(PkgConfig REQUIRED)

# Define 'Debug' as the default build type
if (NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()

# Show all the warnings
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall")
# Expose 'gattlib.h' to all sub-directories
add_subdirectory(dbus)

# Add the Gattlib build directory for the examples
link_directories(${PROJECT_BINARY_DIR}/dbus)


# Generate pkg-config file before building the examples
#configure_file(dbus/gattlib.pc.in ${PROJECT_BINARY_DIR}/gattlib.pc @ONLY)

# Add the build directory to PKG_CONFIG_PATH
#set(ENV{PKG_CONFIG_PATH} "${PROJECT_BINARY_DIR}:$ENV{PKG_CONFIG_PATH}")


#
# Packaging
#
#set(CPACK_PACKAGE_INSTALL_DIRECTORY /usr CACHE STRING "Install directory (default: /usr).")
#if (ENV{TRAVIS_TAG} AND (NOT "ENV{TRAVIS_TAG}" STREQUAL "dev"))
#  message("Package Gattlib for tagged version $ENV{TRAVIS_TAG}")

  # Transform 'v0.3' into '0.3' and 'v0.3-rc1' into '0.3-rc1'
#  string(REGEX REPLACE "v([0-9]+).([0-9]+)(.*)" "\\1.\\2\\3" CPACK_PACKAGE_VERSION $ENV{TRAVIS_TAG})
#else()
#  set(CPACK_PACKAGE_VERSION 0.3-dev)
#  message("Package Gattlib for development version $ENV{CPACK_PACKAGE_VERSION}")
#endif()
#set(CPACK_PACKAGE_CONTACT "Olivier Martin <olivier@labapart.com>")
#set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Library to access GATT information from Bluetooth Low Energy (BLE) devices")
#set(CPACK_PACKAGE_FILE_NAME "${CMAKE_PROJECT_NAME}_${CPACK_PACKAGE_VERSION}_${CPACK_PACKAGE_ARCHITECTURE}")

#
# Debian, RPM and ZIP packages
#
#if (CMAKE_SYSROOT)
  # CPack does like RPM package in a cross-toolchain context
#  set(CPACK_GENERATOR "DEB;ZIP")
#else()
#  set(CPACK_GENERATOR "DEB;RPM;ZIP")
#endif()

#if (NOT CMAKE_SYSROOT)
# Detect platform architecture to use it for the Debian package
#  execute_process(COMMAND dpkg --print-architecture OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE OUTPUT_QUIET)
#endif()

#set(CPACK_DEBIAN_PACKAGE_DEPENDS "libglib2.0-0")

# Bluez DBus API changed from v5.40
#if (GATTLIB_DBUS AND (BLUEZ_VERSION_MAJOR EQUAL 5) AND (BLUEZ_VERSION_MINOR GREATER 39))
#  set(CPACK_DEBIAN_PACKAGE_DEPENDS "${CPACK_DEBIAN_PACKAGE_DEPENDS}, bluez (>= 5.40)")
#endif()

#
# List of file to install
#
#include(GNUInstallDirs)
#install(FILES include/gattlib.h DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
#install(FILES ${PROJECT_BINARY_DIR}/gattlib.pc DESTINATION ${CMAKE_INSTALL_LIBDIR}/pkgconfig)

#include(CPack)
