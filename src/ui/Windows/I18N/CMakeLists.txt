# CMakeLists.txt for pwsafe I18N support:
#
# Supports making language-specific DLLs
# Supports updating language-specific .po files
# Supports creating pwsafe.pot file as the basis for a new translation
#

set (LANGS "cz" "de" "dk" "es" "fr" "hu" "it" "kr" "nl" "pl" "ru" "sv"
     "tr" "zh")

set (TOOLS_BIN_DIR "${CMAKE_BINARY_DIR}/src/Tools/Windows/I18N")
set (RESTEXT "${TOOLS_BIN_DIR}/ResText/Release/ResText.exe")
set (RESPWSL "${TOOLS_BIN_DIR}/ResPWSL/Release/ResPWSL.exe")

string (REGEX REPLACE "(..)" "pos/pwsafe_\\1.po; " POS ${LANGS})

set (BASE_DLL "${CMAKE_BINARY_DIR}/src/ui/Windows/language/Release/pwsafe_base.dll")

add_custom_target("pot" DEPENDS "pwsafe.pot")
add_custom_command(OUTPUT "pwsafe.pot"
                   COMMAND ${RESTEXT}
                   ARGS "extract" "-noupdate" ${BASE_DLL} "pwsafe.pot"
                   DEPENDS ${BASE_DLL} ${RESTEXT}
                   WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/pos"
                   COMMENT "Updating pwsafe.pot using ${RESTEXT}"
                   )
set_property( TARGET pot PROPERTY FOLDER I18N )

# No idea why $POS doesn't work here, but it doesn't...
add_custom_target("update-pos"
                  DEPENDS
                   pos/pwsafe_cz.po
                   pos/pwsafe_de.po
                   pos/pwsafe_dk.po
                   pos/pwsafe_es.po
                   pos/pwsafe_fr.po
                   pos/pwsafe_hu.po
                   pos/pwsafe_it.po
                   pos/pwsafe_kr.po
                   pos/pwsafe_nl.po
                   pos/pwsafe_pl.po
                   pos/pwsafe_ru.po
                   pos/pwsafe_sv.po
                   pos/pwsafe_tr.po
                   pos/pwsafe_zh.po
                   )

foreach (LANG ${LANGS})
 add_custom_command(OUTPUT "pos/pwsafe_${LANG}.po"
    message("COMMAND COPY ${CMAKE_SOURCE_TREE}/pos/pwsafe_${LANG}.po ${CMAKE_CURRENT_BINARY_DIR}/pos")
	  COMMAND ${RESTEXT} "extract" "${BASE_DLL}" "${CMAKE_CURRENT_BINARY_DIR}/pos/pwsafe_${LANG}.po"
    DEPENDS "${BASE_DLL}"
    COMMENT "Updating pwsafe_${LANG}.po"
    )
endforeach(LANG)

set_property( TARGET update-pos PROPERTY FOLDER I18N )
