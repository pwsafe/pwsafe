#!/bin/sh
# Copyright 2019 Jacob Hrbek <kreyren@rixotstudio.cz>
# Distributed under the terms of the GNU General Public License v3 (https://www.gnu.org/licenses/gpl-3.0.en.html) or later
# Based in part upon 'retry.sh' from rsplib	(https://raw.githubusercontent.com/dreibh/rsplib/master/ci/retry), which is:
# 		Copyright (C) 2018-2019 by Thomas Dreibholz <dreibh@iem.uni-due.de> as GPLv3 or any other GPL at your option

set -e

. QA/travis-ci/travis-common.sh

maxTrials=3
pause=10

while [ $# -gt 1 ] ; do
	case "$1" in
			-t|--tries)
					maxTrials="$2"
					shift 2
				;;
			-p|--pause)
					pause="$2"
					shift 2
				;;
			--)
				shift
				break
				;;
			*)
				die "Usage: $0 [-t|--trials max_trials] [-p|--pause seconds] -- command ..."
	esac
done


attempts=1
result=1
command="$*"
while [ "$result" -ne 0 ] && [ "$attempts" -le "$maxTrials" ]; do
	if [ "$attempts" -gt 1 ]; then
			printf '%s\n' "Sleeping ${pause}s ..."
			sleep "$pause"
	fi
	info "Trying $attempts/$maxTrials: $command"
	sh -c "$command" && result=$? || result=$?
	if [ "$result" -eq 127 ]; then
			# Command not found => no need for a retry!
			exit "$result"
	elif [ "$result" -ne 0 ]; then
			# Attempt failed
			attempts="$((attempts=attempts+1))"
	fi
done


exit "$result"
