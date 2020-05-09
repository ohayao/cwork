
pkg_search_module(GLIB REQUIRED glib-2.0)
include_directories(${GLIB_INCLUDE_DIRS})

set(IGN_DEPENDENCIES
  pthread
  ble
  mqtt
)
set(IGN_SRCS
  ./ign.c
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

add_executable(ign ${IGN_SRCS})
target_link_libraries(ign
  ${IGN_DEPENDENCIES}
)