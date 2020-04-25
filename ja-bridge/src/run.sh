#!/bin/bash
../../nanopb-0.4.1/generator-bin/protoc --nanopb_out=./ ./ign.proto
gcc ign.c pb_encode.c pb_decode.c pb_common.c ign.pb.c -lpaho-mqtt3cs -lpthread -I ./ -o ignexe
./ignexe && rm ./ignexe
