
pkg_search_module(GLIB REQUIRED glib-2.0)
include_directories(${GLIB_INCLUDE_DIRS})

set(TEST_DEPENDENCIES
  pthread
  ble
  mqtt
  ign_proto
  ign_lib
)


add_executable(test_ble_scan ./test_ble_scan.c)
target_link_libraries(test_ble_scan
  ${TEST_DEPENDENCIES}
)

add_executable(test_ble_pair ./test_ble_pair.c)
target_link_libraries(test_ble_pair
  ${TEST_DEPENDENCIES}
)

add_executable(test_ble_admin_unlock ./test_ble_admin_unlock.c)
target_link_libraries(test_ble_admin_unlock
  ${TEST_DEPENDENCIES}
)

add_executable(test_ble_admin_unpair ./test_ble_admin_unpair.c)
target_link_libraries(test_ble_admin_unpair
  ${TEST_DEPENDENCIES}
)