# CMakeLists.txt for pwsafe I18N support:
#
# For a "regular" build, we assume the .po files are up-to-date,
# and just create the mo files from them.
#
# If we want to update the po files to send to translators,
# invoke with "pos" target (e.g., "make pos"). Keep in mind
# that this updates the build tree.
#
# Parent CMakeLists.txt already called find_package(Gettext)

set (LANGS "da" "de" "es" "fr" "hu" "it" "ko" "nl" "pl" "ru" "sv" "zh")


# In general, globbing the source file's not recommended for cmake,
# as it breaks the regeneration functionality. This is moot for the
# case of po-related stuff.

set (SRC_BASE_DIR "${PROJECT_SOURCE_DIR}/src")
set (SRC_I18N_DIR "${SRC_BASE_DIR}/ui/wxWidgets/I18N")
  
string (REGEX REPLACE "(..)" "pwsafe_\\1.po;" POS ${LANGS})
string (REGEX REPLACE "(..)" "mos/\\1/LC_MESSAGES/pwsafe.mo;" MOS ${LANGS})


file (GLOB I18N_SRCS
  "${SRC_BASE_DIR}/ui/wxWidgets/*.cpp"
  "${SRC_BASE_DIR}/ui/wxWidgets/*.h"
  "${SRC_BASE_DIR}/core/*.cpp"
  "${SRC_BASE_DIR}/core/*.h"
  "${CMAKE_BINARY_DIR}/src/core/core_st.*"
  "${SRC_BASE_DIR}/os/unix/*.cpp"
  "${SRC_BASE_DIR}/os/unix/*.h"
  )

# pot is updated from source code
add_custom_target("pot"
  DEPENDS "pwsafe.pot")
add_custom_command(OUTPUT "pwsafe.pot"
  COMMAND "xgettext"
  ARGS "--default-domain=pwsafe" "--from-code=UTF-8"
  "--language=C++" "--keyword=_" "--output=pwsafe.pot" ${I18N_SRCS}
  DEPENDS ${I18N_SRCS}
  COMMENT "Updating pwsafe.pot"
  )

add_custom_target("pos"
  DEPENDS pot ${POS})
macro(update_po LANG)
  file (COPY "pos/pwsafe_${LANG}.po" DESTINATION "pos")
  add_custom_command(OUTPUT "pwsafe_${LANG}.po"
	  COMMAND ${GETTEXT_MSGMERGE_EXECUTABLE} "--update" "--backup=off"
    "pos/pwsafe_${LANG}.po" "${CMAKE_CURRENT_BINARY_DIR}/pwsafe.pot"
    DEPENDS "pwsafe.pot"
    COMMENT "Updating pwsafe_${LANG}.po"
    )
endmacro(update_po)
foreach (LANG ${LANGS})
  update_po(${LANG})
endforeach(LANG)

add_custom_target("mos" ALL
  DEPENDS ${MOS})
macro(make_mo LANG)
  file (MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/mos/${LANG}/LC_MESSAGES")
  add_custom_command(OUTPUT "mos/${LANG}/LC_MESSAGES/pwsafe.mo"
    COMMAND ${GETTEXT_MSGFMT_EXECUTABLE} "--check"
    "--output-file=mos/${LANG}/LC_MESSAGES/pwsafe.mo"
    "${SRC_I18N_DIR}/pos/pwsafe_${LANG}.po"
    DEPENDS "${SRC_I18N_DIR}/pos/pwsafe_${LANG}.po"
    )
  endmacro(make_mo)
foreach (LANG ${LANGS})
  make_mo(${LANG})
endforeach(LANG)
