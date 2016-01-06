# -*- coding: utf-8 -*-
"""
Simple newline fix for translated messages in .po files

When using, some tools (for example OmegaT), translated text is split into
sentences to simplify translation. So in some cases newline may break small
sentence that looks strange and \n may be skipped by translator in the
translated block. This tool will scan all message pairs and split translated
text using following rules
 - translated text will be changed if it have no newlines;
 - maximum line width will be set to maximum width for original text;
 - split is made only on word boundaries.

 WARNING: because of some polib internal rules, it may also change some other
 things:
  - reorder metadata fields
  - wrap (reformat) msgid/msgstr text even if it haven't newlines (only .po
    file representation, strings only changed by rules)
  - remove some comments (for example # something before #~ something)
"""

import argparse
import os.path
import sys
import logging
import polib
import textwrap

#########
__version__ = '1.0.0'
_LOGGER = logging.getLogger()
# if line length difference between original and translated text more than
# that, warning will be issued
_WARN_DELTA = 5
_NL_STR = '\n'
# original text contain visible non-breakable spaces (it simplify editing and
# fix issue with polib, that may split on non-breakable space)
_NBSP_STR = u'\u2423'
_NBSP_SUB_STR = u'\u00A0'

_FUZZY_FLAG = 'fuzzy'


def init_logger(enable_debug):
    """
    Initialize logger
      enable_debug: show debug messages in console
      :type enable_debug: bool
    """
    level = logging.DEBUG if enable_debug else logging.INFO

    con_handler = logging.StreamHandler()
    con_handler.setLevel(level)
    _LOGGER.setLevel(level)

    _LOGGER.addHandler(con_handler)


def process_file(in_file, out_file):
    """
      Process .po file
        in_file: path to input file
        :type in_file: str
        out_file: path to output file
        :type out_file: str

      Returns: True, of processing was done, False in case of error
        :return: bool
    """
    _LOGGER.debug("Processing of file '%s' (output will be saved to '%s')",
                  in_file, out_file)
    _LOGGER.debug("Loading file '%s'", in_file)
    try:
        po = polib.pofile(in_file)
    except IOError as e:
        _LOGGER.error("Error while loading file '%s': %s", in_file, e)
        return False
    change_cnt = 0
    skip_cnt = 0
    wrapper = textwrap.TextWrapper(
        break_long_words=False,
        replace_whitespace=False,
        drop_whitespace=True)
    for entry in po:
        if _NL_STR in entry.msgid:
            if process_entry(entry, wrapper):
                change_cnt += 1
            else:
                skip_cnt += 1
        fix_nbsp(entry)

    _LOGGER.info("Changed: %d, skipped: %d", change_cnt, skip_cnt)

    _LOGGER.info("Saving to '%s'", out_file)
    try:
        po.save(out_file)
    except IOError as e:
        _LOGGER.error("Error while saving to file '%s': %s", out_file, e)
        return False
    return True


def process_entry(entry, wrapper):
    """
    Process one entry
      entry: entry for processing
      wrapper: constructed textwrap object used for wrapping
        (width property maye be changed here)
    Returns: True, if entry was changed
      :return: bool
    """
    max_width = max([len(x) for x in entry.msgid.splitlines()])
    max_trans_width = max([len(x) for x in entry.msgstr.splitlines()])
    changed = False

    # if split already OK, don't do anything
    if max_trans_width <= max_width + _WARN_DELTA:
        return changed

    wrapper.width = max_width
    new_line = entry.msgstr

    if _NL_STR in entry.msgstr:
        _LOGGER.debug("Skipped. Newline in translation.\n"
                      "\tmsgid: '%s'\n\tmsgstr: '%s'.\n",
                      polib.escape(entry.msgid), polib.escape(entry.msgstr))
        return changed

    new_line = wrapper.fill(entry.msgstr)

    if new_line != entry.msgstr:
        entry.msgstr = new_line
        _LOGGER.info("Splitted.\n"
                     "\tmsgid: '%s'\n\tmsgstr: '%s'.\n",
                     polib.escape(entry.msgid), polib.escape(entry.msgstr))
        changed = True
    else:
        changed = False

    max_trans_width = max([len(x) for x in entry.msgstr.splitlines()])
    if max_trans_width > max_width + _WARN_DELTA:
        _LOGGER.warning("Fuzzy. Translation's line length more that original.\n"
                        "\tmsgid: '%s'\n\tmsgstr: '%s'.\n",
                        polib.escape(entry.msgid), polib.escape(entry.msgstr))
        # set fuzzy only if string was changed
        if changed and _FUZZY_FLAG not in entry.flags:
            entry.flags.append(_FUZZY_FLAG)
    return changed


def fix_nbsp(entry):
    """
    Process one entry
      entry: entry for processing
    """
    if _NBSP_STR in entry.msgstr:
        entry.msgstr = entry.msgstr.replace(_NBSP_STR, _NBSP_SUB_STR)


#####
# Main
#####
def __main__():
    """
    Main function

    parse arguments and call start
    """

    arg_parser = argparse.ArgumentParser(
        description="Newline fix for .po files")
    arg_parser.add_argument(
        dest="in_file",
        help="input .po file")
    arg_parser.add_argument(
        dest="out_file",
        nargs="?",
        default="",
        help="output file (if not set, equal to input file). "
             "File will be owerwritten if present.")
    arg_parser.add_argument(
        "--debug",
        dest="debug",
        action="store_true",
        required=False,
        help="Show debug messages in console")

    args = arg_parser.parse_args()

    in_file = args.in_file
    out_file = args.out_file or args.in_file
    if not os.path.isfile(in_file):
        sys.stderr.write("Input file '{}' not found\n".format(in_file))
        sys.exit(1)

    init_logger(args.debug)
    if not process_file(in_file, out_file):
        sys.exit(1)

#####
if __name__ == "__main__":
    __main__()
