#!/bin/bash

BMP_NAME="$1"

# Create random noise bitmap for binary reading.
echo "Creating random bitmap..."
convert -size 32x32 -type truecolor xc:gray +noise random "$BMP_NAME"

