#!/bin/bash
~/nanopb-0.4.1/generator-bin/protoc --nanopb_out=./ ./bridge.proto
gcc -Wall main.c pb_encode.c pb_decode.c pb_common.c bridge.pb.c -lpaho-mqtt3cs -lpthread -I ./ -o 123
./123 && rm ./123

