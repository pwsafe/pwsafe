# Create an inteface library called harden_interface that
# contains compiler and linker options meant to harden the
# resulting binaries.  To use it with a given target,
#   target_link_libraries(<target> PRIVATE harden_interface)

include(CheckCXXCompilerFlag)

function(harden_wrap_option FLAG_VAR FLAG RELEASE_ONLY)
  if  (RELEASE_ONLY)
    set(${FLAG_VAR} "$<$<NOT:$<CONFIG:DEBUG>>:${FLAG}>" PARENT_SCOPE)
  else ()
    set(${FLAG_VAR} "${FLAG}" PARENT_SCOPE)
  endif ()
endfunction()

function(harden_add_compile_definition FLAG RELEASE_ONLY)
  message(VERBOSE "Setting compile definition hardening flag ${FLAG}")

  harden_wrap_option(WRAPPED_FLAG "${FLAG}" "${RELEASE_ONLY}")
  target_compile_definitions(harden_interface INTERFACE "${WRAPPED_FLAG}")
endfunction()

function(harden_add_compile_option FLAG FLAG_NAME RELEASE_ONLY)
  unset(OLD_FLAGS)
  if (RELEASE_ONLY)
    set(OLD_FLAGS "${CMAKE_TRY_COMPILE_CONFIGURATION}")
    set(CMAKE_TRY_COMPILE_CONFIGURATION Release)
  endif ()
  check_cxx_compiler_flag("${FLAG}" "${FLAG_NAME}")
  if (OLD_FLAGS)
    set(CMAKE_TRY_COMPILE_CONFIGURATION "${OLD_FLAGS}")
  endif ()

  if (${FLAG_NAME})
    message(VERBOSE "Enabling compile hardening flag ${FLAG}")

    harden_wrap_option(WRAPPED_FLAG "${FLAG}" "${RELEASE_ONLY}")
    target_compile_options(harden_interface INTERFACE "${WRAPPED_FLAG}")
  endif ()
endfunction()

if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.18)
  include(CheckLinkerFlag)

  function(harden_add_link_option FLAG FLAG_NAME RELEASE_ONLY)
    unset(OLD_FLAGS)
    set(CMAKE_TRY_COMPILE_CONFIGURATION Release)
    if (RELEASE_ONLY)
      set(OLD_FLAGS "${CMAKE_TRY_COMPILE_CONFIGURATION}")
    endif ()
    check_linker_flag(CXX "${FLAG}" "${FLAG_NAME}")
    if (OLD_FLAGS)
      set(CMAKE_TRY_COMPILE_CONFIGURATION "${OLD_FLAGS}")
    endif ()

    if (${FLAG_NAME})
      message(VERBOSE "Enabling link hardening flag ${FLAG}")

      harden_wrap_option(WRAPPED_FLAG "${FLAG}" "${RELEASE_ONLY}")
      target_link_options(harden_interface INTERFACE "${WRAPPED_FLAG}")
      endif ()
  endfunction()
endif ()

add_library(harden_interface INTERFACE)

if (MSVC)
  # Toolsets with MSVC-like flags
  include("${CMAKE_CURRENT_LIST_DIR}/harden/harden-msvc.cmake")
else ()
  # Toolsets with GCC-like flags
  include("${CMAKE_CURRENT_LIST_DIR}/harden/harden-gcc.cmake")
endif ()
