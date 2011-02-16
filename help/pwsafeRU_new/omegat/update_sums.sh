#!/bin/sh

DDIR=../../default/html/images
> images.md5
for f in `tree -fin --noreport $DDIR`; do
    if [ -f $f ]; then
        md5sum $f >>images.md5
    fi
done
