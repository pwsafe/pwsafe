if (NOT MSVC)
  return()
endif ()

# When using Ninja and MSVC, invalid link arguments are not considered errors.
# https://gitlab.kitware.com/cmake/cmake/-/issues/22023
set(HARDEN_MSVC_ORIGINAL_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS})
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /WX")

harden_add_compile_option(/sdl HARDEN_COMPILE_SDL FALSE)

harden_add_link_option(/NXCOMPAT HARDEN_LINK_NXCOMPAT FALSE)
harden_add_link_option(/DYNAMICBASE HARDEN_LINK_DYNAMICBASE FALSE)

if (MSVC_VERSION LESS 1900)
  return()
endif ()

option(USE_CONTROL_FLOW_GUARD "Enable control flow guard" ON)

if (USE_CONTROL_FLOW_GUARD)
  # https://learn.microsoft.com/en-us/cpp/build/reference/guard-enable-control-flow-guard
  harden_add_compile_option(/guard:cf HARDEN_COMPILE_GUARD_CF TRUE)
  harden_add_compile_option(/guard:ehcont HARDEN_COMPILE_GUARD_EHCONT TRUE)
  harden_add_link_option(/guard:cf HARDEN_COMPILE_GUARD_CF TRUE)
  harden_add_link_option(/guard:ehcont HARDEN_LINK_GUARD_EHCONT TRUE)
endif ()

harden_add_link_option(/CETCOMPAT HARDEN_LINK_CETCOMPAT FALSE)

option(USE_SPECTRE_MITIGATION "Enable Spectre mitigation" ON)
set(SPECTRE_MITIGATION_FLAG "/Qspectre" CACHE STRING "MSVC Spectre mitigation flag")
set_property(CACHE SPECTRE_MITIGATION_FLAG PROPERTY STRINGS /Qspectre /Qspectre-load /Qspectre-load-cf)

if(USE_SPECTRE_MITIGATION)
  harden_add_compile_option("${SPECTRE_MITIGATION_FLAG}" HARDEN_COMPILE_SPECTRE TRUE)
endif()

set(CMAKE_EXE_LINKER_FLAGS ${HARDEN_MSVC_ORIGINAL_EXE_LINKER_FLAGS})
