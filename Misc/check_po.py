#!/usr/bin/env python
# -*- coding: utf8 -*-

__version__ = '$Id$'

# author: Michał Niklas, michal.niklas@wp.pl

USAGE = """Ensure translation follows the original (leading blanks, quotes etc.)

Usage: check_po.py [file_name]
"""

# translated Polish texts
"""
#. Resource IDs: (191)
msgid "  Field is too long."
msgstr "  Pole jest zbyt długie."

"""

import sys
import glob
import re

RE_START = re.compile('^([^\w&]+)\w+')
RE_STOP = re.compile('(\W)$')
RE_ACCELERATOR = re.compile('(&.)')


def check_re(rx, s1, s2):
	rx1 = rx.search(s1)
	if rx1:
		rx2 = rx.search(s2)
		if not rx2:
			return 1
		if rx1.group(1) != rx2.group(1):
			return 1
	return 0


def check_str(msgid, msgstr):
	warn_cnt = 0
	if msgid and msgstr:
		warn_cnt += check_re(RE_START, msgid, msgstr)
		warn_cnt += check_re(RE_STOP, msgid, msgstr)
		#warn_cnt += check_re(RE_ACCELERATOR, msgid, msgstr)
	return warn_cnt


def check_po(fn):
	warn_cnt = 0
	item_cnt = 0
	empty_item_cnt = 0
	f = open(fn, 'r')
	lines = f.readlines()
	f.close()
	msgid = ''
	msgstr = ''
	line_no = 0
	for line in lines:
		line_no += 1
		line = line.strip()
		if line.startswith('msgid '):
			item_cnt += 1
			msgid = line[6:]
			msgstr = ''
		if line.startswith('msgstr '):
			msgstr = line[7:]
			msgid = msgid.strip('"')
			msgstr = msgstr.strip('"')
			if not msgstr:
				empty_item_cnt += 1
			else:
				wc = check_str(msgid, msgstr)
				if wc:
					print('%s:%d: [%s] [%s]' % (fn, line_no, msgid, msgstr))
					warn_cnt += wc
			msg_str = ''
	print('%s: items: %d; empty items: %d; warning cnt: %d' % (fn, item_cnt, empty_item_cnt, warn_cnt))




if '--version' in sys.argv:
	print(__version__)
elif '--help' in sys.argv:
	print(USAGE)
else:
	ac = 0
	for fn in sys.argv[1:]:
		if not fn.startswith('-'):
			check_po(fn)
			ac += 1
	if ac < 1:
		for fn in glob.glob('pwsafe_*.po'):
			check_po(fn)
