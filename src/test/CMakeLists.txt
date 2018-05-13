set (TEST_SRCS
  AESTest.cpp AliasShortcutTest.cpp FileV3Test.cpp ItemAttTest.cpp OSTest.cpp BlowFishTest.cpp
  FileV4Test.cpp ItemDataTest.cpp SHA256Test.cpp CommandsTest.cpp ItemFieldTest.cpp StringXTest.cpp
  coretest.cpp HMAC_SHA256Test.cpp KeyWrapTest.cpp TwoFishTest.cpp AuxParseTest.cpp UtilTest.cpp
  )

# Setup test data
file (MAKE_DIRECTORY "data")
file (COPY "data/image1.jpg" DESTINATION "data")
file (COPY "data/text1.txt" DESTINATION "data")

add_executable(coretest ${TEST_SRCS})
if (MSVC)
set_target_properties(coretest PROPERTIES EXCLUDE_FROM_DEFAULT_BUILD_RELEASE 1)
target_link_libraries(coretest gtest core os Rpcrt4)
elseif (APPLE)
target_link_libraries(coretest gtest core os pthread "-framework CoreFoundation")
else ()
target_link_libraries(coretest gtest core os uuid pthread ${wxWidgets_LIBRARIES} Xtst X11)
endif()
if (XercesC_LIBRARY)
  target_link_libraries(coretest ${XercesC_LIBRARY})
endif (XercesC_LIBRARY)

add_test(NAME Coretests
  COMMAND coretest
  )
