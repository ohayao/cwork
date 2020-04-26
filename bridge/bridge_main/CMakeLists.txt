set(IGN_DEPENDENCIES
  pthread
)
set(IGN_SRCS
  ./ign.c
  ./log.c
  ./sysinfo.c
  ./task_queue.c
)

add_executable(ign ${IGN_SRCS})
target_link_libraries(ign
  ${IGN_DEPENDENCIES}
)