set (LANGS "RU" "DE" "ES" "FR" "ZH" "PL")

if (WIN32)
  # TBD
else()
  string (REGEX REPLACE "(..)" "help\\1.zip;" HELPFILES ${LANGS})
  add_custom_target("helpfiles" ALL
    DEPENDS "helpEN.zip" ${HELPFILES}
    )
  # English help is a special case: default -> helpEN.zip
  add_custom_command(OUTPUT "helpEN.zip"
    COMMAND "zip" "-qr" "${CMAKE_CURRENT_BINARY_DIR}/helpEN" "./*"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/default"
    )
  # Other languages are amenable to a macro:
  macro(make_help LANG)
    add_custom_command(OUTPUT "help${LANG}.zip"
      COMMAND "zip" "-qr" "${CMAKE_CURRENT_BINARY_DIR}/help${LANG}" "./*"
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/pwsafe${LANG}"
      )
  endmacro(make_help)

  foreach (LANG ${LANGS})
    make_help(${LANG})
  endforeach(LANG)
endif()