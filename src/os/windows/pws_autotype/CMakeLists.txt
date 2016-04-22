
set (pwsafe_at_VERSION_MAJOR 1)
set (pwsafe_at_VERSION_MINOR 1)
set (pwsafe_at_REVISION 0)
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

set (VERSION_AT_IN "version.cmake")

configure_file (
    ${VERSION_AT_IN}
    "version.h"
    )

set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} /D PWS_AT_EXPORTS")
add_library(pws_at SHARED ${AT_SRCS} ${AT_RES})

install (TARGETS pws_at RUNTIME DESTINATION bin)
