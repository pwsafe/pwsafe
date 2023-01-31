set (LANG_RES ../PasswordSafe.rc ../res/PasswordSafe2.rc2 ../res/PasswordSafe3.rc2)

add_library(pwsafe_base MODULE)
target_sources(pwsafe_base PRIVATE ${LANG_RES})
target_link_options(pwsafe_base PRIVATE /NOENTRY)
target_link_libraries(pwsafe_base PRIVATE harden_interface pws_version)

set_target_properties(pwsafe_base PROPERTIES
  FOLDER I18N
  LINKER_LANGUAGE CXX
)

if (USE_INTERPROCEDURAL_OPTIMIZATION)
  # There is no code, so disable LTO/LTCG for this target.
  set_target_properties(pwsafe_base PROPERTIES INTERPROCEDURAL_OPTIMIZATION FALSE)
endif ()
