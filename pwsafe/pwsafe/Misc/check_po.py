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
import string

RE_START = re.compile('^([^\w&]+)\w+')
RE_STOP = re.compile('(\W)$')


def check_re(rx, s1, s2):
	"""checks if regular expresion match from s1 is equal to match from s2"""
	rx1 = rx.search(s1)
	if rx1:
		rx2 = rx.search(s2)
		if not rx2:
			return 1
		if rx1.group(1) != rx2.group(1):
			return 1
	return 0


def check_quotes(msgid, msgstr):
	"""check if quote count is the same"""
	r = 0
	if msgid.count('"') != msgstr.count('"'):
		r += 1
	#if msgid.count("'") / 2 != msgstr.count("'") / 2:
	#	r += 1
	return r


def get_patterns(s):
	"""get %s patterns from s"""
	patterns = []
	in_pattern = 0
	patt = ''
	pattern_end = string.letters + string.whitespace
	for c in s:
		if in_pattern:
			if c in pattern_end:
				patt += c
				patterns.append(patt)
				patt = ''
				in_pattern = 0
				continue
		if c == '%':
			in_pattern = 1
		if in_pattern:
			patt += c
	if patt:
		patterns.append(patt)
	return patterns


def check_cpatterns(msgid, msgstr):
	"""check if %s, %d etc patterns from msgid matches patterns from msgstr"""
	patts1 = get_patterns(msgid)
	patts2 = get_patterns(msgstr)
	if len(patts1) != len(patts2):
		return 1
	patts1.sort()
	patts2.sort()
	if patts1 != patts2:
		return 1
	return 0


def check_accelerator(msgid, msgstr):
	"""check if accelrator &x is the same in both strings"""
	msgid = msgid.lower()
	msgstr = msgstr.lower()
	p1 = msgid.find('&')
	if p1 >= 0:
		a1 = msgid[p1:p1 + 2]
		p2 = msgstr.find('&')
		if p2 < 0 and len(a1) > 1:
			if a1[-1] in msgstr:
				# warn if there is no accelerator in translated version
				# but "accelerated" letter is available
				return 1
		else:
			a2 = msgstr[p2:p2 + 2]
			if a1 != a2:
				return 0 # ok they can be different
	return 0


def check_str(msgid, msgstr):
	"""checks is msgstr had the same leading blanks, etc"""
	warn_cnt = 0
	if msgid and msgstr:
		warn_cnt += check_re(RE_START, msgid, msgstr)
		warn_cnt += check_re(RE_STOP, msgid, msgstr)
		warn_cnt += check_quotes(msgid, msgstr)
		warn_cnt += check_cpatterns(msgid, msgstr)
		warn_cnt += check_accelerator(msgid, msgstr)
	return warn_cnt


def check_po(fn):
	"""checks .po file"""
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
			msgstr = ''
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
