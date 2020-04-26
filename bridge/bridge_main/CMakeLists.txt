set(IGN_DEPENDENCIES
  pthread
)
set(IGN_SRCS
  ./ign.cc
  ./log.cc
  ./sysinfo.cc
  ./task.cc
  ./mqttData.cc
  ./sysinfo.cc
  ./task_queue.cc

)

add_executable(ign ${IGN_SRCS})
target_link_libraries(ign
  ${IGN_DEPENDENCIES}
)