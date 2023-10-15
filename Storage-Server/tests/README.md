# Testing Framework (In progress)

1. To create new filesystem images, use ``./mkvol.sh``
2. To use the filesystem images, use ``./mntvol.sh``. This will mount the image specified at ``/mnt/hdfs/`` and will allow you to move around files like normal
3. Once done, startup the storage server by providing the path of the mounted fs.
