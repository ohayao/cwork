

include_directories(${GATTLIB_INCLUDE_DIR})

pkg_search_module(GLIB REQUIRED glib-2.0)
pkg_search_module(GATTLIB REQUIRED gattlib)

include_directories(${GIO_UNIX_INCLUDE_DIRS} ${BLUEZ_INCLUDE_DIRS})
set(BRIDGE_SRCS 
  bridge.cpp
)

add_library(bridge SHARED ${BRIDGE_SRCS})
target_link_libraries(bridge ${PCRE_LIBRARIES} pthread lock gattlib)

add_executable(test_bridge test_bridge.cpp)

target_link_libraries(test_bridge 
 bridge pthread
)

add_executable(test_lock test_lock.cpp)
target_link_libraries(test_lock 
 lock
)
