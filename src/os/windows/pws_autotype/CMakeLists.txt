# Version configuration:
# version.mfc for Windows MFC build
# version file has the following format:
# AT_VER_MAJOR = n
# AT_VER_MINOR = n
# AT_VER_REV = n (default 0)
# where AT_VER_MAJOR and AT_VER_MINOR are mandatory, and all must be integers

set (VERSION_FILE "${PROJECT_SOURCE_DIR}/version.mfc")

if (NOT EXISTS ${VERSION_FILE})
  message (FATAL_ERROR "Missing ${VERSION_FILE} - unable to proceed")
endif (NOT EXISTS ${VERSION_FILE})

file ( STRINGS ${VERSION_FILE} VERSION_LIST )

foreach ( VERSION_ITEM IN ITEMS ${VERSION_LIST} )
  if (${VERSION_ITEM} MATCHES "^[ ]*AT_VER_MAJOR[ ]*=[ ]*([0-9]+)")
    string (REGEX REPLACE ".*=[ ]*([0-9]+)" "\\1"
            pwsafe_at_VERSION_MAJOR ${VERSION_ITEM})
  elseif (${VERSION_ITEM} MATCHES "^[ ]*AT_VER_MINOR[ ]*=[ ]*([0-9]+)")
    string (REGEX REPLACE ".*=[ ]*([0-9]+)" "\\1"
            pwsafe_at_VERSION_MINOR ${VERSION_ITEM})
  elseif (${VERSION_ITEM} MATCHES "^[ ]*AT_VER_REV[ ]*=[ ]*([0-9]+)")
    string (REGEX REPLACE ".*=[ ]*([0-9]+)" "\\1"
            pwsafe_at_REVISION ${VERSION_ITEM})
  endif ()
endforeach ( VERSION_ITEM )

if (NOT DEFINED pwsafe_at_VERSION_MAJOR)
  message (FATAL_ERROR "AT_VER_MAJOR undefined in ${VERSION_FILE} - unable to proceed")
endif (NOT DEFINED pwsafe_at_VERSION_MAJOR)
if (NOT DEFINED pwsafe_at_VERSION_MINOR)
  message (FATAL_ERROR "AT_VER_MINOR undefined in ${VERSION_FILE} - unable to proceed")
endif (NOT DEFINED pwsafe_at_VERSION_MINOR)

if (pwsafe_at_REVISION)
   set (pwsafe_at_VERSION "${pwsafe_at_VERSION_MAJOR}.${pwsafe_at_VERSION_MINOR}.${pwsafe_at_REVISION}")
else ()
   set (pwsafe_at_VERSION "${pwsafe_at_VERSION_MAJOR}.${pwsafe_at_VERSION_MINOR}.0")
endif (pwsafe_at_REVISION)

set (pwsafe_at_VERSION "${pwsafe_at_VERSION_MAJOR}.${pwsafe_at_VERSION_MINOR}.${pwsafe_at_REVISION}")

execute_process(COMMAND "git" "describe" "--all" "--always" "--dirty=+" "--long"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  RESULT_VARIABLE res
  OUTPUT_VARIABLE pwsafe_at_VERSTRING
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if (res)
  set(pwsafe_at_VERSTRING "local")
endif()

set (AT_SRCS
    pws_at.cpp
    stdafx.cpp
)
set (AT_RES pws_at.rc)

set (VERSION_AT_IN "version.in")

configure_file (
    ${VERSION_AT_IN}
    "version.h"
    )

set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} /D PWS_AT_EXPORTS")
add_library(pws_at SHARED ${AT_SRCS} ${AT_RES})
set_target_properties(pws_at PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CACHEFILE_DIR})

install (TARGETS pws_at RUNTIME DESTINATION bin)
