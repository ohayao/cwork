#!/bin/bash
~/nanopb-0.4.1/generator-bin/protoc --nanopb_out=./ ./proto.proto
gcc -Wall main.c pb_encode.c pb_decode.c pb_common.c proto.pb.c -I ./ -o 123
./123 && rm ./123

