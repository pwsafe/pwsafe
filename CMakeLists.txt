cmake_minimum_required (VERSION 3.0)
project (pwsafe CXX C)

add_subdirectory (src/os)
add_subdirectory (src/core)
#add_subdirectory (src/test)
# add_subdirectory (help)

include_directories (${PROJECT_SOURCE_DIR}/src/os)
include_directories (${PROJECT_SOURCE_DIR}/src/core)

# Following can be different for windows/wx, just wrap around if/else/endif
set (pwsafe_VERSION_MAJOR 4)
set (pwsafe_VERSION_MINOR 0)
set (pwsafe_REVISION 0)
set (pwsafe_SPECIALBUILD "ALPHA")

execute_process(COMMAND "git" "describe" "--all" "--always" "--dirty=+" "--long"
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  RESULT_VARIABLE res
  OUTPUT_VARIABLE pwsafe_VERSTRING
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE)
if (res)
  set(pwsafe_VERSTRING "local")
endif()

option(DEBUG "Debug symbols and AddressSanitize")

set(INSTALL_BIN_DIR "${CMAKE_INSTALL_PREFIX}/bin" CACHE PATH "Installation directory for executables")
include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckLibraryExists)
include(CheckIncludeFile)
include(CheckCSourceCompiles)
include(CheckCXXSourceCompiles)
enable_testing()

check_include_file(stdint.h    HAVE_STDINT_H)

if(NOT NO_YUBI)
  CHECK_LIBRARY_EXISTS(ykpers-1 yk_init "ykpers-1/ykpers.h" HAVE_YKPERS_H)
endif(NOT NO_YUBI)

set(wxWidgets_CONFIGURATION mswu)
find_package(wxWidgets COMPONENTS adv base core html net REQUIRED)
include(${wxWidgets_USE_FILE})
# FindXercesC module was introduced in cmake 3.1. Debian stable (jessie 8.2) currently provide cmake 3.0.2
# include(FindXercesC)
# find_package(XercesC REQUIRED)
# include_directories( ${XercesC_INCLUDE_DIR} )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -DUSE_XML_LIBRARY=XERCES -DWCHAR_INCOMPATIBLE_XMLCH ${CMAKE_WXWINDOWS_CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG -DDEBUG -gdwarf-4 -O0 -fno-omit-frame-pointer -fno-optimize-sibling-calls -fsanitize=address")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE}")

if(NOT HAVE_YKPERS_H)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNO_YUBI")
  message(STATUS "Yubikey support disabled")
endif(NOT HAVE_YKPERS_H)

if (WIN32)
  set(VERSION_IN "${PROJECT_SOURCE_DIR}/src/ui/Windows/version.in")
else()
  set(VERSION_IN "${PROJECT_SOURCE_DIR}/src/ui/wxWidgets/version.in")
endif()

configure_file (
  ${VERSION_IN}
  ${PROJECT_BINARY_DIR}/version.h
  )

include_directories("${PROJECT_BINARY_DIR}")
include_directories("${PROJECT_SOURCE_DIR}/src"
  "${PROJECT_SOURCE_DIR}/src/core"
  "${PROJECT_SOURCE_DIR}/src/ui/wxWidgets")

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
  list(APPEND PWSAFE_SRCS src/ui/wxWidgets/yubicfg.cpp src/ui/wxWidgets/YubiMixin.cpp)
  list(APPEND PWSAFE_HDRS src/ui/wxWidgets/yubicfg.h src/ui/wxWidgets/YubiMixin.h)
endif(HAVE_YKPERS_H)

list(APPEND PWSAFE_HDRS
    src/ui/wxWidgets/pwsafeapp.h
    src/ui/wxWidgets/passwordsafeframe.h
    src/ui/wxWidgets/about.h
    src/ui/wxWidgets/optionspropsheet.h
    src/ui/wxWidgets/PasswordSafeSearch.h
    src/ui/wxWidgets/ViewReport.h
    src/ui/wxWidgets/guiinfo.h
    src/ui/wxWidgets/helpmap.h
    src/ui/wxWidgets/wxMessages.h
    src/ui/wxWidgets/pwsmenushortcuts.h
    src/ui/wxWidgets/safecombinationchange.h
    src/ui/wxWidgets/safecombinationentry.h
    src/ui/wxWidgets/safecombinationsetup.h
    src/ui/wxWidgets/addeditpropsheet.h
    src/ui/wxWidgets/properties.h
    src/ui/wxWidgets/pwsclip.h
    src/ui/wxWidgets/createshortcutdlg.h
    src/ui/wxWidgets/deleteconfirmation.h
    src/ui/wxWidgets/dragbar.h
    src/ui/wxWidgets/editshortcut.h
    src/ui/wxWidgets/fieldselectiondlg.h
    src/ui/wxWidgets/fieldselectionpanel.h
    src/ui/wxWidgets/wxutils.h
    src/ui/wxWidgets/AdvancedSelectionDlg.h
    src/ui/wxWidgets/CompareDlg.h
    src/ui/wxWidgets/ComparisonGridTable.h
    src/ui/wxWidgets/DbSelectionPanel.h
    src/ui/wxWidgets/ExportTextWarningDlg.h
    src/ui/wxWidgets/ExternalKeyboardButton.h
    src/ui/wxWidgets/ImportTextDlg.h
    src/ui/wxWidgets/ImportXmlDlg.h
    src/ui/wxWidgets/ManagePwdPolicies.h
    src/ui/wxWidgets/MergeDlg.h
    src/ui/wxWidgets/OpenFilePickerValidator.h
    src/ui/wxWidgets/PWSDragBar.h
    src/ui/wxWidgets/PWSgrid.h
    src/ui/wxWidgets/PWSgridtable.h
    src/ui/wxWidgets/PWStree.h
    src/ui/wxWidgets/PasswordPolicy.h
    src/ui/wxWidgets/PwsSync.h
    src/ui/wxWidgets/PwsToolbarButtons.h
    src/ui/wxWidgets/RUEList.h
    src/ui/wxWidgets/RecentDBList.h
    src/ui/wxWidgets/SafeCombinationCtrl.h
    src/ui/wxWidgets/SelectionCriteria.h
    src/ui/wxWidgets/SizeRestrictedPanel.h
    src/ui/wxWidgets/SystemTray.h
    src/ui/wxWidgets/SystemTrayMenuId.h
    src/ui/wxWidgets/safecombinationprompt.h
    src/ui/wxWidgets/TimedTaskChain.h
)


list(APPEND PWSAFE_SRCS
    src/ui/wxWidgets/optionspropsheet.cpp
    src/ui/wxWidgets/CompareDlg.cpp
    src/ui/wxWidgets/ImportTextDlg.cpp
    src/ui/wxWidgets/addeditpropsheet.cpp
    src/ui/wxWidgets/PasswordSafeSearch.cpp
    src/ui/wxWidgets/SystemTray.cpp
    src/ui/wxWidgets/about.cpp
    src/ui/wxWidgets/mainManage.cpp
    src/ui/wxWidgets/guiinfo.cpp
    src/ui/wxWidgets/version.cpp
    src/ui/wxWidgets/pwsclip.cpp
    src/ui/wxWidgets/safecombinationentry.cpp
    src/ui/wxWidgets/mainEdit.cpp
    src/ui/wxWidgets/ViewReport.cpp
    src/ui/wxWidgets/safecombinationprompt.cpp
    src/ui/wxWidgets/passwordsafeframe.cpp
    src/ui/wxWidgets/properties.cpp
    src/ui/wxWidgets/createshortcutdlg.cpp
    src/ui/wxWidgets/deleteconfirmation.cpp
    src/ui/wxWidgets/dragbar.cpp
    src/ui/wxWidgets/editshortcut.cpp
    src/ui/wxWidgets/fieldselectiondlg.cpp
    src/ui/wxWidgets/fieldselectionpanel.cpp
    src/ui/wxWidgets/pwsmenushortcuts.cpp
    src/ui/wxWidgets/safecombinationchange.cpp
    src/ui/wxWidgets/ExternalKeyboardButton.cpp
    src/ui/wxWidgets/PasswordPolicy.cpp
    src/ui/wxWidgets/DbSelectionPanel.cpp
    src/ui/wxWidgets/pwsafeapp.cpp
    src/ui/wxWidgets/safecombinationsetup.cpp
    src/ui/wxWidgets/wxutils.cpp
    src/ui/wxWidgets/AdvancedSelectionDlg.cpp
    src/ui/wxWidgets/ComparisonGridTable.cpp
    src/ui/wxWidgets/ExportTextWarningDlg.cpp
    src/ui/wxWidgets/ImportXmlDlg.cpp
    src/ui/wxWidgets/ManagePwdPolicies.cpp
    src/ui/wxWidgets/MergeDlg.cpp
    src/ui/wxWidgets/OpenFilePickerValidator.cpp
    src/ui/wxWidgets/PWSDragBar.cpp
    src/ui/wxWidgets/PWSgrid.cpp
    src/ui/wxWidgets/PWSgridtable.cpp
    src/ui/wxWidgets/PWStree.cpp
    src/ui/wxWidgets/PwsSync.cpp
    src/ui/wxWidgets/RUEList.cpp
    src/ui/wxWidgets/SafeCombinationCtrl.cpp
    src/ui/wxWidgets/SelectionCriteria.cpp
    src/ui/wxWidgets/SizeRestrictedPanel.cpp
    src/ui/wxWidgets/TimedTaskChain.cpp
)

add_executable(pwsafe ${PWSAFE_SRCS})
set_property(TARGET pwsafe PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE True)
target_link_libraries(pwsafe core os)
target_link_libraries(pwsafe ${wxWidgets_LIBRARIES} uuid Xtst X11 ${CMAKE_REQUIRED_LIBRARIES} ${XercesC_LIBRARY})

