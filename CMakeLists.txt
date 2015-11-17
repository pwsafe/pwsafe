cmake_minimum_required (VERSION 3.0)
project (pwsafe CXX C)

# prerequisites
include(CheckLibraryExists)
include(CheckIncludeFile)
include(CheckCXXSourceCompiles)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set (GTEST_ROOT "/usr/src/gtest/build"
    CACHE PATH "The root directory of the gtest install prefix")
endif()
find_package(GTest REQUIRED)

if (NOT WIN32)
  # TODO - add support for wx build under Windows
  set(wxWidgets_CONFIGURATION mswu)
  find_package(wxWidgets COMPONENTS adv base core html net REQUIRED)
  include(${wxWidgets_USE_FILE})
endif()

# Configurable options:
option (NO_YUBI "Set ON to disable YubiKey support" OFF)

if(NOT NO_YUBI)
  CHECK_LIBRARY_EXISTS(ykpers-1 yk_init "ykpers-1/ykpers.h" HAVE_YKPERS_H)
endif(NOT NO_YUBI)

if(NOT HAVE_YKPERS_H)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_YUBI")
  message(STATUS "Yubikey support disabled")
endif(NOT HAVE_YKPERS_H)

enable_testing()

# FindXercesC module was introduced in cmake 3.1.
# Debian stable (jessie 8.2) currently provide cmake 3.0.2
# TODO: support compilation without Xerces-c
find_package (XercesC QUIET)
if (NOT XercesC_FOUND)
  set (XercesC_INCLUDE_DIR "/usr/include")
  set (XercesC_LIBRARY "xerces-c")
endif(NOT XercesC_FOUND)
include_directories( ${XercesC_INCLUDE_DIR} )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -DUSE_XML_LIBRARY=XERCES -DWCHAR_INCOMPATIBLE_XMLCH ${CMAKE_WXWINDOWS_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -DDEBUG -gdwarf-4 -O0 -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

# Common configurations

set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Installation directory for executables")

include_directories ($GTEST_INCLUDE_DIRS)
include_directories (${PROJECT_SOURCE_DIR}/src/os)
include_directories (${PROJECT_SOURCE_DIR}/src/core)
include_directories (${PROJECT_BINARY_DIR})
include_directories ("${PROJECT_SOURCE_DIR}/src"
  "${PROJECT_SOURCE_DIR}/src/core"
  "${PROJECT_SOURCE_DIR}/src/ui/wxWidgets")

# Version configuration:
# Following can be different for windows/wx, just wrap around if/else/endif
set (pwsafe_VERSION_MAJOR 4)
set (pwsafe_VERSION_MINOR 0)
set (pwsafe_REVISION 0)
set (pwsafe_SPECIALBUILD "ALPHA")

execute_process(COMMAND "git" "describe" "--all" "--always" "--dirty=+" "--long"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  RESULT_VARIABLE res
  OUTPUT_VARIABLE pwsafe_VERSTRING
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if (res)
  set(pwsafe_VERSTRING "local")
endif()

if (WIN32)
  set(VERSION_IN "${PROJECT_SOURCE_DIR}/src/ui/Windows/version.in")
else()
  set(VERSION_IN "${PROJECT_SOURCE_DIR}/src/ui/wxWidgets/version.in")
endif()

configure_file (
  ${VERSION_IN}
  ${PROJECT_BINARY_DIR}/version.h
  )


check_include_file(stdint.h    HAVE_STDINT_H)
CHECK_CXX_SOURCE_COMPILES("
#include <sys/prctl.h>
int main() { prctl(PR_SET_DUMPABLE, 0); return 0; }
" HAVE_PR_SET_DUMPABLE)

if(APPLE)
CHECK_CXX_SOURCE_COMPILES("
#include <sys/types.h>
#include <sys/ptrace.h>
int main() { ptrace(PT_DENY_ATTACH, 0, 0, 0); return 0; }
" HAVE_PT_DENY_ATTACH)
endif()

if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Release )
endif( NOT CMAKE_BUILD_TYPE )

if(CMAKE_BUILD_TYPE STREQUAL Release)
  message(STATUS "Possibly disabling ptrace from built binary")

  if(HAVE_PR_SET_DUMPABLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_PR_SET_DUMPABLE")
  endif()

  if(HAVE_PT_DENY_ATTACH)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DHAVE_PT_DENY_ATTACH")
  endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  message(STATUS "ptrace and core dumps allowed from built binary")
endif()

if(HAVE_YKPERS_H)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/include/ykpers-1")
  list(APPEND CMAKE_REQUIRED_LIBRARIES ykpers-1)
endif(HAVE_YKPERS_H)

# Subdirectories

add_subdirectory (src/os) # os-specific library
add_subdirectory (src/core) # core library
add_subdirectory (src/test) # tests (gtest framework)
add_subdirectory (help) # online help
if (WIN32)
  # add_subdirectory (src/ui/Windows)
  # add_subdirectory (src/ui/Windows/I18N)
else()
  add_subdirectory (src/ui/wxWidgets)
  # add_subdirectory (src/ui/wxWidgets/I18N)
endif()

add_executable(pwsafe ${PWSAFE_SRCS})
set_property(TARGET pwsafe PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE True)
target_link_libraries(pwsafe core os)
target_link_libraries(pwsafe ${wxWidgets_LIBRARIES} uuid Xtst X11 ${CMAKE_REQUIRED_LIBRARIES} ${XercesC_LIBRARY})
