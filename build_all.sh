#!/bin/bash

: '
DiskProvision - Allows the creation, management, and updating of disk images for use with QEMU.
build_all.sh - Compile all C files in the src directory and output to the build folder.
BSD 3-Clause "New" or "Revised" License
Copyright (c) 2023 RoyalGraphX
All rights reserved.
'

BUILD_DIR="build"

# Clear the console to begin compilation

clear

# Remove the build folder if it exists
if [ -d "$BUILD_DIR" ]; then
    echo "Removing existing $BUILD_DIR folder..."
    rm -rf "$BUILD_DIR"
fi

# Create the build folder
mkdir "$BUILD_DIR"

# Find all .c files in the src directory
c_files=$(find src -name "*.c")

# Compile each .c file
for c_file in $c_files; do
    output_name="build/${c_file#src/}"
    output_name="${output_name%.c}"  # Remove .c extension
    gcc -o "$output_name" "$c_file"
done

echo "Compilation completed!"

# Clear the console to end compilation, modify sleep timer to see errors or to increase speed.

sleep 4
clear