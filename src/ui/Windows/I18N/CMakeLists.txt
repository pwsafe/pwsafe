# CMakeLists.txt for pwsafe I18N support:
#
# Supports making language-specific DLLs
# Supports updating language-specific .po files
# Supports creating pwsafe.pot file as the basis for a new translation
#

set(BASE_DLL "$<TARGET_FILE:pwsafe_base>")

set (DLL_DIR "${CMAKE_BINARY_DIR}/I18N")

set(POT_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/pwsafe.pot")
set(POT_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/pwsafe.pot")

add_custom_command(
  OUTPUT "${POT_OUTPUT}"
  DEPENDS pwsafe_base "${POT_INPUT}"
  COMMAND "${CMAKE_COMMAND}" -E copy "${POT_INPUT}" "${POT_OUTPUT}"
  COMMAND ResText extract -noupdate "${BASE_DLL}" "${POT_OUTPUT}"
  COMMENT "Updating pwsafe.pot from ${BASE_DLL} using ${RESTEXT}"
  WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
  VERBATIM
)

option(USE_EXISTING_TRANSLATIONS "Use the .po files from the source tree when building the translation DLLs.  When disabled, the DLLs will be built with updated .po files." ON)

file(MAKE_DIRECTORY pos)
file(MAKE_DIRECTORY tmp)

unset(LANG_POS)
unset(LANG_DLLS)

macro( make_dll LANG CODE OUT)
  set(PO_INPUT "${CMAKE_CURRENT_SOURCE_DIR}/pos/pwsafe_${LANG}.po")
  set(PO_OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/pos/pwsafe_${LANG}.po")

  add_custom_command(
    OUTPUT "${PO_OUTPUT}"
    DEPENDS pwsafe_base "${PO_INPUT}"
    COMMAND "${CMAKE_COMMAND}" -E copy "${PO_INPUT}" "${PO_OUTPUT}"
    COMMAND ResText extract "${BASE_DLL}" "${PO_OUTPUT}"
    COMMENT "Updating pwsafe_${LANG}.po using ${RESTEXT}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    VERBATIM
  )

  list(APPEND LANG_POS "${PO_OUTPUT}")

  string (TOUPPER ${LANG} UCLANG)
  set(TEMP_DLL "tmp/pwsafe_${LANG}-tmp.dll")
  set(CODE_DLL "tmp/pwsafe${OUT}.dll")
  set(OUT_DLL "${DLL_DIR}/pwsafe${UCLANG}.dll")

  if (USE_EXISTING_TRANSLATIONS)
    set(PO_FILE "${PO_INPUT}")
  else ()
    set(PO_FILE "${PO_OUTPUT}")
  endif ()

  add_custom_command(
    OUTPUT "${TEMP_DLL}"
    DEPENDS pwsafe_base "${PO_FILE}"
    COMMAND ResText apply "${BASE_DLL}" "${TEMP_DLL}" "${PO_FILE}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Making ${TEMP_DLL} from ${BASE_DLL} and pos/pwsafe_${LANG}.po"
    VERBATIM
  )

  add_custom_command(
    OUTPUT "${CODE_DLL}"
    DEPENDS "${TEMP_DLL}"
    COMMAND ResPWSL apply "${TEMP_DLL}" "${CODE}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Making ${CODE_DLL} from ${TEMP_DLL} for ${CODE}"
    VERBATIM
  )

  add_custom_command(
    OUTPUT "${OUT_DLL}"
    DEPENDS "${CODE_DLL}"
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${CODE_DLL}" "${OUT_DLL}"
    WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
    COMMENT "Copying ${CODE_DLL} to ${OUT_DLL}"
    VERBATIM
  )

  list(APPEND LANG_DLLS "${OUT_DLL}")
endmacro( make_dll LANG CODE )

make_dll(ar    0x2c01 AR_JO)
make_dll(cz    0x0405 CS_CZ)
make_dll(de    0x0407 DE_DE)
make_dll(dk    0x0406 DA_DK)
make_dll(es    0x0c0a ES_ES)
make_dll(fr    0x040c FR_FR)
make_dll(hu    0x040e HU_HU)
make_dll(it    0x0410 IT_IT)
make_dll(kr    0x0412 KO_KR)
make_dll(lv    0x0426 LV_LV)
make_dll(nl    0x0413 NL_NL)
make_dll(pl    0x0415 PL_PL)
make_dll(pt_br 0x0416 PT_BR)
make_dll(ru    0x0419 RU_RU)
make_dll(sv    0x041d SV_SE)
make_dll(sl    0x0424 SL_SI)
make_dll(tr    0x041f TR_TR)
make_dll(zh    0x0804 ZH_CN)

add_custom_target(language-dlls DEPENDS ${LANG_DLLS})

set_property(TARGET language-dlls PROPERTY FOLDER I18N)

# Don't update the in-tree pwsafe.pot unless the user builds this target.
add_custom_target(update-pot
  DEPENDS "${POT_OUTPUT}"
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${POT_OUTPUT}" "${POT_INPUT}"
  VERBATIM
)
set_property(TARGET update-pot PROPERTY FOLDER I18N)

# Don't update the in-tree .po files unless the user builds this target.
add_custom_target(update-pos
  DEPENDS ${LANG_POS}
  COMMAND "${CMAKE_COMMAND}" -E copy_if_different ${LANG_POS} "${CMAKE_CURRENT_SOURCE_DIR}/pos"
  VERBATIM
)
set_property(TARGET update-pos PROPERTY FOLDER I18N)

add_custom_target(update-translations DEPENDS update-pot update-pos)
set_property(TARGET update-translations PROPERTY FOLDER I18N)
