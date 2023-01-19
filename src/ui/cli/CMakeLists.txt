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

if (WIN32)
  list (APPEND CLI_SRCS cli.rc)
endif(WIN32)


add_executable(pwsafe-cli ${CLI_SRCS})
target_link_libraries(pwsafe-cli harden_interface)

install (TARGETS pwsafe-cli RUNTIME DESTINATION "bin")

if (IPO_SUPPORTED)
  set_property(TARGET pwsafe-cli PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE True)
endif (IPO_SUPPORTED)

target_link_libraries(pwsafe-cli core os core uuid ${wxWidgets_LIBRARIES})
if (XercesC_LIBRARY)
  target_link_libraries(pwsafe-cli ${XercesC_LIBRARY})
endif (XercesC_LIBRARY)

if (NOT WIN32)
	target_link_libraries(pwsafe-cli pthread magic)
else ()
	target_link_libraries(pwsafe-cli Rpcrt4)
	set_target_properties(pwsafe-cli PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
endif (NOT WIN32)
