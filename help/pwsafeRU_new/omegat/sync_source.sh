#!/bin/sh
rm -rf ./source/*
mkdir -p ./source/html
cp ../../default/*.hh? ./source
cp ../../default/html/* ./source/html
patch -p0 < omegat_helper.diff
