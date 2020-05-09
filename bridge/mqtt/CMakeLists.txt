


set(ALL_MQTT_HDRS)
set(ALL_MQTT_SRCS)

set(MQTT_DEPENDENCIES
  paho-mqtt3cs
  cjson
)

file(GLOB_RECURSE ALL_MQTT_HDRS "*.h")
file(GLOB_RECURSE ALL_MQTT_SRCS "*.c")

add_library(mqtt
  ${ALL_MQTT_HDRS} ${ALL_MQTT_SRCS}
)
target_link_libraries(mqtt
  ${MQTT_DEPENDENCIES}
)

