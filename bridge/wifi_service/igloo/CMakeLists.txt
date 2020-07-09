cmake_minimum_required(VERSION 3.4)

add_definitions(-D __unix)
SET(CMAKE_BUILD_TYPE "Debug")

include_directories(${CMAKE_SOURCE_DIR}/bridge/wifi_service/igloo)
find_library(igloo_LIB2
		    libigloo.a 
		    ${CMAKE_SOURCE_DIR}/bridge/wifi_service/igloo)

file(GLOB_RECURSE ALL_CBOR_HDRS "cbor/*.h")
file(GLOB_RECURSE ALL_CBOR_SRCS "cbor/*.c")

file(GLOB_RECURSE ALL_NRF_CRYPTO_HDRS "nrf_crypto/*.h")
file(GLOB_RECURSE ALL_NRF_CRYPTO_SRCS "nrf_crypto/*.c")

file(GLOB_RECURSE ALL_CIFRA_HDRS "cifra/*.h")
file(GLOB_RECURSE ALL_CIFRA_SRCS "cifra/*.c")

file(GLOB_RECURSE ALL_MESSAGE_HDRS "messages/*.h")
file(GLOB_RECURSE ALL_MESSAGE_SRCS "messages/*.c")

add_library(nrf_crypto
  ${ALL_NRF_CRYPTO_HDRS} ${ALL_CIFRA_HDRS} ${ALL_MESSAGE_HDRS} ${ALL_CBOR_HDRS}
  ${ALL_NRF_CRYPTO_SRCS} ${ALL_CIFRA_SRCS} ${ALL_MESSAGE_SRCS} ${ALL_CBOR_SRCS}
)


add_executable(test ./test.c ./igloo.c ./serializer.c)
target_link_libraries(test
   nrf_crypto
)