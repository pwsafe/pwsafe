#!/bin/sh
LC_ALL=C diff -bNau ../../default/html ./source/html >omegat_helper.diff
LC_ALL=C diff -bNau ../../default/pwsafe.hhc ./source/pwsafe.hhc >>omegat_helper.diff
