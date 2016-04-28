set (RESTEXT_SRCS
    codecvt.cpp
    POFile.cpp
    ResModule.cpp
    ResText.cpp
    stdafx.cpp
    Utils.cpp
)

set (RESTEXT_RES ResText.rc)

add_executable( ResText
                EXCLUDE_FROM_ALL
                ${RESTEXT_SRCS} ${RESTEXT_RES})

set_property( TARGET ResText
              PROPERTY FOLDER Tools )
