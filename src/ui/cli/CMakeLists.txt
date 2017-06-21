set (CLI_SRCS
  main.cpp
  search.cpp
  argutils.cpp
  searchaction.cpp
  strutils.cpp
	safeutils.cpp
  diff.cpp
  impexp.cpp)

set (CLI_TEST_SRC
  add-entry-test.cpp
  arg-fields-test.cpp
  split-test.cpp
	safeutils.cpp
  argutils.cpp
  searchaction.cpp
  strutils.cpp
  search-test.cpp
  search.cpp)


add_executable(pwsafe-cli ${CLI_SRCS})
target_link_libraries(pwsafe-cli core os core uuid pthread)
if (XercesC_LIBRARY)
  target_link_libraries(pwsafe-cli ${XercesC_LIBRARY})
endif (XercesC_LIBRARY)
