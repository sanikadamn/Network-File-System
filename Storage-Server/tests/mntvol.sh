#!/usr/bin/env sh

usage () {
    echo "Invalid usage!"
    echo "Usage: $0 image-path"
    exit
}

[ $# != 1 ] && usage

# Creating /mnt/hdfs to mount all filesystem images there
[ ! -e /mnt/hdfs ] && sudo mkdir /mnt/hdfs

# Create path for fs image
FILENAME="${1##*/}"
MNTNAME="${FILENAME%%.*}"

if [ ! -e /mnt/hdfs/"$MNTNAME" ]; then
    sudo mkdir /mnt/hdfs/"$MNTNAME"
else
    echo "unmounting any existing dir..."
    sudo umount -d /mnt/hdfs/"$MNTNAME"
fi

# Mount fs
sudo mount -t auto -o loop "$1" /mnt/hdfs/"$MNTNAME" && echo "mounted img at /mnt/hdfs/$MNTNAME"
