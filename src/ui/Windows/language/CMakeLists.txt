set (LANG_SRCS
    languageDLL.cpp
    stdafx.cpp
)

set (LANG_RES ../PasswordSafe.rc ../res/PasswordSafe2.rc2 ../res/PasswordSafe3.rc2)

set(CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} /D languageDLL_EXPORTS")

add_library(pwsafe_base SHARED ${LANG_SRCS} ${LANG_RES})
set_property( TARGET pwsafe_base PROPERTY FOLDER I18N )
set_target_properties(pwsafe_base PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CACHEFILE_DIR})
