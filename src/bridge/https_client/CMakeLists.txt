


include_directories(.)

include_directories(../../../mbedtls)


file(GLOB_RECURSE ALL_HTTPS_CLIENT_HDRS "./*.h")
file(GLOB_RECURSE ALL_HTTPS_CLIENT_SRCS "./*.c")


add_library(https_client
  ${ALL_HTTPS_CLIENT_HDRS} ${ALL_HTTPS_CLIENT_SRCS}
)

