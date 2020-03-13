set (LANGS "RU" "DE" "ES" "FR" "ZH" "PL")

if (WIN32)
  set (LANGS "DE" "FR" "RU" "ES")
  # pwsafePL -- nothing there :-(
  # pwsafeZH -- currently breaks my hhc :-(

  set (HHC "HTMLHelpWorkshopCompiler_undefined" CACHE FILEPATH
       "Microsoft's help compiler (hhc.exe)")

  string (REGEX REPLACE "(..)" "pwsafe\\1/pwsafe\\1.chm;" HELPFILES ${LANGS})
  add_custom_target("helpfiles" ALL
    DEPENDS "default/pwsafe.chm" ${HELPFILES}
    )

  # English help is a special case: default -> pwsafe.chm
  add_custom_command(OUTPUT "pwsafe.chm"
                     COMMAND ${HHC} "pwsafe.hhp"
                     WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/default"
                     )

  macro(make_help LANG)
    add_custom_command(OUTPUT "pwsafe${LANG}.chm"
                       COMMAND ${HHC} "pwsafe.hhp"
                       WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/pwsafe${LANG}"
                       )
  endmacro(make_help)

else (WIN32)

  string (REGEX REPLACE "(..)" "help\\1.zip;" HELPFILES ${LANGS})
  add_custom_target("helpfiles" ALL
    DEPENDS "helpEN.zip" ${HELPFILES}
    )
  # English help is a special case: default -> helpEN.zip
  add_custom_command(OUTPUT "helpEN.zip"
    COMMAND "sed" "-i" "pwsafe*.hh?" "-e" "'s&html\\\\&html/&g'"
    COMMAND "zip" "-Xqr" "${CMAKE_CURRENT_BINARY_DIR}/helpEN" "./*"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/default"
    )
  # Other languages are amenable to a macro:
  macro(make_help LANG)
    add_custom_command(OUTPUT "help${LANG}.zip"
      COMMAND "sed" "-i" "pwsafe*.hh?" "-e" "'s&html\\\\&html/&g'"
      COMMAND "zip" "-Xqr" "${CMAKE_CURRENT_BINARY_DIR}/help${LANG}" "./*"
      WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/pwsafe${LANG}"
      )
  endmacro(make_help)

endif (WIN32)

foreach (LANG ${LANGS})
  make_help(${LANG})
endforeach(LANG)
