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
install (TARGETS pwsafe-cli RUNTIME DESTINATION "bin")

if (IPO_SUPPORTED)
  set_property(TARGET pwsafe-cli PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE True)
endif (IPO_SUPPORTED)
if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  # squash lintian hardening-no-relro warning, probably good for all Linuxen
  set_target_properties(pwsafe-cli PROPERTIES LINK_FLAGS "-z relro")
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

target_link_libraries(pwsafe-cli core os core uuid pthread ${wxWidgets_LIBRARIES})
if (XercesC_LIBRARY)
  target_link_libraries(pwsafe-cli ${XercesC_LIBRARY})
endif (XercesC_LIBRARY)
