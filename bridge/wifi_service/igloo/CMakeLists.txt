cmake_minimum_required(VERSION 3.4)

include_directories(${CMAKE_SOURCE_DIR}/bridge/wifi_service/igloo)
find_library(igloo_LIB2
		    libigloo.a 
		    ${CMAKE_SOURCE_DIR}/bridge/wifi_service/igloo)


add_executable(test ./test.c)
target_link_libraries(test
  ${igloo_LIB2}
)