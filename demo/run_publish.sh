#!/bin/bash
rm ./cliPub
rm ./cliSub
ps -ef|grep cliPub |awk '{print $2}' |xargs kill -9
ps -ef|grep cliSub |awk '{print $2}' |xargs kill -9

gcc -Wall publish.c proto.pb-c.c -lpaho-mqtt3cs -lprotobuf-c -I ../src -I /usr/local/protobuf-c/include/ -o cliPub
gcc -Wall subscribe.c proto.pb-c.c -lpaho-mqtt3cs -lprotobuf-c -I ../src -I /usr/local/protobuf-c/include/ -o cliSub
nohup ./cliPub > /dev/null 2>&1 &
./cliSub


