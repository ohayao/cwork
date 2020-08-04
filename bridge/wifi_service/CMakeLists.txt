cmake_minimum_required(VERSION 3.4)

set(WIFI_DEPENDENCIES
  lock
  ign_lib
  ble
)

set(CMAKE_BUILD_TYPE Debug)
find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(GIO_UNIX REQUIRED gio-unix-2.0)
pkg_search_module(DBUS REQUIRED dbus-1)
include_directories(${GIO_UNIX_INCLUDE_DIRS})
include_directories(${GLIB_INCLUDE_DIRS})
include_directories(${DBUS_INCLUDE_DIRS})
include_directories(.)

set(ALL_GDBUS_HDRS)
set(ALL_GDBUS_SRCS)

file(GLOB_RECURSE ALL_GDBUS_HDRS "gdbus/*.h")
file(GLOB_RECURSE ALL_GDBUS_SRCS "gdbus/*.c")

add_library(bluez_gdbus
  ${ALL_GDBUS_HDRS} ${ALL_GDBUS_SRCS}
)

set(WIFI_SERVICE_HDR
  ./SetWifiInfoRequest.h
)

set(WIFI_SERVICE_SRC
  ./SetWifiInfoRequest.c
)

add_library(ign_wifi_service
  ${WIFI_SERVICE_HDR} ${WIFI_SERVICE_SRC}
)

# fsm lib
set(WIFI_FSM_HDR
  ./fsm1.h
  ./pairing.h
)

set(WIFI_FSM_SRC
  ./fsm1.c
  ./pairing.c
)

add_library(ign_wifi_fsm1
  ${WIFI_FSM_HDR} ${WIFI_FSM_SRC}
)
target_link_libraries(ign_wifi_fsm1
  lock
)


add_executable(server ./server.c ./error.c ad.c)
target_link_libraries(server
  ${GLIB_LDFLAGS} ${GIO_UNIX_LDFLAGS} ${DBUS_LDFLAGS} 
  bluez_gdbus ign_wifi_service ${WIFI_DEPENDENCIES} ign_wifi_fsm1
)

add_executable(wifi_server ./wifi_server.c ./error.c ad.c)
target_link_libraries(wifi_server
  ${GLIB_LDFLAGS} ${GIO_UNIX_LDFLAGS} ${DBUS_LDFLAGS} 
  bluez_gdbus ign_wifi_service ${WIFI_DEPENDENCIES} ign_wifi_fsm1
)