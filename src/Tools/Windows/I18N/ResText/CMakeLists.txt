set (RESTEXT_SRCS
    codecvt.cpp
    POFile.cpp
    ResModule.cpp
    ResText.cpp
    stdafx.cpp
    UnicodeUtils.cpp
    Utils.cpp
)

set (RESTEXT_RES ResText.rc)

add_executable( ResText
                EXCLUDE_FROM_ALL
                ${RESTEXT_SRCS} ${RESTEXT_RES})

set_property( TARGET ResText
              PROPERTY FOLDER Tools )
              
target_compile_definitions(ResText PRIVATE 
              _CRT_SECURE_NO_WARNINGS 
              _CRT_NON_CONFORMING_SWPRINTFS)