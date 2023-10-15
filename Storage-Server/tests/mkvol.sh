#!/bin/bash

usage () {
    echo "Invalid usage!"
    echo "Usage: $0 image-path block-size block-count"
    exit
}

[ $# != 3 ] && usage

if [ -f $1 ]; then
   echo "$1 already exists!" && exit
else
    echo "$1 being created..."
    dd if=/dev/zero of="$1" bs="$2" count="$3" && echo "created!"
    echo "adding fs..."
    mkfs -t ext4 "$1" && echo "done!"
fi
