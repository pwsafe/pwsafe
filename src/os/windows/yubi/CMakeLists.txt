set (YUBI_SRCS
    YkLib.cpp
    stdafx.cpp
)

add_library(YkLib22 STATIC ${YUBI_SRCS})
target_link_libraries(YkLib22 PRIVATE harden_interface)
