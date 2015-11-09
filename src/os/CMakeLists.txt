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

if(NOT NO_YUBI)
  CHECK_LIBRARY_EXISTS(ykpers-1 yk_init "ykpers-1/ykpers.h" HAVE_YKPERS_H)
endif(NOT NO_YUBI)

if(NOT HAVE_YKPERS_H)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_YUBI")
  message(STATUS "Yubikey support disabled")
endif(NOT HAVE_YKPERS_H)

set(wxWidgets_CONFIGURATION mswu)
find_package(wxWidgets COMPONENTS adv base core html REQUIRED)
include(${wxWidgets_USE_FILE})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -DDEBUG -gdwarf-4 -O0 -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

include_directories("${PROJECT_SOURCE_DIR}/src/core"
  "${PROJECT_SOURCE_DIR}/src"
  "/usr/include/ykpers-1")

add_library(os ${OS_SRCS})
