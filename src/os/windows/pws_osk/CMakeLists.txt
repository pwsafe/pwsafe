set (pwsafe_osk_VERSION_MAJOR 1)
set (pwsafe_osk_VERSION_MINOR 1)
set (pwsafe_osk_REVISION 0)
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

set (VERSION_OSK_IN "version.cmake")

configure_file (
    ${VERSION_OSK_IN}
    "version.h"
    )

set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} /D PWS_OSK_EXPORTS")
add_library(pws_osk SHARED ${OSK_SRCS} ${OSK_RES})

install (TARGETS pws_osk RUNTIME DESTINATION bin)
