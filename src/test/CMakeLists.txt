set (TEST_SRCS
  AESTest.cpp FileV3Test.cpp ItemAttTest.cpp OSTest.cpp BlowFishTest.cpp
  FileV4Test.cpp ItemDataTest.cpp SHA256Test.cpp CommandsTest.cpp ItemFieldTest.cpp StringXTest.cpp
  coretest.cpp HMAC_SHA256Test.cpp KeyWrapTest.cpp TwoFishTest.cpp
  )

include_directories (${PROJECT_SOURCE_DIR}/src)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11 -DUSE_XML_LIBRARY=XERCES -DWCHAR_INCOMPATIBLE_XMLCH ${CMAKE_WXWINDOWS_CXX_FLAGS}")

add_executable(coretest ${TEST_SRCS})
target_link_libraries(coretest ${GTEST_LIBRARIES} core os uuid pthread)
add_test(Coretests coretest)
