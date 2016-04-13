set (AT_SRCS
    pws_at.cpp
    stdafx.cpp
)
set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} /D PWS_AT_EXPORTS")
add_library(pws_at SHARED ${AT_SRCS})
