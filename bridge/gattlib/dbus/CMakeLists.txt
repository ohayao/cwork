
cmake_minimum_required(VERSION 3.0)
find_package(PkgConfig REQUIRED)

# Added Glib support to ensure we have 'gdbus-codegen'
pkg_search_module(GLIB REQUIRED glib-2.0)

# For DBus support
pkg_search_module(GIO_UNIX REQUIRED gio-unix-2.0)

# Needed by 'bluez5/lib/uuid.c'
pkg_search_module(BLUEZ REQUIRED bluez)

message("Build DBus gattlib for Bluez v${BLUEZ_VERSION_MAJOR}.${BLUEZ_VERSION_MINOR}")
set(DBUS_BLUEZ_API dbus-bluez-v5.48)

# GATTLIB_ROOT_DIR
# message(${GATTLIB_ROOT_DIR})
add_custom_command(OUTPUT ${GATTLIB_ROOT_DIR}/dbus/${DBUS_BLUEZ_API}/org-bluez-adaptater1.c
                   COMMAND gdbus-codegen --interface-prefix org.bluez.Adapter1. --generate-c-code ${GATTLIB_ROOT_DIR}/dbus/${DBUS_BLUEZ_API}/org-bluez-adaptater1 ${CMAKE_CURRENT_SOURCE_DIR}/${DBUS_BLUEZ_API}/org.bluez.Adapter1.xml
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${DBUS_BLUEZ_API}/org.bluez.Adapter1.xml
                   COMMENT "Generate D-Bus 'org.bluez.Adapter1.xml'"
                   )

add_custom_command(OUTPUT ${GATTLIB_ROOT_DIR}/dbus/${DBUS_BLUEZ_API}/org-bluez-device1.c
                   COMMAND gdbus-codegen --interface-prefix org.bluez.Device1. --generate-c-code ${GATTLIB_ROOT_DIR}/dbus/${DBUS_BLUEZ_API}/org-bluez-device1 ${CMAKE_CURRENT_SOURCE_DIR}/${DBUS_BLUEZ_API}/org.bluez.Device1.xml
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${DBUS_BLUEZ_API}/org.bluez.Device1.xml
                   COMMENT "Generate D-Bus 'org.bluez.Device1.xml'"
                   )

add_custom_command(OUTPUT ${GATTLIB_ROOT_DIR}/dbus/${DBUS_BLUEZ_API}/org-bluez-gattservice1.c
                   COMMAND gdbus-codegen --interface-prefix org.bluez.GattService1. --generate-c-code ${GATTLIB_ROOT_DIR}/dbus/${DBUS_BLUEZ_API}/org-bluez-gattservice1 ${CMAKE_CURRENT_SOURCE_DIR}/${DBUS_BLUEZ_API}/org.bluez.GattService1.xml
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${DBUS_BLUEZ_API}/org.bluez.GattService1.xml
                   COMMENT "Generate D-Bus 'org.bluez.GattService1.xml'"
                   )

add_custom_command(OUTPUT ${GATTLIB_ROOT_DIR}/dbus/${DBUS_BLUEZ_API}/org-bluez-gattcharacteristic1.c
                   COMMAND gdbus-codegen --interface-prefix org.bluez.Characteristic1. --generate-c-code ${GATTLIB_ROOT_DIR}/dbus/${DBUS_BLUEZ_API}/org-bluez-gattcharacteristic1 ${CMAKE_CURRENT_SOURCE_DIR}/${DBUS_BLUEZ_API}/org.bluez.GattCharacteristic1.xml
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${DBUS_BLUEZ_API}/org.bluez.GattCharacteristic1.xml
                   COMMENT "Generate D-Bus 'org.bluez.GattCharacteristic1.xml'"
                   )

add_custom_command(OUTPUT ${GATTLIB_ROOT_DIR}/dbus/${DBUS_BLUEZ_API}/org-bluez-gattdescriptor1.c
                   COMMAND gdbus-codegen --interface-prefix org.bluez.Descriptor1. --generate-c-code ${GATTLIB_ROOT_DIR}/dbus/${DBUS_BLUEZ_API}/org-bluez-gattdescriptor1 ${CMAKE_CURRENT_SOURCE_DIR}/${DBUS_BLUEZ_API}/org.bluez.GattDescriptor1.xml
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${DBUS_BLUEZ_API}/org.bluez.GattDescriptor1.xml
                   COMMENT "Generate D-Bus 'org.bluez.GattDescriptor1.xml'"
                   )

add_custom_command(OUTPUT ${GATTLIB_ROOT_DIR}/dbus/${DBUS_BLUEZ_API}/org-bluez-battery1.c
                   COMMAND gdbus-codegen --interface-prefix org.bluez.Battery1. --generate-c-code ${GATTLIB_ROOT_DIR}/dbus/${DBUS_BLUEZ_API}/org-bluez-battery1 ${CMAKE_CURRENT_SOURCE_DIR}/${DBUS_BLUEZ_API}/org.bluez.Battery1.xml
                   DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${DBUS_BLUEZ_API}/org.bluez.Battery1.xml
                   COMMENT "Generate D-Bus 'org.bluez.Battery1.xml'"
                   )


set(gattlib_SRCS gattlib.c
                 gattlib_adapter.c
                 gattlib_advertisement.c
                 gattlib_char.c
                 gattlib_stream.c
                 bluez5/lib/uuid.c
                 ${GATTLIB_ROOT_DIR}/common/gattlib_common.c
                 ${GATTLIB_ROOT_DIR}/common/gattlib_eddystone.c
                 ${GATTLIB_ROOT_DIR}/dbus/${DBUS_BLUEZ_API}/org-bluez-adaptater1.c
                 ${GATTLIB_ROOT_DIR}/dbus/${DBUS_BLUEZ_API}/org-bluez-device1.c
                 ${GATTLIB_ROOT_DIR}/dbus/${DBUS_BLUEZ_API}/org-bluez-gattcharacteristic1.c
                 ${GATTLIB_ROOT_DIR}/dbus/${DBUS_BLUEZ_API}/org-bluez-gattdescriptor1.c
                 ${GATTLIB_ROOT_DIR}/dbus/${DBUS_BLUEZ_API}/org-bluez-gattservice1.c)


list(APPEND gattlib_SRCS ${GATTLIB_ROOT_DIR}/dbus/${DBUS_BLUEZ_API}/org-bluez-battery1.c)


set(gattlib_LIBS ${GLIB_LDFLAGS} ${GIO_UNIX_LDFLAGS})

# Gattlib
add_library(gattlib SHARED ${gattlib_SRCS})
target_link_libraries(gattlib ${gattlib_LIBS})

include(GNUInstallDirs)
# ${CMAKE_INSTALL_LIBDIR} : lib
install(TARGETS gattlib LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})