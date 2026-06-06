#!/usr/bin/env python
# -*- coding: utf8 -*-

__version__ = '$Id$'

# author: Michał Niklas, michal.niklas@wp.pl

USAGE = """change "TRANSLATION" into "en:English text" for untranslated texts
\tusage:
\t\tlng_prepare.py [file_name]
\t\t\t-coverts selected file and creates .lng2 file

\t\tlng_prepare.py 
\t\t\t-coverts pwsafe_*.lng files and creates .lng2 files
"""

# untranslated Polish texts
"""
; START_SHOW "Show in start menu"
LangString START_SHOW ${LANG_POLISH} "TRANSLATION"

; START_SHORTCUT "Install desktop shortcut"
LangString START_SHORTCUT ${LANG_POLISH} "TRANSLATION"
"""

import sys
import glob
import re

RE_ENG_TXT = re.compile(r';\s*(\w+)\s+\"(.*)\"')
RE_TRANSLATION = re.compile(r'LangString\s+(\w+)\s+.* \"(\w+)\"')


def prepare_file(fn):
	change_cnt = 0
	f = open(fn, 'r')
	lines = f.readlines()
	f.close()
	tran_dict = {}
	for line in lines:
		line = line.strip()
		if line.startswith(';'):
			rx = RE_ENG_TXT.search(line)
			if rx:
				tran_dict[rx.group(1)] = rx.group(2)
	lines2 = []
	for line in lines:
		line = line.strip()
		rx = RE_TRANSLATION.search(line)
		if rx:
			k = rx.group(1)
			if rx.group(2) == 'TRANSLATION':
				try:
					v = tran_dict[k]
				except KeyError:
					v = k
				line = line.replace('"TRANSLATION"', '"en:%s"' % (v))
				change_cnt += 1
		lines2.append(line)
	if change_cnt > 0:
		f = open(fn + '2', 'w')
		f.write('\n'.join(lines2))
		f.close()



if '--version' in sys.argv:
	print(__version__)
elif '--help' in sys.argv:
	print(USAGE)
else:
	ac = 0
	for fn in sys.argv[1:]:
		if not fn.startswith('--'):
			prepare_file(fn)
			ac += 1
	if ac < 1:
		for fn in glob.glob('pwsafe_??.lng'):
			prepare_file(fn)
