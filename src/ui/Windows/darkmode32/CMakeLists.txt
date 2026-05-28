add_library(darkmode32plus STATIC
  DarkMessageBox.cpp
  DarkMode.cpp
  DMSubclass.cpp
  SysColorHook.cpp
  DarkMode.h
  DMSubclass.h
  IatHook.h
  ModuleHelper.h
  SysColorHook.h
  UAHMenuBar.h
  Version.h
  WinVerHelper.h
)

target_include_directories(darkmode32plus PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

target_compile_definitions(darkmode32plus PRIVATE
  UNICODE
  _UNICODE
)

target_link_libraries(darkmode32plus PRIVATE
  comctl32
  dwmapi
  gdi32
  uxtheme
)
