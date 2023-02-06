########
# Version configuration:
# version.mfc for Windows MFC build, version.wx for all the rest
# version file has the following format:
# VER_MAJOR = n
# VER_MINOR = n
# VER_REV = n (default 0)
# VER_SPECIAL = text [or not set]
# where VER_MAJOR and VER_MINOR are mandatory, and all except
# VER_SPECIAL must be integers

#######
## Shared code (executed during both build and config)
#######

if (WIN32 AND NOT WX_WINDOWS)
  set (PWS_VERSION_FILE "${PROJECT_SOURCE_DIR}/version.mfc")
else (WIN32 AND NOT WX_WINDOWS)
  set (PWS_VERSION_FILE "${PROJECT_SOURCE_DIR}/version.wx")
endif (WIN32 AND NOT WX_WINDOWS)

if (NOT EXISTS "${PWS_VERSION_FILE}")
  message (FATAL_ERROR "Missing ${PWS_VERSION_FILE} - unable to proceed")
endif (NOT EXISTS "${PWS_VERSION_FILE}")

file ( STRINGS "${PWS_VERSION_FILE}" VERSION_LIST )

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
  elseif (${VERSION_ITEM} MATCHES "^[ ]*VER_SPECIAL[ ]*=[ ]*([^ ]+)")
    string (REGEX REPLACE ".*=[ ]*([^ ]+)" "\\1"
            pwsafe_SPECIALBUILD ${VERSION_ITEM})
  endif ()
endforeach ( VERSION_ITEM )

if (NOT DEFINED pwsafe_VERSION_MAJOR)
  message (FATAL_ERROR "VER_MAJOR undefined in ${PWS_VERSION_FILE} - unable to proceed")
endif (NOT DEFINED pwsafe_VERSION_MAJOR)
if (NOT DEFINED pwsafe_VERSION_MINOR)
  message (FATAL_ERROR "VER_MINOR undefined in ${PWS_VERSION_FILE} - unable to proceed")
endif (NOT DEFINED pwsafe_VERSION_MINOR)

if (NOT pwsafe_REVISION)
  set (pwsafe_REVISION 0)
endif ()

set (pwsafe_VERSION "${pwsafe_VERSION_MAJOR}.${pwsafe_VERSION_MINOR}.${pwsafe_REVISION}")

#######
## Config code
#######

if (NOT PWS_VERSION_BUILD)
  set(PWS_VERSION_OUT "${PROJECT_BINARY_DIR}/version.h")

  if (WIN32 AND NOT WX_WINDOWS)
    set(PWS_VERSION_IN "${PROJECT_SOURCE_DIR}/src/ui/Windows/version.in")
  else (WIN32 AND NOT WX_WINDOWS)
    set(PWS_VERSION_IN "${PROJECT_SOURCE_DIR}/src/ui/wxWidgets/version.in")
  endif (WIN32 AND NOT WX_WINDOWS)

  find_package(Git)

  # Use a target that depends on the output of the command to prevent the
  # command from running more than once.
  # https://gitlab.kitware.com/cmake/cmake/-/issues/16767 We add a dummy
  # output to force the command to run every build.
  # https://www.mattkeeter.com/blog/2018-01-06-versioning/
  add_custom_command(
    OUTPUT
      "${PWS_VERSION_OUT}"
      "${PWS_VERSION_OUT}.nosuchfile"
    DEPENDS
      "${PWS_VERSION_IN}"
      "${PWS_VERSION_FILE}"
    COMMAND "${CMAKE_COMMAND}"
      "-DPWS_BUILD_CONFIG=$<CONFIG>"
      "-DPROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}"
      "-DPWS_VERSION_IN=${PWS_VERSION_IN}"
      "-DPWS_VERSION_OUT=${PWS_VERSION_OUT}"
      "-DGIT_EXECUTABLE=${GIT_EXECUTABLE}"
      -DPWS_VERSION_BUILD=TRUE
      -P "${CMAKE_CURRENT_LIST_FILE}"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    VERBATIM
  )

  add_custom_target(pws_version_intermediate DEPENDS "${PWS_VERSION_OUT}")

  add_library(pws_version INTERFACE)
  add_dependencies(pws_version INTERFACE pws_version_intermediate)

  target_include_directories(pws_version
    INTERFACE "$<BUILD_INTERFACE:${PROJECT_BINARY_DIR}>")
endif ()

#######
## Shared code (executed during both build and config)
#######

if (GIT_EXECUTABLE)
  execute_process(COMMAND "${GIT_EXECUTABLE}" describe --all --always --dirty=+ --long
    RESULT_VARIABLE res
    OUTPUT_VARIABLE pwsafe_VERSTRING
    ERROR_QUIET
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if (res)
    set(pwsafe_VERSTRING "local")
  endif ()

  execute_process(
    COMMAND "${GIT_EXECUTABLE}" show -s --format=%ci HEAD
    OUTPUT_VARIABLE pwsafe_TIMESTAMP
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
    )

  execute_process(
    COMMAND "${GIT_EXECUTABLE}" config --get remote.origin.url
    OUTPUT_VARIABLE pwsafe_REPO_URL
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
  )
else ()
  set(pwsafe_VERSTRING "local")
endif ()

set (PWS_VERSION_MAJOR "${pwsafe_VERSION_MAJOR}")
set (PWS_VERSION_MINOR "${pwsafe_VERSION_MINOR}")
set (PWS_VERSION_PATCH "${pwsafe_REVISION}")

set (PWS_PRODUCTVER "${pwsafe_VERSION_MAJOR}, ${pwsafe_VERSION_MINOR}, ${pwsafe_REVISION}" )
set (PWS_PRODUCTVER_STR "${pwsafe_VERSION_MAJOR}.${pwsafe_VERSION_MINOR}.${pwsafe_REVISION}${pwsafe_SPECIALBUILD} ${pwsafe_VERSTRING}")

set (PWS_FILEVER "${PWS_PRODUCTVER}")
set (PWS_FILEVER_STR "${PWS_PRODUCTVER_STR}")

set (PWS_DESCRIBE_STR "${pwsafe_VERSTRING}")

if (pwsafe_PRIVATEBUILD)
  set (PWS_PRIVATEBUILD_STR "${pwsafe_PRIVATEBUILD}")
endif ()

if (pwsafe_SPECIALBUILD)
  set (PWS_SPECIALBUILD_STR "${pwsafe_SPECIALBUILD}")
endif ()

if (pwsafe_TIMESTAMP)
  set (PWS_TIMESTAMP_STR "${pwsafe_TIMESTAMP}")
endif ()

if (pwsafe_REPO_URL)
  set (PWS_REPO_URL_STR "${pwsafe_REPO_URL}")
endif ()

message(STATUS "Updating version file (${PWS_PRODUCTVER_STR}).")
configure_file (
  "${PWS_VERSION_IN}"
  "${PWS_VERSION_OUT}"
  )

