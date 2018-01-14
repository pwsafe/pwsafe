# -*- coding: utf-8 -*-
"""
Simple hhk generator

Autogeneration of HHK files only work with hhw compiler, but it doens't support
UTF-8 in the keywords index. So we'll extract keywords fron html files and
create static HHK.
"""

import argparse
import os.path
import posixpath
import os
import sys
import logging
from lxml import etree


#########
__version__ = '1.0.0'
_LOGGER = logging.getLogger()
_KEYWORD_CLSID = "clsid:1e2a7bd0-dab9-11d0-b93a-00c04fc99f9e"


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


def process_file(in_file, index, base_uri=None):
    """
      Process .html file
        in_file: path to file
        :type in_file: str
        index:
        :type index: dics(str, set(tuple(str, str)))
        :param base_uri: base uri for links in index
        :type base_uri: str|None

      :raise: RuntimeError in case of processing error
    """
    if not os.path.isfile(in_file):
        raise RuntimeError("File not found: {}".format(in_file))

    _LOGGER.debug("Processing file <{}> with base <{}>"
                  .format(in_file, base_uri))

    name = os.path.basename(in_file)

    if base_uri:
        name = os.path.join(base_uri, name)

    root = etree.parse(in_file, etree.HTMLParser())
    title = root.findtext("head/title") or ""

    for param in root.iterfind(".//object[@classid='{}']/param"
                               .format(_KEYWORD_CLSID)):
        if param.get("name") == "Keyword":
            value = param.get("value", "")
            if not value:
                _LOGGER.warning("Skipping entry with empty keyword in {}".
                                format(name))
                continue

            if value not in index:
                index[value] = set()

            index[value].add((title, name))


def process_dir(in_dir, index, base_uri=None):
    """
      Process directory with .html files
        in_dir: path to directory for processing
        :type in_dir: str
        index:
        :type index: dics(str, set(tuple(str, str)))
        :param base_uri: base uri for links in index
        :type base_uri: str|None

      :raise: RuntimeError in case of processing error
    """
    if not os.path.isdir(in_dir):
        raise RuntimeError("Dir not found: {}".format(in_dir))

    _LOGGER.debug("Processing dir <{}> with base <{}>"
                  .format(in_dir, base_uri))

    base = base_uri or ""
    for root, dirs, files in os.walk(in_dir):
        prefix = os.path.join(base, os.path.relpath(root, in_dir))
        for f in files:
            process_file(os.path.join(root, f), index, prefix)


def load_index(in_file, index):
    """
      Process hhk file
        in_file: path to file
        :type in_file: str
        index:
        :type index: dics(str, set(tuple(str, str)))

      :raise: RuntimeError in case of processing error
    """
    if not os.path.isfile(in_file):
        raise RuntimeError("File not found: {}".format(in_file))

    _LOGGER.debug("Processing hhk file <{}>".format(in_file))

    root = etree.parse(in_file, etree.HTMLParser())

    for link in root.iterfind(".//object[@type='text/sitemap']"):
        keyword = ""
        title = ""
        pos = 0
        for param in link.iterfind("./param"):
            name = param.get("name")
            if name == "Name":
                if pos == 0:
                    keyword = param.get("value", "")
                    if not keyword:
                        _LOGGER.warning("Skipping entry with empty keyword")
                        break
                elif pos % 2:
                    title = param.get("value", "")
                    if not title:
                        _LOGGER.warning("Skipping entry with empty title")
                        break
                else:
                    _LOGGER.warning("Skipping entry with wrong param order")
                    break
            elif name == "Local":
                if pos % 2 == 0:
                    uri = param.get("value", "")
                    if not uri:
                        _LOGGER.warning("Skipping entry with empty uri")
                        break

                    if keyword not in index:
                        index[keyword] = set()

                    index[keyword].add((title, uri))
                else:
                    _LOGGER.warning("Skipping entry with wrong param order")
                    break
            else:
                _LOGGER.warning("Skipping entry with unknown param: {}"
                                .format(name))
                break
            pos += 1


def _add_param(parent, name, value):
    """
    Add param node
    :param parent: parent node
    :type parent: Element
    :param name: value of 'name' attribute
    :type name: str
    :param value: value of 'value' attribute
    :type value: str
    """
    param = etree.SubElement(parent, "param")
    param.set("name", name)
    param.set("value", value)


def save_index(index, out_file):
    """
    Save prepared index into file in HHK format
    :param index: dict with keyword assocciations
    :type index: dics(str, set(tuple(str, str)))
    :param out_file: path to file
    :type out_file: str
    """
    root = etree.Element("html")

    head = etree.SubElement(root, "head")
    meta_content = etree.SubElement(head, "meta")
    meta_content.set("http-equiv", "content-type")
    meta_content.set("content", "text/html; charset=UTF-8")

    body = etree.SubElement(root, "body")

    frame_props = etree.SubElement(body, "object")
    frame_props.set("type", "text/site properties")
    _add_param(frame_props, "FrameName", "right")

    ul = etree.SubElement(body, "ul")

    for keyword, links in index.iteritems():
        li = etree.SubElement(ul, "li")
        obj = etree.SubElement(li, "object")
        obj.set("type", "text/sitemap")

        _add_param(obj, "Name", keyword)
        for title, path in links:
            _add_param(obj, "Name", title)
            _add_param(obj, "Local",
                       posixpath.normpath(path.replace("\\", "/")))

    tree = etree.ElementTree(root)
    tree.docinfo.public_id = "-//IETF//DTD HTML//EN"

    tree.write(out_file, encoding="UTF-8", xml_declaration=False, method="html")
#        for title, path in links:
#            _LOGGER.debug("<{}>: <{}> => {}"
#                          .format(keyword, title,
#                                  posixpath.normpath(path.replace("\\", "/"))))


#####
# Main
#####
def __main__():
    """
    Main function

    parse arguments and call start
    """

    arg_parser = argparse.ArgumentParser(
        description="HHK file creator")
    arg_parser.add_argument(
        dest="input_file",
        help="input file or directory")
    arg_parser.add_argument(
        dest="out_file",
        help="output file")
    arg_parser.add_argument(
        dest="merge_file",
        nargs="?",
        help="index file for merge")
    arg_parser.add_argument(
        "--baseuri",
        dest="base_uri",
        required=False,
        help="base URI for file links")
    arg_parser.add_argument(
        "--debug",
        dest="debug",
        action="store_true",
        required=False,
        help="Show debug messages in console")

    args = arg_parser.parse_args()

    init_logger(args.debug)

    input_file = args.input_file
    out_file = args.out_file
    merge_file = args.merge_file
    base_uri = args.base_uri

    try:
        if not os.path.exists(input_file):
            raise RuntimeError("Input file/directory '{}' not found"
                               .format(input_file))

        index = {}  # dict(keyword: set((title, uri)))
        if os.path.isfile(input_file):
            process_file(input_file, index, base_uri)
        elif os.path.isdir(input_file):
            process_dir(input_file, index, base_uri)
        else:
            raise RuntimeError("Invalid file type: {}".format(input_file))

        if merge_file:
            load_index(merge_file, index)

        save_index(index, out_file)
    except Exception as ex:
        _LOGGER.error(ex)
        raise
        sys.exit(1)


#####
if __name__ == "__main__":
    __main__()
