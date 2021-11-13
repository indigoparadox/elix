#!/bin/bash

IMAGE_NAME="$1"

# If image doesn't exist, create it.
if [ ! -f "$IMAGE_NAME" ]; then
   dd if=/dev/urandom of="$IMAGE_NAME" bs=1M count=25
   /sbin/mkfs.vfat -F 16 "$IMAGE_NAME"
fi

shift

# Copy test files into image.
for f in $@; do
   mcopy -v -D o -i "$IMAGE_NAME" "$f" ::
done

