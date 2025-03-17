set (CLI_SRCS
  main.cpp
  search.cpp
  argutils.cpp
  searchaction.cpp
  strutils.cpp
  safeutils.cpp
  diff.cpp
  impexp.cpp)

set (CLI_TEST_SRCS
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
  list (APPEND CLI_TEST_SRCS cli.rc)
  set_source_files_properties(cli.rc PROPERTIES LANGUAGE RC)
endif(WIN32)


add_executable(pwsafe-cli ${CLI_SRCS})
target_link_libraries(pwsafe-cli harden_interface)

install (TARGETS pwsafe-cli RUNTIME DESTINATION "bin")

if (IPO_SUPPORTED)
  set_property(TARGET pwsafe-cli PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE True)
endif (IPO_SUPPORTED)

target_link_libraries(pwsafe-cli core os core ${wxWidgets_LIBRARIES})
if (XercesC_LIBRARY)
  target_link_libraries(pwsafe-cli ${XercesC_LIBRARY})
endif (XercesC_LIBRARY)

if (NOT APPLE)
  target_link_libraries(pwsafe-cli uuid)
endif (NOT APPLE)

if (NOT APPLE AND NOT WIN32)
  target_link_libraries(pwsafe-cli magic)
endif (NOT APPLE AND NOT WIN32)


if (NOT WIN32)
	target_link_libraries(pwsafe-cli pthread)
else ()
	target_link_libraries(pwsafe-cli Rpcrt4)
	set_target_properties(pwsafe-cli PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
endif (NOT WIN32)

# Add test executable
add_executable(pwsafe-cli-test ${CLI_TEST_SRCS})
set_target_properties(pwsafe-cli-test PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD_RELEASE 1)
target_link_libraries(pwsafe-cli-test harden_interface gtest gtest_main core os core ${wxWidgets_LIBRARIES})

if (XercesC_LIBRARY)
  target_link_libraries(pwsafe-cli-test ${XercesC_LIBRARY})
endif (XercesC_LIBRARY)

if (NOT APPLE)
  target_link_libraries(pwsafe-cli-test uuid)
endif (NOT APPLE)

if (NOT APPLE AND NOT WIN32)
  target_link_libraries(pwsafe-cli-test magic)
endif (NOT APPLE AND NOT WIN32)

if (NOT WIN32)
  target_link_libraries(pwsafe-cli-test pthread)
else ()
  target_link_libraries(pwsafe-cli-test Rpcrt4)
  set_target_properties(pwsafe-cli-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}")
endif (NOT WIN32)

# Add test target
if (NOT WIN32) # Windows cli-test broken, doesn't recognize rc resources.
add_test(NAME cli-test
  COMMAND pwsafe-cli-test
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)
endif (NOT WIN32)
