if (WIN32)
  cmake_minimum_required (VERSION 3.5.1)
else (WIN32)
  # oldest cmake version is currently on CentOS 7:
  cmake_minimum_required (VERSION 2.8)
endif (WIN32)
if (POLICY CMP0069)
  cmake_policy(SET CMP0069 NEW)
endif (POLICY CMP0069)

if (POLICY CMP0048)
  cmake_policy(SET CMP0048 NEW)
endif (POLICY CMP0048)

project (passwordsafe CXX C)

# prerequisites
include(CheckLibraryExists)
include(CheckIncludeFile)

# IPO = interprocedural optimisation, not initial public offering (unfortunately)
include(CheckIPOSupported OPTIONAL RESULT_VARIABLE CHECK_IPO_FOUND)
if (${CHECK_IPO_FOUND} STREQUAL "NOTFOUND")
else (${CHECK_IPO_FOUND} STREQUAL "NOTFOUND")
  check_ipo_supported(RESULT IPO_SUPPORTED)
endif (${CHECK_IPO_FOUND} STREQUAL "NOTFOUND")

if (NOT WIN32)
include(FindUnixCommands)
endif (NOT WIN32)

# Version configuration:
# version.mfc for Windows MFC build, version.wx for all the rest
# version file has the following format:
# VER_MAJOR = n
# VER_MINOR = n
# VER_REV = n (default 0)
# VER_SPECIAL = text [or not set]
# where VER_MAJOR and VER_MINOR are mandatory, and all except
# VER_SPECIAL must be integers

if (WIN32 AND NOT WX_WINDOWS)
   set (VERSION_FILE "${PROJECT_SOURCE_DIR}/version.mfc")
else (WIN32 AND NOT WX_WINDOWS)
   set (VERSION_FILE "${PROJECT_SOURCE_DIR}/version.wx")
endif (WIN32 AND NOT WX_WINDOWS)

if (NOT EXISTS ${VERSION_FILE})
  message (FATAL_ERROR "Missing ${VERSION_FILE} - unable to proceed")
endif (NOT EXISTS ${VERSION_FILE})

file ( STRINGS ${VERSION_FILE} VERSION_LIST )

foreach ( VERSION_ITEM IN ITEMS ${VERSION_LIST} )
  if (${VERSION_ITEM} MATCHES "^[ ]*VER_MAJOR[ ]*=[ ]*([0-9]+)")
    string (REGEX REPLACE ".*=[ ]*([0-9]+)" "\\1"
            pwsafe_VERSION_MAJOR ${VERSION_ITEM})
  elseif (${VERSION_ITEM} MATCHES "^[ ]*VER_MINOR[ ]*=[ ]*([0-9]+)")
    string (REGEX REPLACE ".*=[ ]*([0-9]+)" "\\1"
            pwsafe_VERSION_MINOR ${VERSION_ITEM})
  elseif (${VERSION_ITEM} MATCHES "^[ ]*VER_REV[ ]*=[ ]*([0-9]+)")
    string (REGEX REPLACE ".*=[ ]*([0-9]+)" "\\1"
            pwsafe_REVISION ${VERSION_ITEM})
  elseif (${VERSION_ITEM} MATCHES "^[ ]*VER_SPECIAL[ ]*=[ ]*(.+)")
    string (REGEX REPLACE ".*=[ ]*(.+)" "\\1"
            pwsafe_SPECIALBUILD ${VERSION_ITEM})
  endif ()
endforeach ( VERSION_ITEM )

if (NOT DEFINED pwsafe_VERSION_MAJOR)
  message (FATAL_ERROR "VER_MAJOR undefined in ${VERSION_FILE} - unable to proceed")
endif (NOT DEFINED pwsafe_VERSION_MAJOR)
if (NOT DEFINED pwsafe_VERSION_MINOR)
  message (FATAL_ERROR "VER_MINOR undefined in ${VERSION_FILE} - unable to proceed")
endif (NOT DEFINED pwsafe_VERSION_MINOR)

if (pwsafe_REVISION)
   set (pwsafe_VERSION "${pwsafe_VERSION_MAJOR}.${pwsafe_VERSION_MINOR}.${pwsafe_REVISION}")
else ()
   set (pwsafe_VERSION "${pwsafe_VERSION_MAJOR}.${pwsafe_VERSION_MINOR}.0")
endif (pwsafe_REVISION)

# Configurable options:
option (NO_YUBI "Set ON to disable YubiKey support" OFF)
option (NO_GTEST "Set ON to disable gtest unit testing" OFF)

if (WIN32)
  option (WX_WINDOWS "Build wxWidget under Windows" OFF)
  if (WX_WINDOWS)
    option (XML_XERCESC "Set ON for XML import support with XercesC" ON)
  else (WX_WINDOWS)
    option (XML_MSXML "Set ON for XML import support with MSXML" ON)
    option (XML_XERCESC "Set ON for XML import support with XercesC" OFF)    
  endif (WX_WINDOWS)
  if (XML_MSXML AND XML_XERCESC)
    message (FATAL_ERROR "MSXML and XERCESC are mutually exclusive")
  endif (XML_MSXML AND XML_XERCESC)
  if (XML_MSXML AND WX_WINDOWS)
    message (FATAL_ERROR "MSXML is not supported for wxWidget under Windows")
  endif (XML_MSXML AND WX_WINDOWS)
else (WIN32)
  option (XML_XERCESC "Set OFF to disable XML import support" ON)
endif (WIN32)
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  option (USE_ASAN "Set ON to add -fsanitize=address to Debug builds" OFF)
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  option (NO_QR "Set ON to disable QR support" OFF)
  if (NOT NO_QR)
    CHECK_INCLUDE_FILE("qrencode.h" HAVE_LIBQRENCODE_DEV)
    if(NOT HAVE_LIBQRENCODE_DEV)
      unset(HAVE_LIBQRENCODE_DEV CACHE)
      message( FATAL_ERROR "libqrencode-dev(el) not installed: install or select NO_QR")
    endif(NOT HAVE_LIBQRENCODE_DEV)
  else (NOT NO_QR)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_QR")
  endif (NOT NO_QR)
else(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if (NOT WIN32 OR WX_WINDOWS)
  if (NOT WIN32)
     # help people with wxWidgets on non-standard installation
     # quick-and-dirty since wxWdigets cmake's support is not yet loaded.
     find_program(PWSHINT_wxconfig wx-config)
     if (NOT PWSHINT_wxconfig)
       message(STATUS
               "Failed to find 'wx-config' executable:\n"
               "   Tip: can be pointed from command-line this way:\n"
               "        $ cmake -D wxWidgets_CONFIG_EXECUTABLE=/path/to/wx-config ."
               )
     endif (NOT PWSHINT_wxconfig)
  endif (NOT WIN32)
  find_package(wxWidgets COMPONENTS adv base core html net REQUIRED)
  include(${wxWidgets_USE_FILE})

  find_package(OpenSSL REQUIRED)
  include_directories(${OPENSSL_INCLUDE_DIR})
endif (NOT WIN32 OR WX_WINDOWS)

if (NOT WIN32)
    if (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
      set( CMAKE_REQUIRED_INCLUDES "/usr/local/include" )
    endif (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
  CHECK_INCLUDE_FILE("uuid/uuid.h" UUID_H)
  if (NOT UUID_H)
    message(FATAL_ERROR
      "uuid.h not found - uuid-dev / libuuid-devel not installed?")
  endif(NOT UUID_H)
  if(NOT NO_YUBI)
    CHECK_LIBRARY_EXISTS(ykpers-1 yk_init "ykpers-1/ykpers.h" HAVE_YKPERS_H)
  endif(NOT NO_YUBI)

  if(NOT HAVE_YKPERS_H)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_YUBI")
    message(STATUS "Yubikey support disabled")
  endif(NOT HAVE_YKPERS_H)
endif (NOT WIN32)

if (XML_XERCESC)
  find_package (XercesC QUIET MODULE)
  if (NOT XercesC_FOUND)
    # FindXercesC module was added to cmake in 3.1. Debian 8 uses 3.0.2
    # If the module wasn't found yet XML is enabled, we set the default
    # values. If this isn't what you want, either configure NO_XML
    # or edit the following values manually.
        if (NOT XercesC_INCLUDE_DIR)
            message (WARNING "XML enabled but XercesC_INCLUDE_DIR unset - using default")
           set (XercesC_INCLUDE_DIR "/usr/include")
        endif ()
        if (NOT XercesC_LIBRARY)
            message (WARNING "XML enabled but XercesC_LIBRARY unset - using default")
           set (XercesC_LIBRARY "xerces-c")
        endif ()
  endif(NOT XercesC_FOUND)
  include_directories( ${XercesC_INCLUDE_DIR} )
  add_definitions ("-DUSE_XML_LIBRARY=XERCES")
endif (XML_XERCESC)

if (XML_MSXML)
  add_definitions ("-DUSE_XML_LIBRARY=MSXML")
  set (MSXML_LIB "msxml6")
endif (XML_MSXML)

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
# Debug build looks for dlls with _D postfix, this provides it:
set (CMAKE_DEBUG_POSTFIX "_D")
# This copies dlls to same directory as exec upon 'install' build
set (CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})

set_property (GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} /MP /GS /analyze- /W4 /Gy /Zc:wchar_t /Zi /Gm-
  /Ob1 /fp:precise /D WIN32 /D _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1 /D _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT=1 /D WINVER=0x0601 /D _USING_V110_SDK71_ /D UNICODE /D _UNICODE /GF /WX- /Zc:forScope /RTC1 /Gd /Oy-")

if (NO_YUBI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_YUBI")
endif (NO_YUBI)


if (WX_WINDOWS)
   set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D __WX__")
endif (WX_WINDOWS)

set(CMAKE_CXX_FLAGS_DEBUG " /Od  /D _DEBUG /WX- /Gd /Oy- /Ob0 /Od /MTd")

set(CMAKE_CXX_FLAGS_RELEASE "/MT /O2 /Ob2 /D NDEBUG  /W4 /Zc:wchar_t  /Od /WX-")

else ()
set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -fPIC -std=gnu++11 -DUNICODE -DWCHAR_INCOMPATIBLE_XMLCH ${CMAKE_WXWINDOWS_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -DDEBUG")
if (USE_ASAN)
  set(CMAKE_CXX_FLAGS_DEBUG
    "${CMAKE_CXX_FLAGS_DEBUG} -gdwarf-4 -O0 -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize=address")
endif (USE_ASAN)
endif (MSVC)

if (NOT NO_GTEST)
   # Download and unpack googletest at configure time
   # See https://crascit.com/2015/07/25/cmake-gtest/
   configure_file(Misc/CMakeLists.gtest.txt.in googletest-download/CMakeLists.txt)
   execute_process(COMMAND "${CMAKE_COMMAND}" -G "${CMAKE_GENERATOR}" .
       WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download" )
   execute_process(COMMAND "${CMAKE_COMMAND}" --build .
       WORKING_DIRECTORY "${CMAKE_BINARY_DIR}/googletest-download" )

   # Prevent GoogleTest from overriding our compiler/linker options
   # when building with Visual Studio
   set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
   # Set some other gtest configurations:
   set(BUILD_GMOCK OFF CACHE BOOL "" FORCE)
   set(BUILD_GTEST ON CACHE BOOL "" FORCE)
   set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)

   # Add googletest directly to our build. This adds
   # the following targets: gtest, gtest_main, gmock
   # and gmock_main
   add_subdirectory("${CMAKE_BINARY_DIR}/googletest-src"
                    "${CMAKE_BINARY_DIR}/googletest-build")

   include_directories("${gtest_SOURCE_DIR}/include"
#                       "${gmock_SOURCE_DIR}/include"
                       )
endif(NOT NO_GTEST)

if (WIN32 AND NOT WX_WINDOWS)
  set(VERSION_IN "${PROJECT_SOURCE_DIR}/src/ui/Windows/version.in")
else (WIN32 AND NOT WX_WINDOWS)
  set(VERSION_IN "${PROJECT_SOURCE_DIR}/src/ui/wxWidgets/version.in")
endif (WIN32 AND NOT WX_WINDOWS)

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

if (NOT MSVC)
   # Following is because (a) -O3 breaks the test and application, and
   # (b) -O3 is the default for cmake
   set (CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
endif (NOT MSVC)

if (WIN32)
   set (CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif (WIN32)

# Subdirectories

add_subdirectory (src/os) # os-specific library
add_subdirectory (src/core) # core library
if (NOT NO_GTEST)
   add_subdirectory (src/test) # tests (gtest framework)
endif(NOT NO_GTEST)
add_subdirectory (help) # online help
if (WIN32 AND NOT WX_WINDOWS)
  add_subdirectory (src/ui/Windows)
  add_subdirectory (src/ui/Windows/I18N)
  add_subdirectory (src/Tools/Windows/I18N/ResText)
  add_subdirectory (src/Tools/Windows/I18N/ResPWSL)
else()
  add_subdirectory (src/ui/wxWidgets)
  add_subdirectory(src/ui/cli "./cli")
  if (NOT WIN32)
    # TBD - handle I18N under WX_WINDOWS.
    find_package(Gettext QUIET)
    if (GETTEXT_FOUND)
      add_subdirectory (src/ui/wxWidgets/I18N)
    else (GETTEXT_FOUND)
      message("Gettext support not found, skipping I18N build")
    endif(GETTEXT_FOUND)
  endif (NOT WIN32)
endif()

if (NOT WIN32)
  add_executable(pwsafe ${PWSAFE_SRCS})
  if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  # squash lintian hardening-no-relro warning, probably good for all Linuxen
  set_target_properties(pwsafe PROPERTIES LINK_FLAGS "-z relro")
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
else (NOT WIN32)
   add_executable(pwsafe WIN32 ${PWSAFE_SRCS})
   if (NOT WX_WINDOWS)
      set (CMAKE_MFC_FLAG 1)
      set (PWS_DLL_LIBS "pws_at" "pws_osk")
      set_target_properties(pwsafe PROPERTIES COMPILE_DEFINITIONS
                            _BIND_TO_CURRENT_CRT_VERSION,_BIND_TO_CURRENT_MFC_VERSION
                            LINK_FLAGS "/ENTRY:\"wWinMainCRTStartup\"")
   else (NOT WX_WINDOWS)
#      set_target_properties(pwsafe PROPERTIES COMPILE_DEFINITIONS
#                            _BIND_TO_CURRENT_CRT_VERSION
#                            LINK_FLAGS "/ENTRY:\"wWinMainCRTStartup\"")
   endif (NOT WX_WINDOWS)
endif (NOT WIN32)
target_link_libraries(pwsafe core os)
if (XML_XERCESC)
  target_link_libraries(pwsafe ${XercesC_LIBRARY})
endif (XML_XERCESC)

if (APPLE)
    FIND_LIBRARY(QUARTZCORE_LIBS QuartzCore)
    target_link_libraries(pwsafe ${wxWidgets_LIBRARIES} ${CMAKE_REQUIRED_LIBRARIES} ${QUARTZCORE_LIBS} curl ${OPENSSL_SSL_LIBRARY})
elseif (WIN32)
  if (NOT NO_YUBI)
   set (YUBILIB "YkLib22")
  endif ()
  target_link_libraries(pwsafe ${wxWidgets_LIBRARIES} 
      ${YUBILIB} ${PWS_DLL_LIBS} version Rpcrt4 htmlhelp SetupAPI Hid usp10
      Dbghelp Urlmon gdiplus ${MSXML_LIB} ${CMAKE_REQUIRED_LIBRARIES})
else ()
  if (NOT NO_QR)
    target_link_libraries(pwsafe qrencode  ${CMAKE_REQUIRED_LIBRARIES})
  endif (NOT NO_QR)
  target_link_libraries(pwsafe ${wxWidgets_LIBRARIES} uuid Xtst X11 curl ${OPENSSL_SSL_LIBRARY}
  ${CMAKE_REQUIRED_LIBRARIES})
endif()

if (IPO_SUPPORTED)
  set_property(TARGET pwsafe PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE True)
endif (IPO_SUPPORTED)

# Installation stuff (for 'make install', but mainly for 'make package')
install (TARGETS pwsafe RUNTIME DESTINATION "bin")


if (NOT WIN32)
  install (DIRECTORY xml DESTINATION "share/passwordsafe/")
  ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_BINARY_DIR}/pwsafe.1.gz
                     COMMAND ${GZIP} ARGS -9 -n -c ${PROJECT_SOURCE_DIR}/docs/pwsafe.1 > pwsafe.1.gz
                     WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
  ADD_CUSTOM_TARGET(manpage ALL DEPENDS ${CMAKE_BINARY_DIR}/pwsafe.1.gz)
  install (FILES ${CMAKE_BINARY_DIR}/pwsafe.1.gz DESTINATION "share/man/man1") 
  install (FILES "install/desktop/pwsafe.desktop" DESTINATION "share/applications")
  install (FILES "install/graphics/pwsafe.png" DESTINATION "share/pixmaps")
  install (DIRECTORY "${CMAKE_BINARY_DIR}/src/ui/wxWidgets/I18N/mos/"
           DESTINATION "share/locale")

  install (DIRECTORY ${PROJECT_BINARY_DIR}/help/
           DESTINATION "share/passwordsafe/help"
           FILES_MATCHING PATTERN "*.zip"
           PATTERN "CMakeFiles" EXCLUDE)

else ()
  install (FILES "xml/pwsafe.xsd" DESTINATION "bin")
  install (FILES "xml/pwsafe_filter.xsd" DESTINATION "bin")
endif (NOT WIN32)

# uninstall target
if(NOT TARGET uninstall)
  configure_file(
      "${CMAKE_CURRENT_SOURCE_DIR}/install/cmake_uninstall.cmake.in"
      "${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake"
      IMMEDIATE @ONLY)

  add_custom_target(uninstall
      COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/cmake_uninstall.cmake)
endif()


###################################################################################################
#
# P A C K A G I N G
#
###################################################################################################

#
# Major version number of software version (e.g. 1.08-BETA).
#                                                ^
set (CPACK_PACKAGE_VERSION_MAJOR "${pwsafe_VERSION_MAJOR}")

#
# Minor version number of software version (e.g. 1.08-BETA), which should consist of two digits.
#                                                   ^
string(LENGTH ${pwsafe_VERSION_MINOR} VERSION_MINOR_LENGTH)
if (VERSION_MINOR_LENGTH GREATER 1)
  set (CPACK_PACKAGE_VERSION_MINOR "${pwsafe_VERSION_MINOR}")
else ()
  set (CPACK_PACKAGE_VERSION_MINOR "0${pwsafe_VERSION_MINOR}")
endif ()

#
# Mandatory part is complete. What follows is optional.
#
set (CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION_MAJOR}.${CPACK_PACKAGE_VERSION_MINOR}")

#
# Revision number of software version (e.g. 1.08.1-BETA).
#                                                ^
if (pwsafe_REVISION)
  set (CPACK_PACKAGE_VERSION_PATCH "${pwsafe_REVISION}")
  set (CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}.${CPACK_PACKAGE_VERSION_PATCH}")
endif (pwsafe_REVISION)

#
# Special build tag of software version (e.g. 1.08.1-BETA).
#                                                    ^
if (pwsafe_SPECIALBUILD)
  set (CPACK_PACKAGE_VERSION "${CPACK_PACKAGE_VERSION}-${pwsafe_SPECIALBUILD}")
endif (pwsafe_SPECIALBUILD)

#
# Miscellaneous package information.
#
set (CPACK_PACKAGE_CONTACT "Rony Shapiro <ronys@pwsafe.org>")
set (CPACK_PACKAGE_VENDOR "Rony Shapiro")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Password Safe is a password database utility.")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/install/description.txt")
set (CPACK_STRIP_FILES "TRUE")

#
# Package file name is project name.
#
if (NOT CPACK_PACKAGE_NAME)
  string(TOLOWER "${PROJECT_NAME}" CPACK_PACKAGE_NAME)
endif ()

#
# Determine the architecture (amd64, i386) with command 'uname -m'
#
if (NOT CPACK_DEBIAN_PACKAGE_ARCHITECTURE)
  find_program(UNAME_EXECUTABLE uname)
  mark_as_advanced(UNAME_EXECUTABLE)

  if (NOT UNAME_EXECUTABLE)
    message(STATUS "Cannot find 'uname' in your path. Set architecture to default i386.")
    set (CPACK_DEBIAN_PACKAGE_ARCHITECTURE i386)
  else ()
    execute_process(COMMAND "${UNAME_EXECUTABLE}" -m
                    OUTPUT_VARIABLE CPACK_DEBIAN_PACKAGE_ARCHITECTURE
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    if (CPACK_DEBIAN_PACKAGE_ARCHITECTURE MATCHES "x86_64")
      set (CPACK_DEBIAN_PACKAGE_ARCHITECTURE amd64)
    endif ()
  endif ()
endif ()

#
# Determine name of distribution (Ubuntu, LinuxMint, ...)
#

# Find out what linux distro we're on... currently only used for 
# debian based distros
find_program(LSB_RELEASE_EXECUTABLE lsb_release
  DOC "Linux Standard Base and Distribution command line query client")
mark_as_advanced(LSB_RELEASE_EXECUTABLE)

if (LSB_RELEASE_EXECUTABLE)
  execute_process(COMMAND ${LSB_RELEASE_EXECUTABLE} -is
    OUTPUT_VARIABLE RELEASE_ID_SHORT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  execute_process(COMMAND ${LSB_RELEASE_EXECUTABLE} -rs
    OUTPUT_VARIABLE RELEASE_NUM_SHORT
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
else(LSB_RELEASE_EXECUTABLE) # look for /etc/os-release
  set (OS_RELEASE_FILE "/etc/os-release")
  if (EXISTS ${OS_RELEASE_FILE})
    file ( STRINGS ${OS_RELEASE_FILE} OS_RELEASE_LIST )
    foreach ( OS_RELEASE_ITEM IN ITEMS ${OS_RELEASE_LIST} )
      string (REGEX REPLACE "\;" "" OS_RELEASE_ITEM ${OS_RELEASE_ITEM})
      if (${OS_RELEASE_ITEM} MATCHES "^[ ]*NAME[ ]*=[ ]*(.+)")
        string (REGEX REPLACE ".*=[ ]*(.+)" "\\1"
          RELEASE_ID_SHORT ${OS_RELEASE_ITEM})
      elseif (${OS_RELEASE_ITEM} MATCHES "^[ ]*VERSION_ID[ ]*=[ ]*([0-9]+)")
        string (REGEX REPLACE ".*=[ ]*([0-9]+)" "\\1"
          RELEASE_NUM_SHORT ${OS_RELEASE_ITEM})
      endif ()
      endforeach (OS_RELEASE_ITEM)
  endif (EXISTS ${OS_RELEASE_FILE})
endif(LSB_RELEASE_EXECUTABLE)

if (RELEASE_ID_SHORT)
  string(TOLOWER ${RELEASE_ID_SHORT} DISTRO_NAME)
endif (RELEASE_ID_SHORT)

#
# Determine major distroversion number from output (e.g. '18.04') of command 'lsb_release -rs'
# or from VERSION_ID of /etc/os-release
#
# Get the first number up to the separator ('.' or '-').
# The remaining characters doesn't matter.
#
if (${RELEASE_NUM_SHORT} MATCHES "^([0-9]+)([\\.\\-]*.)*$")
  if (CMAKE_MATCH_COUNT GREATER 0)
    set (DISTRO_MAJOR_VERSION ${CMAKE_MATCH_1})
  else ()
    set (DISTRO_MAJOR_VERSION ${RELEASE_NUM_SHORT})
  endif ()
else ()
  set (DISTRO_MAJOR_VERSION ${RELEASE_NUM_SHORT})
endif ()

#
# Build name of package file (e.g. passwordsafe-ubuntu18-1.08.1-BETA-amd64.deb)
#
set (CPACK_PACKAGE_FILE_NAME ${CPACK_PACKAGE_NAME}-${DISTRO_NAME}${DISTRO_MAJOR_VERSION}-${CPACK_PACKAGE_VERSION}-${CPACK_DEBIAN_PACKAGE_ARCHITECTURE})

if (EXISTS "/etc/debian_version")
## we are on a debian based distro, but we can't make assumptions about which
## we will do general deb related setup and relegate deps to specific distros 

ADD_CUSTOM_COMMAND(OUTPUT ${CMAKE_BINARY_DIR}/changelog.Debian.gz
                   COMMAND /bin/gzip ARGS -9 -n -k 
                   "${PROJECT_SOURCE_DIR}/install/deb/changelog.Debian"
                   COMMAND /bin/mv ARGS 
                   "${PROJECT_SOURCE_DIR}/install/deb/changelog.Debian.gz" 
                   ${CMAKE_BINARY_DIR}/changelog.Debian.gz)

ADD_CUSTOM_TARGET(changelog ALL DEPENDS ${CMAKE_BINARY_DIR}/changelog.Debian.gz)
install (FILES ${CMAKE_BINARY_DIR}/changelog.Debian.gz 
         DESTINATION "share/doc/passwordsafe")
install (FILES "install/deb/copyright.debian" DESTINATION "share/doc/passwordsafe"
         RENAME "copyright")

set (CPACK_DEBIAN_PACKAGE_SECTION "utils")
set (CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set (CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://pwsafe.org/")
set (CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION "TRUE")
set (CPACK_DEBIAN_PACKAGE_CONFLICTS "passwordsafe-common") # Debian package maintainer split stuff

if (${RELEASE_ID_SHORT} STREQUAL "Ubuntu")
  ## Ubuntu-specific
  if (NOT ${RELEASE_NUM_SHORT} VERSION_LESS "18.0")
    set (LIBCURL "libcurl4 (>= 7.58.0-2)")
  else (NOT ${RELEASE_NUM_SHORT} VERSION_LESS "18.0")
    set (LIBCURL "libcurl3 (>= 7.47.0)")
  endif (NOT ${RELEASE_NUM_SHORT} VERSION_LESS "18.0")
  string(REGEX REPLACE "([0-9]+)\\.([0-9]+)\\.[0-9]+" "\\1.\\2" XercesC_VER ${XercesC_VERSION})
  set (CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.11.1-0ubuntu7.2), ${LIBCURL}, libuuid1 (>= 2.17.2-0ubuntu1), libwxgtk3.0-0v5 (>= 3.0.0-2) | libwxgtk3.0-0 (>= 3.0.0-2), libxtst6 (>= 2:1.1.0-2), libxerces-c${XercesC_VER} (>= 3.1.0-1), libykpers-1-1 (>= 1.7.0-1), libqrencode3 (>= 3.4.4-1)")
endif (${RELEASE_ID_SHORT} STREQUAL "Ubuntu")

if (${RELEASE_ID_SHORT} STREQUAL "Debian")
  ## Debian-specific:
  set (CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.7-18lenny4), libcurl3 (>= 7.52.1-5), libuuid1 (>= 1.41.3-1), libwxgtk3.0-0 (>= 3.0.0-2) | libwxgtk3.0-0v5 (>= 3.0.0-2), libxtst6 (>= 2:1.0.3-1), libxerces-c3.1 (>= 3.1.1-1+b1), libykpers-1-1 (>= 1.7.0-1), libqrencode3 (>= 3.4.4-1+b2)")
  set (CPACK_DEBIAN_PACKAGE_RECOMMENDS "xvkbd (>= 3.3-1+b1)")
endif (${RELEASE_ID_SHORT} STREQUAL "Debian")

if (${RELEASE_ID_SHORT} STREQUAL "LinuxMint")
  ## LinuxMint-specific:
  if (NOT ${RELEASE_NUM_SHORT} VERSION_LESS "19.1")
    set (LIBCURL "libcurl4 (>= 7.58.0)")
  else (NOT ${RELEASE_NUM_SHORT} VERSION_LESS "19.1")
    set (LIBCURL "libcurl3 (>= 7.47.0)")
  endif (NOT ${RELEASE_NUM_SHORT} VERSION_LESS "19.1")
  set (CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.11.1-0ubuntu7.2), ${LIBCURL}, libuuid1 (>= 2.17.2-0ubuntu1), libwxgtk3.0-0v5 (>= 3.0.0-2) | libwxgtk3.0-0 (>= 3.0.0-2), libxtst6 (>= 2:1.1.0-2), libxerces-c3.2 (>= 3.1.0-1), libykpers-1-1 (>= 1.7.0), libqrencode3 (>= 3.4.4)")
endif (${RELEASE_ID_SHORT} STREQUAL "LinuxMint")

set (CPACK_DEBIAN_PACKAGE_DESCRIPTION "Password Safe is a password database utility.\n Like many other such products, commercial and otherwise,\n it stores your passwords in an encrypted file.")

set (CPACK_DEBIAN_PACKAGE_CONTROL_EXTRA 
  "${CMAKE_CURRENT_SOURCE_DIR}/install/deb/prerm;${CMAKE_CURRENT_SOURCE_DIR}/install/deb/postrm")
endif (EXISTS "/etc/debian_version")

## RPM-specific
set (CPACK_RPM_PACKAGE_REQUIRES "wxBase3, wxGTK3, xerces-c, ykpers, qrencode-libs")
set (CPACK_RPM_PACKAGE_SUGGESTS "xvkbd")
set (CPACK_RPM_PACKAGE_URL "https://pwsafe.org/")
set (CPACK_RPM_PACKAGE_LICENSE "Artistic2.0")
set (CPACK_RPM_PACKAGE_GROUP "Applications/Utils")
set (CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION "/usr/share/man" "/usr/share/man/man1" "/usr/share/locale" "/usr/share/locale/da" "/usr/share/locale/da/LC_MESSAGES" "/usr/share/locale/de" "/usr/share/locale/de/LC_MESSAGES" "/usr/share/locale/es" "/usr/share/locale/es/LC_MESSAGES" "/usr/share/locale/fr" "/usr/share/locale/fr/LC_MESSAGES" "/usr/share/locale/it" "/usr/share/locale/it/LC_MESSAGES" "/usr/share/locale/ko" "/usr/share/locale/ko/LC_MESSAGES" "/usr/share/locale/nl" "/usr/share/locale/nl/LC_MESSAGES" "/usr/share/locale/pl" "/usr/share/locale/pl/LC_MESSAGES" "/usr/share/locale/ru" "/usr/share/locale/ru/LC_MESSAGES" "/usr/share/locale/sv" "/usr/share/locale/sv/LC_MESSAGES" "/usr/share/locale/zh" "/usr/share/locale/zh/LC_MESSAGES" "/usr/share/applications" "/usr/share/pixmaps")
include(CPack)
### End of packaging section
### End of CMakeLists.txt
