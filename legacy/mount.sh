#!/bin/bash

#
#  Copyright (c) 2024 RoyalGraphX
#  BSD 3-Clause License
#

# Check if nbd module is already loaded
if lsmod | grep -q "^nbd"; then
    echo "nbd module is already loaded. Proceeding..."
else
    echo "nbd module is not loaded. Loading..."
    sudo modprobe nbd max_part=8
fi

# Check if /dev/nbd0 is already connected
if lsblk -o NAME | grep -q "^nbd0$"; then
    echo "/dev/nbd0 is already connected. Disconnecting..."
    sudo qemu-nbd --disconnect /dev/nbd0
fi

sudo qemu-nbd --connect=/dev/nbd0 -f raw OpenCore.img
echo "Connected /dev/nbd0."

# Create "mnt" directory if it doesn't exist
if [ ! -d "mnt" ]; then
    mkdir mnt
    echo "Created 'mnt' directory."
fi

# Mount /dev/nbd0 to the "mnt" directory with desired ownership
sudo mount -o uid=$(id -u),gid=$(id -g) /dev/nbd0 mnt
echo "Image mounted to 'mnt' directory with desired ownership successfully."