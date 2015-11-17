include(CheckLibraryExists)

if (WIN32)
  set (OS_SRCS
    indows/debug.cpp
    windows/dir.cpp
    windows/env.cpp
    windows/file.cpp
    windows/KeySend.cpp
    windows/lib.cpp
    windows/logit.cpp
    windows/media.cpp
    windows/mem.cpp
    windows/rand.cpp
    windows/registry.cpp
    windows/run.cpp
    windows/sleep.cpp
    windows/utf8conv.cpp
    windows/UUID.cpp
    windows/pws_autotype/pws_at.cpp
    windows/pws_autotype/stdafx.cpp
    windows/pws_osk/pws_osk.cpp
    windows/pws_osk/stdafx.cpp
    windows/yubi/stdafx.cpp
    windows/yubi/YkLib.cpp
    )
  elseif (APPLE)
  set (OS_SRCS
    mac/debug.cpp
    mac/dir.cpp
    mac/env.cpp
    mac/file.cpp
    mac/KeySend.cpp
    mac/logit.cpp
    mac/macsendstring.cpp
    mac/mem.cpp
    mac/pws_str.cpp
    mac/pws_time.cpp
    mac/rand.cpp
    mac/registry.cpp
    mac/run.cpp
    mac/sleep.cpp
    mac/utf8conv.cpp
    mac/UUID.cpp
    )
  elseif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set (OS_SRCS
    linux/KeySend.cpp
    linux/PWYubi.cpp
    linux/UUID.cpp
    linux/debug.cpp
    linux/dir.cpp
    linux/env.cpp
    linux/file.cpp
    linux/logit.cpp
    linux/media.cpp
    linux/mem.cpp
    linux/pws_str.cpp
    linux/pws_time.cpp
    linux/rand.cpp
    linux/registry.cpp
    linux/run.cpp
    linux/sleep.cpp
    linux/unicode2keysym.cpp
    linux/utf8conv.cpp
    linux/xsendstring.cpp
    )
else()
  message (FATAL_ERROR "Unsupported OS "
    ${CMAKE_SYSTEM_NAME}
    " - can't build OS library")
endif()

add_library(os ${OS_SRCS})
