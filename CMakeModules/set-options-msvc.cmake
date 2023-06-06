# Assume that we're either MSVC or a Unix-like
if (NOT MSVC)
  return()
endif ()

add_compile_options(/diagnostics:caret)
add_compile_options(/EHsc /fp:precise /Gy /GF /W4 /WX- /Zi)

# Language standard options
add_compile_options(/utf-8 /volatile:iso /Zc:__cplusplus /Zc:inline /Zc:lambda)

# Some of the Windows-specific code needs updating before this can be enabled.
# https://learn.microsoft.com/en-us/cpp/build/reference/permissive-standards-conformance
#add_compile_options(/permissive- /Zc:gotoScope-)

# Set the runtime library
if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.15)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
else()
  add_compile_options("/MT$<$<CONFIG:Debug>:d>")
endif()

# Debug build
add_compile_options("$<$<CONFIG:DEBUG>:/Od;/Oy-;/RTC1>")

# Not Debug build
add_compile_options("$<$<NOT:$<CONFIG:DEBUG>>:/Gw;/O2;/Oi>")

set(CMAKE_MFC_FLAG 1)  # Static MFC

include_directories(
    ${VC_IncludePath}
    ${WindowsSDK_IncludePath}
    ${FrameworkSDKDir}/include
)



add_compile_definitions(WIN32
  _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES=1
  _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT=1
  _SILENCE_CXX17_CODECVT_HEADER_DEPRECATION_WARNING
  WINVER=0x0a00
  _WINDOWS
  _UNICODE
  _AFX
  _MFC
)

if(USE_ASAN)
  add_compile_options(/fsanitize=address)
endif()

if(USE_UBSAN)
  message(FATAL_ERROR "UBSAN is not supported on MSVC")
endif()

# Linker options
add_link_options("$<$<NOT:$<CONFIG:DEBUG>>:/OPT:REF;/OPT:ICF=3>")

if (CMAKE_GENERATOR MATCHES "Visual Studio")
  add_compile_options(/MP)

  # Disable incremental LTCG.
  # https://gitlab.kitware.com/cmake/cmake/-/issues/20484
  if (USE_INTERPROCEDURAL_OPTIMIZATION)
    add_link_options("$<$<NOT:$<CONFIG:DEBUG>>:/LTCG>")
  endif ()
endif ()
