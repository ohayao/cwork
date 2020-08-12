
pkg_search_module(GLIB REQUIRED glib-2.0)
include_directories(${GLIB_INCLUDE_DIRS})

set(IGN_DEPENDENCIES
  pthread
  ble
  mqtt
  ign_proto
  https_client
  mbedtls
  mbedcrypto
  mbedx509
)

set(IGN_LIB_SRCS
  ./log.c
  ./sysinfo.c
  ./task_queue.c
  ./mutex_helper.c
  ./bleData.c
  ./mqttData.c
  ./task.c
  ./thread_helper.c
  ./wait_Ble.c
  ./lock_list.c
)

set(IGN_SRCS
  ./ign.c
)

add_library(ign_lib 
  ${IGN_LIB_SRCS}
)

add_executable(ign ${IGN_SRCS})
target_link_libraries(ign
  ${IGN_DEPENDENCIES}
  ign_lib
)
