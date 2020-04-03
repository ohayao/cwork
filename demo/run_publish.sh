#!/bin/bash
rm 1111
gcc -Wall publish.c -lpaho-mqtt3cs -o 1111
./1111
