#!/usr/bin/env bash


usage () {
    echo "Invalid usage!"
    echo "Usage: $0 path no-files"
    echo ""
    echo "Creates no-files number of random files at path"
    exit
}

[ $# != 2 ] && usage

i=1
limit=$2
while [ $i -le $limit ]; do
    filename=$(echo -en "$i" | xxd -p)
    echo $filename
    dd if=/dev/urandom of="$1/$filename" bs=1 count=$(( RANDOM + 1024 ))
    i=$(($i+1))
done
