# Version configuration:
# versionDLLs.mfc for Windows MFC build
# version file has the following format:
# OSK_VER_MAJOR = n
# OSK_VER_MINOR = n
# OSK_VER_REV = n (default 0)
# where OSK_VER_MAJOR and OSK_VER_MINOR are mandatory, and all must be integers

set (VERSION_FILE "${PROJECT_SOURCE_DIR}/version.mfc")

if (NOT EXISTS ${VERSION_FILE})
  message (FATAL_ERROR "Missing ${VERSION_FILE} - unable to proceed")
endif (NOT EXISTS ${VERSION_FILE})

file ( STRINGS ${VERSION_FILE} VERSION_LIST )

foreach ( VERSION_ITEM IN ITEMS ${VERSION_LIST} )
  if (${VERSION_ITEM} MATCHES "^[ ]*OSK_VER_MAJOR[ ]*=[ ]*([0-9]+)")
    string (REGEX REPLACE ".*=[ ]*([0-9]+)" "\\1"
            pwsafe_osk_VERSION_MAJOR ${VERSION_ITEM})
  elseif (${VERSION_ITEM} MATCHES "^[ ]*OSK_VER_MINOR[ ]*=[ ]*([0-9]+)")
    string (REGEX REPLACE ".*=[ ]*([0-9]+)" "\\1"
            pwsafe_osk_VERSION_MINOR ${VERSION_ITEM})
  elseif (${VERSION_ITEM} MATCHES "^[ ]*OSK_VER_REV[ ]*=[ ]*([0-9]+)")
    string (REGEX REPLACE ".*=[ ]*([0-9]+)" "\\1"
            pwsafe_osk_REVISION ${VERSION_ITEM})
  endif ()
endforeach ( VERSION_ITEM )

if (NOT DEFINED pwsafe_osk_VERSION_MAJOR)
  message (FATAL_ERROR "OSK_VER_MAJOR undefined in ${VERSION_FILE} - unable to proceed")
endif (NOT DEFINED pwsafe_osk_VERSION_MAJOR)
if (NOT DEFINED pwsafe_osk_VERSION_MINOR)
  message (FATAL_ERROR "OSK_VER_MINOR undefined in ${VERSION_FILE} - unable to proceed")
endif (NOT DEFINED pwsafe_osk_VERSION_MINOR)

if (pwsafe_osk_REVISION)
   set (pwsafe_osk_VERSION "${pwsafe_osk_VERSION_MAJOR}.${pwsafe_osk_VERSION_MINOR}.${pwsafe_osk_REVISION}")
else ()
   set (pwsafe_osk_VERSION "${pwsafe_osk_VERSION_MAJOR}.${pwsafe_osk_VERSION_MINOR}.0")
endif (pwsafe_osk_REVISION)

set (pwsafe_osk_VERSION "${pwsafe_osk_VERSION_MAJOR}.${pwsafe_osk_VERSION_MINOR}.${pwsafe_osk_REVISION}")

execute_process(COMMAND "git" "describe" "--all" "--always" "--dirty=+" "--long"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  RESULT_VARIABLE res
  OUTPUT_VARIABLE pwsafe_osk_VERSTRING
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if (res)
  set(pwsafe_osk_VERSTRING "local")
endif()

set (OSK_SRCS
    pws_osk.cpp
    stdafx.cpp
)
set (OSK_RES pws_osk.rc)

set (VERSION_OSK_IN "version.in")

configure_file (
    ${VERSION_OSK_IN}
    "version.h"
    )

set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} /D PWS_OSK_EXPORTS")
add_library(pws_osk SHARED ${OSK_SRCS} ${OSK_RES})
set_target_properties(pws_osk PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CACHEFILE_DIR})

install (TARGETS pws_osk RUNTIME DESTINATION bin)
