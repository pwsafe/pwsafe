cmake_minimum_required (VERSION 2.8)
project (pwsafe CXX C)

# prerequisites
include(CheckLibraryExists)
include(CheckIncludeFile)

# Version configuration:
# Following can be different for windows/wx, just wrap around if/else/endif
set (pwsafe_VERSION_MAJOR 0)
set (pwsafe_VERSION_MINOR 98)
set (pwsafe_REVISION 1)
set (pwsafe_SPECIALBUILD "BETA")
set (pwsafe_VERSION "${pwsafe_VERSION_MAJOR}.${pwsafe_VERSION_MINOR}.${pwsafe_REVISION}")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set (GTEST_ROOT "/usr/src/gtest/build"
    CACHE PATH "The root directory of the gtest install prefix")
else()
  set (GTEST_MSVC_SEARCH "MT")
  set (GTEST_ROOT ""
    CACHE PATH "The root directory of the gtest install prefix")
endif()
find_package(GTest REQUIRED)

# TODO - add support for wx build under Windows

if (NOT WIN32)
  # help people with wxWidgets on non-standard installation
  # quick-and-dirty since wxWdigets cmake's support is not yet loaded.
  find_program(PWSHINT_wxconfig wx-config)
  if(NOT(PWSHINT_wxconfig))
    message(STATUS
      "Failed to find 'wx-config' executable:\n"
      "   Tip: can be pointed from command-line this way:\n"
      "        $ cmake -D wxWidgets_CONFIG_EXECUTABLE=/path/to/wx-config ."
    )
  endif()
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
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
option (USE_ASAN "Set ON to add -fsanitize=address to Debug builds" OFF)
endif()

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
  # Requires libasan, which I can't get CHECK_LIBRARY_EXISTS to find under Debian
  CHECK_LIBRARY_EXISTS("asan" "" "" HAS_ASAN)
  CHECK_LIBRARY_EXISTS("asan" "" "/usr/lib/x86_64-linux-gnu/" HAS_ASAN1)
  if (NOT HAS_ASAN AND NOT HAS_ASAN1)
    message (WARNING
      "-fsanitize=address requested but libasan not found.")
  endif (NOT HAS_ASAN AND NOT HAS_ASAN1)
endif (USE_ASAN)

enable_testing()

# Common configurations


include_directories (${GTEST_INCLUDE_DIRS})
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

# Assume that we're either MSVC or a Unix-like
if (MSVC)
set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} /MP /GS /analyze- /W4 /Gy /Zc:wchar_t /Zi /Gm- /O2 /Ob1 /fp:precise /D WIN32 /D NDEBUG  /D _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1 /D _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT=1 /D WINVER=0x0501 /D _USING_V110_SDK71_ /D _UNICODE /D UNICODE /GF /WX- /Zc:forScope /arch:IA32 /Gd /Oy- /MT")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MP /GS /analyze- /W4 /Zc:wchar_t /Zi /Gm- /Od  /fp:precise /D WIN32 /D _DEBUG  /D _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1 /D _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT=1 /D WINVER=0x0501 /D _USING_V110_SDK71_ /D _UNICODE /D UNICODE /WX- /Zc:forScope /RTC1 /arch:IA32 /Gd /Oy- /MTd")
else ()
set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -fPIC -std=gnu++11 -DUNICODE -DWCHAR_INCOMPATIBLE_XMLCH ${CMAKE_WXWINDOWS_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -DDEBUG")
if (USE_ASAN)
  set(CMAKE_CXX_FLAGS_DEBUG
    "${CMAKE_CXX_FLAGS_DEBUG} -gdwarf-4 -O0 -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize=address")
endif (USE_ASAN)
endif (MSVC)

if (WIN32)
  set(VERSION_IN "${PROJECT_SOURCE_DIR}/src/ui/Windows/version.in")
else()
  set(VERSION_IN "${PROJECT_SOURCE_DIR}/src/ui/wxWidgets/version.in")
endif()

configure_file (
  ${VERSION_IN}
  ${PROJECT_BINARY_DIR}/version.h
  )

if( NOT CMAKE_BUILD_TYPE )
  set( CMAKE_BUILD_TYPE Release )
endif( NOT CMAKE_BUILD_TYPE )

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
  add_subdirectory (src/ui/Windows)
  add_subdirectory (src/ui/Windows/I18N)
else()
  add_subdirectory (src/ui/wxWidgets)
  add_subdirectory(src/ui/cli EXCLUDE_FROM_ALL)
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
if (NOT NO_XML)
  target_link_libraries(pwsafe ${XercesC_LIBRARY})
endif (NOT NO_XML)

if (APPLE)
    target_link_libraries(pwsafe ${wxWidgets_LIBRARIES} ${CMAKE_REQUIRED_LIBRARIES})
else ()
    target_link_libraries(pwsafe ${wxWidgets_LIBRARIES} uuid Xtst X11 ${CMAKE_REQUIRED_LIBRARIES})
endif()

# Installation stuff (for 'make install', but mainly for 'make package')
install (TARGETS pwsafe RUNTIME DESTINATION "bin")

install (DIRECTORY xml DESTINATION "share/pwsafe/")
install (FILES "docs/pwsafe.1" DESTINATION "share/man/man1") # gzip'ed in postinst
install (FILES "install/desktop/pwsafe.desktop" DESTINATION "share/pwsafe")
install (FILES "install/graphics/pwsafe.png" DESTINATION "share/pwsafe")
install (DIRECTORY "${CMAKE_BINARY_DIR}/src/ui/wxWidgets/I18N/mos/"
  DESTINATION "share/locale")

# Following because "install (DIRECTORY..." copies tree structure, which is not what we
# want here. Also "install (TARGETS" doesn't seem to work with custom targets.
file (GLOB HELPFILES "help/*.zip")
install (FILES ${HELPFILES} DESTINATION "share/doc/passwordsafe/help")
# TODO - Install following only if on a Debian system (or if we're building a deb?)
install (FILES "install/deb/changelog.Debian" DESTINATION "share/doc/passwordsafe")

# Packaging stuff:
## Common:
set (CPACK_PACKAGE_VERSION_MAJOR "${pwsafe_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MAJOR "${pwsafe_VERSION_MINOR}")
set (CPACK_PACKAGE_VERSION_PATCH "${pwsafe_REVISION}")
set (CPACK_PACKAGE_VERSION "${pwsafe_VERSION}-${pwsafe_SPECIALBUILD}")
set (CPACK_PACKAGE_CONTACT "ronys@pwsafe.org")
set (CPACK_PACKAGE_VENDOR "Rony Shapiro")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Password Safe is a password database utility.")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/install/description.txt")

## Debian-specific:
set (CPACK_DEBIAN_PACKAGE_SECTION "utils")
set (CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set (CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://pwsafe.org/")
set (CPACK_DEBIAN_PACKAGE_DEPENDS
  "libc6 (>= 2.7-18lenny4), libuuid1 (>= 1.41.3-1), libwxgtk3.0-0 (>= 3.0.0-2) | libwxgtk3.0-0v5 (>= 3.0.0-2), libxtst6 (>= 2:1.0.3-1), libxerces-c3.1 (>= 3.1.1-1+b1), libykpers-1-1 (>= 1.3.2-1squeeze1)")
set(CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA
  "${CMAKE_CURRENT_SOURCE_DIR}/install/deb/prerm;${CMAKE_CURRENT_SOURCE_DIR}/install/deb/postrm")

## RPM-specific
set (CPACK_RPM_PACKAGE_REQUIRES "wxBase3, wxGTK3, xerces-c, ykpers")
set (CPACK_RPM_PACKAGE_URL "https://pwsafe.org/")
set (CPACK_RPM_PACKAGE_LICENSE "Artistic2.0")
set (CPACK_RPM_PACKAGE_GROUP "Applications/Utils")
include(CPack)
### End of packaging section
### End of CMakeLists.txt

