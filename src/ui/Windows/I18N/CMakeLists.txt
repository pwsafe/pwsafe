# CMakeLists.txt for pwsafe I18N support:
#
# Supports making language-specific DLLs
# Supports updating language-specific .po files
# Supports creating pwsafe.pot file as the basis for a new translation
#

set (LANGS "ar" "cz" "de" "dk" "es" "fr" "hu" "it" "kr" "nl" "pl" "ru" "sv"
     "sl" "tr" "zh")

string (REGEX REPLACE "(..)" "pos/pwsafe_\\1.po; " POS ${LANGS})

set(BASE_DLL "$<TARGET_FILE:pwsafe_base>")

set (DLL_DIR "${CMAKE_BINARY_DIR}/I18N")

add_custom_target("pot" DEPENDS "pwsafe.pot")
add_custom_command(OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/pwsafe.pot"
                   DEPENDS pwsafe_base
                   COMMAND ResText extract -noupdate "${BASE_DLL}" pwsafe.pot
                   WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
                   COMMENT "Updating pwsafe.pot from ${BASE_DLL} using ${RESTEXT}"
                   VERBATIM
                   )
set_property( TARGET pot PROPERTY FOLDER I18N )

# No idea why $POS doesn't work here, but it doesn't...
# Note that this updates the SOURCE TREE!!!
#
add_custom_target("update-pos"
                  DEPENDS
                   pos_pwsafe_ar.po
                   pos/pwsafe_pt_br.po
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
  add_custom_command(OUTPUT "${CMAKE_CURRENT_SOURCE_DIR}/pos/pwsafe_${LANG}.po"
    DEPENDS pwsafe_base pwsafe.pot
    COMMAND ResText "extract" "${BASE_DLL}" "pos/pwsafe_${LANG}.po"
    COMMENT "Updating pwsafe_${LANG}.po using ${RESTEXT}"
    WORKING_DIRECTORY  "${CMAKE_CURRENT_SOURCE_DIR}"
    VERBATIM
  )
endforeach(LANG)

set_property( TARGET update-pos PROPERTY FOLDER I18N )


set(TEMP_DIR "${CMAKE_CURRENT_BINARY_DIR}/tmp")

unset(LANG_DLLS)

macro( make_dll LANG CODE OUT)
  string (TOUPPER ${LANG} UCLANG)
  set(TEMP_DLL "${TEMP_DIR}/pwsafe_${LANG}-tmp.dll")
  set(CODE_DLL "${TEMP_DIR}/pwsafe${OUT}.dll")
  set(OUT_DLL "${DLL_DIR}/pwsafe${UCLANG}.dll")
  set(PO_FILE "${CMAKE_CURRENT_SOURCE_DIR}/pos/pwsafe_${LANG}.po")
  add_custom_command(OUTPUT "${TEMP_DLL}"
                     DEPENDS pwsafe_base "${PO_FILE}"
                     COMMAND "${CMAKE_COMMAND}" -E make_directory "${CMAKE_CURRENT_BINARY_DIR}/tmp"
                     COMMAND ResText apply "${BASE_DLL}" "${TEMP_DLL}" "${PO_FILE}"
                     WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
                     COMMENT "Making ${TEMP_DLL} from ${BASE_DLL} and pos/pwsafe_${LANG}.po"
                     )

  add_custom_command(OUTPUT "${CODE_DLL}"
                     DEPENDS "${TEMP_DLL}"
                     COMMAND ResPWSL apply "${TEMP_DLL}" "${CODE}"
                     WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
                     COMMENT "Making ${CODE_DLL} from ${TEMP_DLL} for ${CODE}"
                     )

  add_custom_command(OUTPUT "${OUT_DLL}"
                     DEPENDS "${CODE_DLL}"
                     COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CODE_DLL}" "${OUT_DLL}"
                     WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
                     COMMENT "Copying ${CODE_DLL} to ${OUT_DLL}"
                     )

  list(APPEND LANG_DLLS "${OUT_DLL}")
endmacro( make_dll LANG CODE )

make_dll( "ar" "0x2c01" "AR_JO")
make_dll( "cz" "0x0405" "CS_CZ")
make_dll( "de" "0x0407" "DE_DE")
make_dll( "dk" "0x0406" "DA_DK")
make_dll( "es" "0x0c0a" "ES_ES")
make_dll( "fr" "0x040c" "FR_FR")
make_dll( "hu" "0x040e" "HU_HU")
make_dll( "it" "0x0410" "IT_IT")
make_dll( "kr" "0x0412" "KO_KR")
make_dll( "nl" "0x0413" "NL_NL")
make_dll( "pl" "0x0415" "PL_PL")
make_dll( "pt_br" "0x0416" "PT_BR")
make_dll( "ru" "0x0419" "RU_RU")
make_dll( "sv" "0x041d" "SV_SE")
make_dll( "sl" "0x0424" "SL_SI")
make_dll( "tr" "0x041f" "TR_TR")
make_dll( "zh" "0x0804" "ZH_CN")

add_custom_target(dlls DEPENDS ${LANG_DLLS})

set_property( TARGET dlls PROPERTY FOLDER I18N )
