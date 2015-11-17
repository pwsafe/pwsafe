set (TEST_SRCS
  AESTest.cpp FileV3Test.cpp ItemAttTest.cpp OSTest.cpp BlowFishTest.cpp
  FileV4Test.cpp ItemDataTest.cpp SHA256Test.cpp CommandsTest.cpp ItemFieldTest.cpp StringXTest.cpp
  coretest.cpp HMAC_SHA256Test.cpp KeyWrapTest.cpp TwoFishTest.cpp
  )

# Setup test data
file (MAKE_DIRECTORY "data")
file (COPY "data/image1.jpg" DESTINATION "data")
file (COPY "data/text1.txt" DESTINATION "data")

add_executable(coretest ${TEST_SRCS})
target_link_libraries(coretest ${GTEST_LIBRARIES} core os uuid ${XercesC_LIBRARY} pthread)
add_test(NAME Coretests
  COMMAND coretest
  )
