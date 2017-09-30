#!/bin/sh
REV=91f0c3efb
GIT_ROOT=`git rev-parse --show-toplevel`
CDIR=$GIT_ROOT/help/pwsafeRU/html/images
DDIR=$GIT_ROOT/help/default/html/images
EXT1="jpg"
EXT2="png"
#check for updated files
git diff --name-only $REV HEAD | grep help/default/html/images

#check for new files
cd $DDIR
for f in `find . -type f`; do
    bf=${f%.*}
    if [ -f $f ] && ! [ -e $CDIR/${bf}.$EXT1 ] && ! [ -e $CDIR/${bf}.$EXT2 ]; then
	echo "$f is absent in translation dir"
    fi
done

#check for old files
cd $CDIR
for f in `find . -type f`; do
    bf=${f%.*}
    if [ -f $f ] && ! [ -e $DDIR/${bf}.$EXT1 ] && ! [ -e $DDIR/${bf}.$EXT2 ]; then
	echo "$f is absent in default dir"
    fi
done
