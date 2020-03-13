# CMakeLists.txt for pwsafe I18N support:
#
# Supports making language-specific DLLs
# Supports updating language-specific .po files
# Supports creating pwsafe.pot file as the basis for a new translation
#

set (LANGS "cz" "de" "dk" "es" "fr" "hu" "it" "kr" "nl" "pl" "ru" "sv"
     "sl" "tr" "zh")

set (TOOLS_BIN_DIR "${CMAKE_BINARY_DIR}/src/Tools/Windows/I18N")
set (RESTEXT "${TOOLS_BIN_DIR}/ResText/Release/ResText.exe")
set (RESPWSL "${TOOLS_BIN_DIR}/ResPWSL/Release/ResPWSL.exe")

string (REGEX REPLACE "(..)" "pos/pwsafe_\\1.po; " POS ${LANGS})

set (BASE_DLL
     "${CMAKE_BINARY_DIR}/Release/pwsafe_base.dll")

set (DLL_DIR "${CMAKE_BINARY_DIR}/bin")

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
# Note that this updates the SOURCE TREE!!!
#
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
                   pos/pwsafe_sl.po
                   pos/pwsafe_tr.po
                   pos/pwsafe_zh.po
                   )

foreach (LANG ${LANGS})
 add_custom_command(OUTPUT "pos/pwsafe_${LANG}.po"
	  COMMAND ${RESTEXT} "extract" "${BASE_DLL}" "${CMAKE_CURRENT_SOURCE_DIR}/pos/pwsafe_${LANG}.po"
    DEPENDS "${BASE_DLL}"
    COMMENT "Updating pwsafe_${LANG}.po"
    )
endforeach(LANG)

set_property( TARGET update-pos PROPERTY FOLDER I18N )

add_custom_target("dlls"
                  DEPENDS
                   ${DLL_DIR}/pwsafeCZ.dll
                   ${DLL_DIR}/pwsafeDE.dll
                   ${DLL_DIR}/pwsafeDK.dll
                   ${DLL_DIR}/pwsafeES.dll
                   ${DLL_DIR}/pwsafeFR.dll
                   ${DLL_DIR}/pwsafeHU.dll
                   ${DLL_DIR}/pwsafeIT.dll
                   ${DLL_DIR}/pwsafeKR.dll
                   ${DLL_DIR}/pwsafeNL.dll
                   ${DLL_DIR}/pwsafePL.dll
                   ${DLL_DIR}/pwsafeRU.dll
                   ${DLL_DIR}/pwsafeSL.dll
                   ${DLL_DIR}/pwsafeSV.dll
                   ${DLL_DIR}/pwsafeTR.dll
                   ${DLL_DIR}/pwsafeZH.dll
                   )


macro( make_dll LANG CODE OUT)
  string (TOUPPER ${LANG} UCLANG)
  add_custom_command(OUTPUT "${DLL_DIR}/pwsafe${UCLANG}.dll"
                     COMMAND "${RESTEXT}" "apply" "${BASE_DLL}"
                             "foo.dll" "${CMAKE_CURRENT_SOURCE_DIR}/pos/pwsafe_${LANG}.po"
                     COMMAND "${RESPWSL}" "apply" "foo.dll" "${CODE}"
                     COMMAND "DEL" "foo.dll" "pwsafe${UCLANG}.dll"
                     COMMAND "REN" "pwsafe${OUT}.dll" "pwsafe${UCLANG}.dll"
                     WORKING_DIRECTORY ${DLL_DIR}
                     COMMENT "Making pwsafe${UCLANG}.dll"
                     )
endmacro( make_dll LANG CODE )

make_dll( "cz" "0x0405" "CS_CZ")
make_dll( "da" "0x0406" "DA_DK")
make_dll( "de" "0x0407" "DE_DE")
make_dll( "es" "0x0c0a" "ES_ES")
make_dll( "fr" "0x040c" "FR_FR")
make_dll( "hu" "0x040e" "HU_HU")
make_dll( "it" "0x0410" "IT_IT")
make_dll( "kr" "0x0412" "KO_KR")
make_dll( "nl" "0x0413" "NL_NL")
make_dll( "pl" "0x0415" "PL_PL")
make_dll( "ru" "0x0419" "RU_RU")
make_dll( "sv" "0x041d" "SV_SE")
make_dll( "sl" "0x0424" "SL_SI")
make_dll( "tr" "0x041f" "TR_TR")
make_dll( "zh" "0x0804" "ZH_CN")

set_property( TARGET dlls PROPERTY FOLDER I18N )
