cmake_minimum_required(VERSION 3.4)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLIB REQUIRED glib-2.0)
include_directories(${GLIB_INCLUDE_DIRS})

set(CMAKE_BUILD_TYPE Debug)

set(TEST_DEPENDENCIES
  pthread
  ble
  mqtt
  ign_proto
  ign_lib
)

add_executable(decode_base64 ./decode_base64.c)
target_link_libraries(decode_base64 ${TEST_DEPENDENCIES})

add_executable(scan ./scan.c)
target_link_libraries(scan ${TEST_DEPENDENCIES})

add_executable(guest_unlock ./guest_unlock.c)
target_link_libraries(guest_unlock ${TEST_DEPENDENCIES})

add_executable(guest_lock ./guest_lock.c)
target_link_libraries(guest_lock ${TEST_DEPENDENCIES})


#add_executable(pair ./pair.c)
#target_link_libraries(pair
#  ${TEST_DEPENDENCIES}
#)

#add_executable(admin_unpair ./admin_unpair.c)
#target_link_libraries(admin_unpair
#  ${TEST_DEPENDENCIES}
#)

#add_executable(admin_unlock ./admin_unlock.c)
#target_link_libraries(admin_unlock
#  ${TEST_DEPENDENCIES}
#)

#add_executable(multi_admin_unlock ./multi_admin_unlock.c)
#target_link_libraries(multi_admin_unlock
#  ${TEST_DEPENDENCIES}
#)

#add_executable(admin_lock ./admin_lock.c)
#target_link_libraries(admin_lock
#  ${TEST_DEPENDENCIES}
#)

#add_executable(admin_get_logs ./admin_get_logs.c)
#target_link_libraries(admin_get_logs
#  ${TEST_DEPENDENCIES}
#)

#add_executable(admin_get_lock_status ./admin_get_lock_status.c)
#target_link_libraries(admin_get_lock_status
#  ${TEST_DEPENDENCIES}
#)

#add_executable(admin_get_battery ./admin_get_battery.c)
#target_link_libraries(admin_get_battery
#  ${TEST_DEPENDENCIES}
#)

#add_executable(admin_create_pin ./admin_create_pin.c)
#target_link_libraries(admin_create_pin
#  ${TEST_DEPENDENCIES}
#)

#add_executable(admin_delete_pin ./admin_delete_pin.c)
#target_link_libraries(admin_delete_pin
#  ${TEST_DEPENDENCIES}
#)


