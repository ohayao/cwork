#!/bin/bash
#../../nanopb-0.4.1/generator-bin/protoc --nanopb_out=./ ./ign.proto
python3 ../../nanopb-0.4.1/generator/nanopb_generator.py ./ign.proto
gcc ign.c pb_encode.c pb_decode.c pb_common.c ign.pb.c cJSON.c ../../src/bridge/https_client/https.c -lpaho-mqtt3cs -lpthread -lcjson -lmbedtls -lmbedcrypto -lmbedx509 -I ../../mbedtls/ -I ./ -o ignexe
./ignexe && rm ./ignexe
