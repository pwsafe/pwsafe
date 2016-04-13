set (OSK_SRCS
    pws_osk.cpp
    stdafx.cpp
)

set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} /D PWS_OSK_EXPORTS")
add_library(pws_osk SHARED ${OSK_SRCS})
