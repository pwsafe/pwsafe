#!/bin/sh
REV=ee3022
GIT_ROOT=`git rev-parse --show-toplevel`
CDIR=$GIT_ROOT/help/pwsafeRU/html/images
DDIR=$GIT_ROOT/help/default/html/images

#check for updated files
git diff --name-only $REV HEAD | grep help/default/html/images

#check for new files
cd $DDIR
for f in `tree -fin --noreport .`; do
    if [ -f $f ] && ! [ -e $CDIR/$f ]; then
	echo "$f is absent in translation dir"
    fi
done

#check for old files
cd $CDIR
for f in `tree -fin --noreport .`; do
    if [ -f $f ] && ! [ -e $DDIR/$f ]; then
	echo "$f is absent in default dir"
    fi
done
