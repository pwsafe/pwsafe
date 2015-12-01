set (CLI_SRCS main.cpp)

add_executable(pwsafe-cli ${CLI_SRCS})
target_link_libraries(pwsafe-cli core os uuid pthread)
if (XercesC_LIBRARY)
  target_link_libraries(pwsafe-cli ${XercesC_LIBRARY})
endif (XercesC_LIBRARY)
