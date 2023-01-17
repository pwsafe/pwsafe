# Inspiration from:
#   https://wiki.debian.org/Hardening
#   https://blog.quarkslab.com/clang-hardening-cheat-sheet.html

if (MSVC)
  return()
endif ()


option(HARDEN_USE_VISIBILITY "Use the -fvisibility flag (required for CFI)" OFF)

set(HARDEN_VISIBILITY "hidden" CACHE STRING "The value to use for -fvisibility=")
set_property(CACHE HARDEN_VISIBILITY PROPERTY STRINGS hidden default internal protected)

option(HARDEN_USE_CFI "Enable the Control Flow Integrity (CFI) sanitizer" OFF)

if (HARDEN_USE_VISIBILITY)
  function(harden_gcc_check_visbility_cfi)
    set(VISIBILITY_FLAG "-fvisibility=${HARDEN_VISIBILITY}")

    harden_add_compile_option("${VISIBILITY_FLAG}" HARDEN_COMPILE_VISIBILITY FALSE)

    if (HARDEN_USE_CFI)
      unset(OLD_COMPILE_FLAGS)
      if (CMAKE_REQUIRED_FLAGS)
        set(OLD_COMPILE_FLAGS "${CMAKE_REQUIRED_FLAGS}")
      endif ()
      string(APPEND CMAKE_REQUIRED_FLAGS  "-flto ${VISIBILITY_FLAG}")

      harden_add_compile_option(-fsanitize=cfi HARDEN_COMPILE_SANITIZE_CFI TRUE)

      if (OLD_COMPILE_FLAGS)
        set(CMAKE_REQUIRED_FLAGS "${OLD_COMPILE_FLAGS}")
      else ()
        unset(CMAKE_REQUIRED_FLAGS)
      endif ()
    endif()
  endfunction()
endif ()

harden_add_compile_definition(_FORTIFY_SOURCE=2 TRUE)

harden_add_compile_option(-Wformat HARDEN_COMPILE_WFORMAT FALSE)
harden_add_compile_option(-Wformat-security HARDEN_COMPILE_WFORMAT_SECURITY FALSE)
harden_add_compile_option(-Werror=format-security HARDEN_COMPILE_WERROR_FORMAT_SECURITY FALSE)
harden_add_compile_option(-fstack-clash-protection HARDEN_COMPILE_STACK_CLASH_PROTECTION  FALSE)

option(HARDEN_USE_SAFE_STACK "Enable the SafeStack sanitizer" ON)

if (HARDEN_USE_SAFE_STACK)
  harden_add_compile_option(-fsanitize=safe-stack HARDEN_COMPILE_SANITIZE_SAFE_STACK TRUE)

  if (COMMAND harden_add_link_option)
    harden_add_link_option(-fsanitize=safe-stack HARDEN_LINK_SANITIZE_SAFE_STACK TRUE)
  else ()
    harden_add_compile_option("LINKER:-fsanitize=safe-stack" HARDEN_LINK_SANITIZE_SAFE_STACK TRUE)
  endif ()

  # Invert the individual flags to change unset to true. ("if (<unset> EQUAL <unset>)" is false.)
  if (NOT (NOT HARDEN_COMPILE_SANITIZE_SAFE_STACK) EQUAL (NOT HARDEN_LINK_SANITIZE_SAFE_STACK))
    message(WARNING "Both the compiler and linker need to support SafeStack.")
  endif ()
endif ()

if (COMMAND harden_gcc_check_visbility_cfi)
harden_gcc_check_visbility_cfi()
endif ()

harden_add_compile_option(-fstack-protector-strong HARDEN_COMPILE_STACK_PROTECTOR_STRONG FALSE)
if (NOT HARDEN_COMPILE_STACK_PROTECTOR_STRONG)
  harden_add_compile_option(-fstack-protector HARDEN_COMPILE_STACK_PROTECTOR FALSE)
endif ()

# https://www.redhat.com/en/blog/hardening-elf-binaries-using-relocation-read-only-relro
if (COMMAND harden_add_link_option)
  harden_add_link_option("SHELL:-z relro" HARDEN_LINK_Z_RELRO FALSE)
  harden_add_link_option("SHELL:-z now" HARDEN_LINK_Z_NOW FALSE)
else ()
  harden_add_compile_option("-Wl,-z,relro" HARDEN_LINK_Z_RELRO FALSE)
  harden_add_compile_option("-Wl,-z,now" HARDEN_LINK_Z_NOW FALSE)
endif ()
