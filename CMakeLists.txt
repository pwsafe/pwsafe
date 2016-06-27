if (WIN32)
  cmake_minimum_required (VERSION 3.5.1)
else (WIN32)
  cmake_minimum_required (VERSION 2.8)
endif (WIN32)
project (passwordsafe CXX C)

# prerequisites
include(CheckLibraryExists)
include(CheckIncludeFile)

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
  set (GTEST_ROOT "/usr/src/gtest/build"
    CACHE PATH "The root directory of the gtest install prefix")
else(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  set (GTEST_MSVC_SEARCH "MT")
  set (GTEST_ROOT ""
    CACHE PATH "The root directory of the gtest install prefix")
endif(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
find_package(GTest REQUIRED)

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
endif (NOT WIN32 OR WX_WINDOWS)

if (NOT WIN32)
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
# Debug build looks for dlls with _D postfix, this provides it:
set (CMAKE_DEBUG_POSTFIX "_D")
# This copies dlls to same directory as exec upon 'install' build
set (CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR})

set_property (GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} /MP /GS /analyze- /W4 /Gy /Zc:wchar_t /Zi /Gm-
  /O2 /Ob1 /fp:precise /D WIN32 /D _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1 /D _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT=1 /D WINVER=0x0501 /D _USING_V110_SDK71_ /D UNICODE /D _UNICODE /GF /WX- /Zc:forScope /arch:IA32 /RTC1 /Gd /Oy-")

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
add_subdirectory (src/test) # tests (gtest framework)
add_subdirectory (help) # online help
if (WIN32 AND NOT WX_WINDOWS)
  add_subdirectory (src/ui/Windows)
  add_subdirectory (src/ui/Windows/I18N)
  add_subdirectory (src/Tools/Windows/I18N/ResText)
  add_subdirectory (src/Tools/Windows/I18N/ResPWSL)
else()
  add_subdirectory (src/ui/wxWidgets)
  add_subdirectory(src/ui/cli EXCLUDE_FROM_ALL)
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
set_property(TARGET pwsafe PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE True)
target_link_libraries(pwsafe core os)
if (XML_XERCESC)
  target_link_libraries(pwsafe ${XercesC_LIBRARY})
endif (XML_XERCESC)

if (APPLE)
    target_link_libraries(pwsafe ${wxWidgets_LIBRARIES} ${CMAKE_REQUIRED_LIBRARIES})
elseif (WIN32)
  if (NOT NO_YUBI)
   set (YUBILIB "YkLib22")
  endif ()
  target_link_libraries(pwsafe ${wxWidgets_LIBRARIES} 
      ${YUBILIB} ${PWS_DLL_LIBS} version Rpcrt4 htmlhelp SetupAPI Hid
      Dbghelp Urlmon ${MSXML_LIB} ${CMAKE_REQUIRED_LIBRARIES})
else ()
  target_link_libraries(pwsafe ${wxWidgets_LIBRARIES} uuid Xtst X11
  ${CMAKE_REQUIRED_LIBRARIES})
endif()

# Installation stuff (for 'make install', but mainly for 'make package')
install (TARGETS pwsafe RUNTIME DESTINATION "bin")


if (NOT WIN32)
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
   install (FILES "install/deb/copyright.debian" DESTINATION "share/doc/passwordsafe"
            RENAME "copyright")
else ()
   install (FILES "xml/pwsafe.xsd" DESTINATION "bin")
   install (FILES "xml/pwsafe_filter.xsd" DESTINATION "bin")
endif (NOT WIN32)

# Packaging stuff:
## Common:
set (CPACK_PACKAGE_VERSION_MAJOR "${pwsafe_VERSION_MAJOR}")
set (CPACK_PACKAGE_VERSION_MAJOR "${pwsafe_VERSION_MINOR}")
if (pwsafe_REVISION)
   set (CPACK_PACKAGE_VERSION_PATCH "${pwsafe_REVISION}")
endif (pwsafe_REVISION)
if (pwsafe_SPECIALBUILD)
   set (CPACK_PACKAGE_VERSION "${pwsafe_VERSION}-${pwsafe_SPECIALBUILD}")
else ()
   set (CPACK_PACKAGE_VERSION "${pwsafe_VERSION}")
endif (pwsafe_SPECIALBUILD)
set (CPACK_PACKAGE_CONTACT "Rony Shapiro <ronys@pwsafe.org>")
set (CPACK_PACKAGE_VENDOR "Rony Shapiro")
set (CPACK_PACKAGE_DESCRIPTION_SUMMARY "Password Safe is a password database utility.")
set (CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set (CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/install/description.txt")
set (CPACK_STRIP_FILES "TRUE")

## Debian-specific:
set (CPACK_DEBIAN_PACKAGE_SECTION "utils")
set (CPACK_DEBIAN_PACKAGE_PRIORITY "optional")
set (CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://pwsafe.org/")
set (CPACK_DEBIAN_PACKAGE_CONTROL_STRICT_PERMISSION "TRUE")
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

