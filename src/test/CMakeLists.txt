set (TEST_SRCS
  AESTest.cpp AliasShortcutTest.cpp FileV3Test.cpp ItemAttTest.cpp OSTest.cpp BlowFishTest.cpp
  FileV4Test.cpp ItemDataTest.cpp SHA256Test.cpp SHA1Test.cpp CommandsTest.cpp ItemFieldTest.cpp
  StringXTest.cpp coretest.cpp HMAC_SHA256Test.cpp HMAC_SHA1Test.cpp KeyWrapTest.cpp TwoFishTest.cpp
  AuxParseTest.cpp UtilTest.cpp FileEncDecTest.cpp ImportTextTest.cpp TOTPTest.cpp Base32Test.cpp)

if (WIN32)
  list (APPEND TEST_SRCS ../core/core.rc2)
  set_source_files_properties(../core/core.rc2 PROPERTIES LANGUAGE RC)
endif(WIN32)


# Setup test data
file (MAKE_DIRECTORY "data")
file (COPY "data/image1.jpg" DESTINATION "data")
file (COPY "data/text1.txt" DESTINATION "data")
file (COPY "data/import-text-unit-test1.txt" DESTINATION "data")
file (COPY "data/import-text-unit-test2.csv" DESTINATION "data")
file (COPY "data/import-text-unit-test3.csv" DESTINATION "data")
file (COPY "data/import-text-unit-test4.csv" DESTINATION "data")
file (COPY "data/import-text-unit-test5.csv" DESTINATION "data")

add_executable(coretest ${TEST_SRCS})
if (MSVC)
set_target_properties(coretest PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD_RELEASE 1)
target_link_libraries(coretest gtest core os Rpcrt4 harden_interface)
elseif (APPLE)
target_link_libraries(coretest gtest core os pthread "-framework CoreFoundation")
else ()
target_link_libraries(coretest gtest core os uuid pthread magic ${wxWidgets_LIBRARIES} Xtst X11)
endif()
if (XercesC_LIBRARY)
  target_link_libraries(coretest ${XercesC_LIBRARY})
endif (XercesC_LIBRARY)

target_link_libraries(coretest harden_interface)

add_test(NAME Coretests
  COMMAND coretest
  )
