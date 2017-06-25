include(CheckLibraryExists)

if (WIN32)
  set (OS_SRCS
    windows/debug.cpp
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
    )
if (NOT NO_YUBI)
    list(APPEND OS_SRCS
        windows/yubi/stdafx.cpp
        windows/yubi/YkLib.cpp)
endif ()

if (NOT WX_WINDOWS)
    add_subdirectory (windows/pws_autotype)
    add_subdirectory (windows/pws_osk)
endif (NOT WX_WINDOWS)

if (NOT NO_YUBI)
    add_subdirectory (windows/yubi)
endif ()
  elseif (APPLE)
  set (OS_SRCS
    mac/cleanup.cpp
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
    mac/media.cpp
    )
  elseif (${CMAKE_SYSTEM_NAME} MATCHES "(Linux|FreeBSD)")
  set (OS_SRCS
    unix/KeySend.cpp
    unix/UUID.cpp
    unix/cleanup.cpp
    unix/debug.cpp
    unix/dir.cpp
    unix/env.cpp
    unix/file.cpp
    unix/keyname.cpp
    unix/logit.cpp
    unix/media.cpp
    unix/mem.cpp
    unix/pws_str.cpp
    unix/pws_time.cpp
    unix/rand.cpp
    unix/registry.cpp
    unix/run.cpp
    unix/sleep.cpp
    unix/unicode2keysym.cpp
    unix/utf8conv.cpp
    unix/xsendstring.cpp
    )
  set (OS_YUBI_SRC unix/PWYubi.cpp)
else()
  message (FATAL_ERROR "Unsupported OS "
    ${CMAKE_SYSTEM_NAME}
    " - can't build OS library")
endif()

if(HAVE_YKPERS_H)
    list(APPEND OS_SRCS ${OS_YUBI_SRC})
endif()

add_library(os ${OS_SRCS})
