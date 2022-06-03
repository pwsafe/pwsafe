#
# FindMagic
# ---------
#
# Find the Magic header and library.
#
# IMPORTED Targets
# ^^^^^^^^^^^^^^^^
#
# none
#
# Result Variables
# ^^^^^^^^^^^^^^^^
#
# This module defines the following variables:
#
# ``MAGIC_FOUND``
#   True if Magic found.
#
# ``MAGIC_INCLUDE_DIRS``
#   Location of magic.h.
#
# ``MAGIC_LIBRARIES``
#   List of libraries when using Magic.
#
#
# Installation files of DEB package 'libmagic-dev'
# ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
# /usr
# /usr/include
# /usr/include/file
# /usr/include/file/file.h
# /usr/include/magic.h
# /usr/lib
# /usr/lib/x86_64-linux-gnu
# /usr/lib/x86_64-linux-gnu/libmagic.a
# /usr/share
# /usr/share/doc
# /usr/share/doc/libmagic-dev
# /usr/share/doc/libmagic-dev/copyright
# /usr/share/man
# /usr/share/man/man3
# /usr/share/man/man3/libmagic.3.gz
# /usr/lib/x86_64-linux-gnu/libmagic.so
# /usr/share/doc/libmagic-dev/changelog.Debian.gz

#
# Locate header 'magic.h'
#
find_path(MAGIC_INCLUDE_DIR
  NAMES magic.h
  PATHS /usr/include /usr/local/include
)
mark_as_advanced(MAGIC_INCLUDE_DIR)

#
# Locate shared library 'libmagic'
#
find_library(MAGIC_LIBRARY
  NAMES magic
  PATHS /usr/lib /usr/lib64 /usr/local/lib
)
mark_as_advanced(MAGIC_LIBRARY)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Magic
  REQUIRED_VARS MAGIC_LIBRARY MAGIC_INCLUDE_DIR
)

if (MAGIC_FOUND)
  set(MAGIC_INCLUDE_DIRS ${MAGIC_INCLUDE_DIR})
  set(MAGIC_LIBRARIES ${MAGIC_LIBRARY})
endif ()
