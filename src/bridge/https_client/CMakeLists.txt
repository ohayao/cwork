
# 得有个搜索头文件得东西
# mbedtls 一定要是 .so
file(GLOB_RECURSE ALL_HTTPS_CLIENT_HDRS "./*.h")
file(GLOB_RECURSE ALL_HTTPS_CLIENT_SRCS "./*.c")

add_library(https_client
  ${ALL_HTTPS_CLIENT_HDRS} ${ALL_HTTPS_CLIENT_SRCS}
)

target_link_libraries(https_client
    mbedtls
    mbedcrypto
    mbedx509
)
