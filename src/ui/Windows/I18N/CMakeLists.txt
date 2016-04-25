# CMakeLists.txt for pwsafe I18N support:
#
# Supports making language-specific DLLs
# Supports updating language-specific .po files
# Supports creating pwsafe.pot file as the basis for a new translation
#

set (LANGS "ca" "de" "dk" "es" "fr" "hu" "it" "kr" "nl" "pl" "ru" "sv"
     "tr" "zh")

set (TOOLS_BIN_DIR "${CMAKE_BINARY_DIR}/src/Tools/Windows/I18N")
set (RESTEXT "${TOOLS_BIN_DIR}/ResText/Release/ResText.exe")
set (RESPWSL "${TOOLS_BIN_DIR}/ResPWSL/Release/ResPWSL.exe")

string (REGEX REPLACE "(..)" "pwsafe_\\1.po; " POS ${LANGS})

set (BASE_DLL "${CMAKE_BINARY_DIR}/src/ui/Windows/language/Release/pwsafe_base.dll")

add_custom_target("pot" DEPENDS "pwsafe.pot")
add_custom_command(OUTPUT "pwsafe.pot"
                   COMMAND ${RESTEXT}
                   ARGS "extract" "-noupdate" ${BASE_DLL} "pwsafe.pot"
                   DEPENDS ${BASE_DLL} ${RESTEXT}
                   COMMENT "Updating pwsafe.pot using ${RESTEXT}"
                   )
set_property( TARGET pot PROPERTY FOLDER I18N )
