cmake_minimum_required (VERSION 2.8)
project (pwsafe CXX C)

# prerequisites
include(CheckLibraryExists)
include(CheckIncludeFile)
include(CheckCXXSourceCompiles)

# Version configuration:
# Following can be different for windows/wx, just wrap around if/else/endif
set (pwsafe_VERSION_MAJOR 4)
set (pwsafe_VERSION_MINOR 0)
set (pwsafe_REVISION 0)
set (pwsafe_SPECIALBUILD "ALPHA")


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

  CHECK_INCLUDE_FILE("uuid/uuid.h" UUID_H)
  if (NOT UUID_H)
    message(FATAL_ERROR
      "uuid.h not found - uuid-dev / libuuid-devel not installed?")
    endif(NOT UUID_H)
endif()

# Configurable options:
option (NO_YUBI "Set ON to disable YubiKey support" OFF)
option (NO_XML "Set ON to disable XML import support" OFF)
option (USE_ASAN "Set ON to add -fsanitize=address to Debug builds" OFF)

if(NOT NO_YUBI)
  CHECK_LIBRARY_EXISTS(ykpers-1 yk_init "ykpers-1/ykpers.h" HAVE_YKPERS_H)
endif(NOT NO_YUBI)

if(NOT HAVE_YKPERS_H)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_YUBI")
  message(STATUS "Yubikey support disabled")
endif(NOT HAVE_YKPERS_H)

if (NOT NO_XML)
  find_package (XercesC QUIET MODULE)
  if (NOT XercesC_FOUND)
    # FindXercesC module was added to cmake in 3.1. Debian 8 uses 3.0.2
    # If the module wasn't found yet XML is enabled, we set the default
    # values. If this isn't what you want, either configure NO_XML
    # or edit the followingvalues manually.
    message (WARNING "XML enabled but Xerces-C not found - using defaults")
    set (XercesC_INCLUDE_DIR "/usr/include")
    set (XercesC_LIBRARY "xerces-c")
  endif(NOT XercesC_FOUND)
  include_directories( ${XercesC_INCLUDE_DIR} )
  add_definitions ("-DUSE_XML_LIBRARY=XERCES")
endif (NOT NO_XML)

if (USE_ASAN) 
  CHECK_LIBRARY_EXISTS(asan "" "" HAS_ASAN)
  if (NOT HAS_ASAN)
    message (WARNING
      "-fsanitize=address requested but libasan not found - disabling")
    set (USE_ASAN FALSE)
  endif (HAS_ASAN)
endif (USE_ASAN)

enable_testing()

set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -std=gnu++11 -DWCHAR_INCOMPATIBLE_XMLCH ${CMAKE_WXWINDOWS_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -DDEBUG")
if (USE_ASAN)
  set(CMAKE_CXX_FLAGS_DEBUG
    "${CMAKE_CXX_FLAGS_DEBUG} -gdwarf-4 -O0 -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize=address")
endif (USE_ASAN)
# Common configurations


include_directories ($GTEST_INCLUDE_DIRS)
include_directories (${PROJECT_SOURCE_DIR}/src/os)
include_directories (${PROJECT_SOURCE_DIR}/src/core)
include_directories (${PROJECT_BINARY_DIR})
include_directories ("${PROJECT_SOURCE_DIR}/src"
  "${PROJECT_SOURCE_DIR}/src/core"
  "${PROJECT_SOURCE_DIR}/src/ui/wxWidgets")

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

# Following is because (a) -O3 breaks the test and application, and
# (b) -O3 is the default for cmake
set (CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

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
  find_package(Gettext QUIET)
  if (GETTEXT_FOUND)
    add_subdirectory (src/ui/wxWidgets/I18N)
  else()
    message("Gettext support not found, skipping I18N build")
  endif()
endif()

add_executable(pwsafe ${PWSAFE_SRCS})
set_property(TARGET pwsafe PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE True)
target_link_libraries(pwsafe core os)
if (NOT NOT_XML)
  target_link_libraries(pwsafe ${XercesC_LIBRARY})
endif (NOT NOT_XML)

target_link_libraries(pwsafe ${wxWidgets_LIBRARIES} uuid Xtst X11 ${CMAKE_REQUIRED_LIBRARIES})

# Installation stuff (for 'make install', but mainly for 'make package')
install (TARGETS pwsafe RUNTIME DESTINATION "bin")

install (DIRECTORY xml DESTINATION "share/pwsafe/")
install (FILES "docs/pwsafe.1" DESTINATION "share/man/man1") # gzip'ed in postinst
install (FILES "install/desktop/pwsafe.desktop" DESTINATION "share/pwsafe")
install (FILES "install/graphics/pwsafe.png" DESTINATION "share/pwsafe")
install (DIRECTORY "${CMAKE_BINARY_DIR}/src/ui/wxWidgets/I18N/mos/"
  DESTINATION "share/locale")

# Following because "install (DIRECTORY..." copies tree strcture, which is not what we
# want here. Also "install (TARGETS" doesn't seem to work with custom targets.
file (GLOB HELPFILES "${CMAKE_BINARY_DIR}/help/*.zip")
install (FILES ${HELPFILES} DESTINATION "share/doc/passwordsafe/help")
install (FILES "install/deb/changelog.Debian" DESTINATION "share/doc/passwordsafe")

# Packaging stuff:
## Common:
set (CPACK_PACKAGE_VERSION_MAJOR "${pwsafe_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MAJOR "${pwsafe_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${pwsafe_REVISION}")
set (CPACK_PACKAGE_VERSION "${pwsafe_VERSION_MAJOR}.${pwsafe_VERSION_MINOR}.${pwsafe_REVISION}-${pwsafe_SPECIALBUILD}")
set (CPACK_PACKAGE_CONTACT "ronys@pwsafe.org")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

## Debian-specific:
set (CPACK_DEBIAN_PACKAGE_SECTION "utils")
set (CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set (CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://pwsafe.org/")
set (CPACK_DEBIAN_PACKAGE_DESCRIPTION
  "Portable version of popular password manager program, pwsafe. This is the Linux version of the popular Windows PasswordSafe password manager.")
set (CPACK_DEBIAN_PACKAGE_DEPENDS
  "libc6 (>= 2.7-18lenny4), libuuid1 (>= 1.41.3-1), libwxgtk3.0-0 (>= 3.0.0-2), libxtst6 (>= 2:1.0.3-1), libxerces-c3.1 (>= 3.1.1-1+b1), libykpers-1-1 (>= 1.3.2-1squeeze1)")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
  "${CMAKE_CURRENT_SOURCE_DIR}/install/deb/prerm;${CMAKE_CURRENT_SOURCE_DIR}/install/deb/postrm")

include(CPack)
### End of packaging section
### End of CMakeLists.txt

