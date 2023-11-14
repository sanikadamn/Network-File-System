#!/bin/bash

usage () {
    echo "Invalid usage!"
    echo "Usage: $0 image-path block-size block-count"
    echo ""
    echo "Creates new volume at path image-path, with block-count blocks"
    echo "each of which is block-size bytes. Uses dd under the hood, so "
    echo "arguments defining sizes as K,M,G also work."
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
