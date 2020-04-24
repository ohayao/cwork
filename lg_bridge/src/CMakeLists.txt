
set(GATTLIB_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/gattlib/include)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_subdirectory(gattlib)

add_subdirectory(lock)

include_directories(.)
add_subdirectory(bridge_function)