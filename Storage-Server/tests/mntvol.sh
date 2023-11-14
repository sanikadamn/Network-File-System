#!/usr/bin/env sh

usage () {
    echo "Invalid usage!"
    echo "Usage: $0 image-path"
    echo ""
    echo "Mounts image path at /mnt/hdfs, and creates the directory if "
    echo "it does not exist. Also gives $USER permission using chown."
    exit
}

[ $# != 1 ] && usage

# Creating /mnt/hdfs to mount all filesystem images there
[ ! -e /mnt/hdfs ] && sudo mkdir /mnt/hdfs

# Create path for fs image
FILENAME="${1##*/}"
MNTNAME="${FILENAME%%.*}"
MNTPATH="/mnt/hdfs/${MNTNAME}"

if [ ! -e /mnt/hdfs/"$MNTNAME" ]; then
    echo "creating dir for mounting..."
    sudo mkdir "$MNTPATH"
else
    echo "unmounting any existing dir..."
    sudo umount -d "$MNTPATH"
fi

# Mount fs
sudo mount -t auto -o loop "$1" $MNTPATH && echo "mounted img at $MNTPATH" && sudo chown -R $USER:$USER "$MNTPATH"
