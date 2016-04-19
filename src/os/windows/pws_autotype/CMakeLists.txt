set (AT_SRCS
    pws_at.cpp
    stdafx.cpp
)
set (AT_RES pws_at.rc)

set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} /D PWS_AT_EXPORTS")
add_library(pws_at SHARED ${AT_SRCS} ${AT_RES})
set (LIBRARY_OUTPUT_NAME_Debug "pws_at_D")

install (TARGETS pws_at RUNTIME DESTINATION bin)
