#!/bin/sh
if [ "$1" == "" ]; then
    echo "missing parameter sdl3 header filename"
    exit 1
fi
# ./parse_sdl $1 | grep $1 | grep struct | sort -r | uniq | ../../DreiZehn parser.13 $1 | less
# ./parse_sdl $1 | grep $1 | sort -r | uniq | ../../DreiZehn parser.13 $1 | less
# ./parse_sdl $1 | grep $1 | sort -r | uniq | ../../DreiZehn parser.13 $1
./clang_parser/parse_sdl $1 | grep $1 | sort -r | uniq > json/$1.json
exit 0
