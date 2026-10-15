#!/bin/sh
if [ "$1" == "" ]; then
    echo "missing parameter sdl3 header filename"
    exit 1
fi
../../DreiZehn scripts/main.13 $1
