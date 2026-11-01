/*
* Copyright (c) 2013-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
* Contributed by Ashley R. Thomas, 05-Oct-2023
*/
// TestCommon.h
//-----------------------------------------------------------------------------

#ifndef _TESTCOMMON_H
#define _TESTCOMMON_H

#include <cassert>
#include <cstring>
#include <cstdint>
#include <vector>

using byte_vector = std::vector<uint8_t>;

inline byte_vector byte_vector_from_string(const char* s, size_t len) {
  return byte_vector(s, s + len);
}

// Useful for creating byte_vector instances from const char arrays
// containing embedded '\0' null values which are part of valid data,
// and do not indicate an end of C-string.
template<std::size_t N>
byte_vector byte_vector_from_const_char_array(const char(&s)[N], bool exclude_term_null = true)
{
  assert(!exclude_term_null || N > 0);
  size_t len = exclude_term_null ? N - 1 : N;
  return byte_vector_from_string(s, len);
}

inline byte_vector byte_vector_from_string(const char* s) {
  return byte_vector_from_string(s, strlen(s));
}

inline byte_vector byte_vector_from_string(const std::string& str) {
  return byte_vector(str.begin(), str.end());
}

// In Windows debug builds, MFC's ASSERT reports via _CrtDbgReport(), which
// gtest redirects to stderr, so the assertion doesn't terminate the process.
// Use DEATH_GUARD inside EXPECT_DEATH statements to make it abort, e.g.:
//   EXPECT_DEATH({ DEATH_GUARD; foo(); }, "Assertion");
// Note that MFC's message doesn't include the asserted expression, so match
// "Assertion", which is common to MSVC, glibc and macOS messages.
#if defined(_WIN32) && defined(_DEBUG)
class AbortOnCrtAssert { // implemented in coretest.cpp
public:
  AbortOnCrtAssert();
  ~AbortOnCrtAssert();
};
#define DEATH_GUARD AbortOnCrtAssert death_guard
#else
#define DEATH_GUARD
#endif

#endif
