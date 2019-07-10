#!/bin/bash

IMAGE_NAME=testimg.img

rm "$IMAGE_NAME"
dd if=/dev/urandom of="$IMAGE_NAME" bs=1M count=25
/sbin/mkfs.vfat -F 16 "$IMAGE_NAME"

mcopy -i "$IMAGE_NAME" doxyfile ::
mcopy -i "$IMAGE_NAME" x86 ::

rm -f count.txt
for i in {1..10000}; do
   echo $i >> count.txt
done

mcopy -i "$IMAGE_NAME" count.txt ::
rm count.txt

rm -f random.bmp
convert -size 32x32 -type truecolor xc:gray +noise random random.bmp
mcopy -i "$IMAGE_NAME" random.bmp ::
rm random.bmp

