#!/bin/bash
rm ./cliPub
rm ./cliSub
ps -ef|grep cliPub |awk '{print $2}' |xargs kill -9
ps -ef|grep cliSub |awk '{print $2}' |xargs kill -9

gcc -Wall publish.c -lpaho-mqtt3cs -o cliPub
gcc -Wall subscribe.c -lpaho-mqtt3cs -o cliSub
nohup ./cliPub > /dev/null 2>&1 &
./cliSub


