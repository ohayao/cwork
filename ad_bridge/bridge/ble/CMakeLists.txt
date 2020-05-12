
set(ALL_BLE_HDRS)
set(ALL_BLE_SRCS)

set(BLE_DEPENDENCIES
  lock
  gattlib
  pthread
)

file(GLOB_RECURSE ALL_BLE_HDRS "./*.h")
file(GLOB_RECURSE ALL_BLE_SRCS "./*.c")

add_library(ble
  ${ALL_BLE_HDRS} ${ALL_BLE_SRCS}
)
target_link_libraries(ble
  ${BLE_DEPENDENCIES}
)