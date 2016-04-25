set (RESPWSL_SRCS
    ResPWSL.cpp
    StdAfx.cpp
    StringFileInfo.cpp
    StringTable.cpp
    VersionInfo.cpp
    VersionInfoBuffer.cpp
    VersionInfoString.cpp
)

set (RESPWSL_RES ResPWSL.rc)

set( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /D _AFXDLL" )

add_executable( ResPWSL
                EXCLUDE_FROM_ALL
                ${RESPWSL_SRCS} ${RESPWSL_RES})

set_property( TARGET ResPWSL
              PROPERTY FOLDER Tools )
