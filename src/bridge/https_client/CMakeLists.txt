


include_directories(.)


file(GLOB_RECURSE ALL_HTTPS_CLIENT_HDRS "./*.h")
file(GLOB_RECURSE ALL_HTTPS_CLIENT_SRCS "./*.c")


add_library(http_client
  ${ALL_HTTPS_CLIENT_HDRS} ${ALL_HTTPS_CLIENT_SRCS}
)

