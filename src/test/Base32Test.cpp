/*
* Copyright (c) 2003-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
* Contributed by Ashley R. Thomas, 01-Oct-2023
*/
// TOTPTest.cpp: Unit test for TOTP.

#ifdef WIN32
#include "../ui/Windows/stdafx.h"
#endif

#include <algorithm>
#include <vector>
#include <string>

#include "core/crypto/external/Chromium/base32.h"
#include "core/crypto/totp.h"
#include "gtest/gtest.h"

#include "TestCommon.h"

using namespace std;

// The odd string splits below are an effort to mitigate
// CI/CD false positives around secrets detection.

vector<
  tuple<
    byte_vector,  // Original unencoded data.
    string   // Base32 encoding of original data.
  >
> Base32_TestVectors = {
  // RFC4648 Base32 Test Vectors:
  {
    byte_vector_from_string(""),
    string("")
  },
  {
    byte_vector_from_string("f"),
    string("MY======")
  },
  {
    byte_vector_from_string("fo"),
    string("MZXQ====")
  },
  {
    byte_vector_from_string("foo"),
    string("MZXW6===")
  },
  {
    byte_vector_from_string("foob"),
    string("MZXW6YQ=")
  },
  {
    byte_vector_from_string("fooba"),
    string("MZXW6YTB")
  },
  {
    byte_vector_from_string("foobar"),
    string("MZXW6YTBOI======")
  },

  // Password Safe Base32 Test Vectors:
  // (Arbitrary aside from purposefully using the alphabet,
  //  all digits, and some embedded non-printable characters.)
  {
    byte_vector_from_string("T"),
    string("KQ======")
  },
  {
    byte_vector_from_string("Th"),
    string("KRUA====")
  },
  {
    byte_vector_from_string("The"),
    string("KRUGK===")
  },
  {
    byte_vector_from_string("The "),
    string("KRUGKIA=")
  },
  {
    byte_vector_from_string("The q"),
    string("KRUGKIDR")
  },
  {
    byte_vector_from_string("The qu"),
    string("KRUGKIDROU=====" "=")
  },
  {
    byte_vector_from_string("The qui"),
    string("KRUGKIDROVUQ===" "=")
  },
  {
    byte_vector_from_string("The quic"),
    string("KRUGKIDROVUWG==" "=")
  },
  {
    byte_vector_from_string("The quick"),
    string("KRUGKIDROVUWG2Y" "=")
  },
  {
    byte_vector_from_string("The quick "),
    string("KRUGKIDROVUWG2Z" "A")
  },
  {
    byte_vector_from_string("The quick b"),
    string("KRUGKIDROVUWG2Z" "AMI======")
  },
  {
    byte_vector_from_string("The quick br"),
    string("KRUGKIDROVUWG2Z" "AMJZA====")
  },
  {
    byte_vector_from_string("The quick bro"),
    string("KRUGKIDROVUWG2Z" "AMJZG6===")
  },
  {
    byte_vector_from_string("The quick brow"),
    string("KRUGKIDROVUWG2Z" "AMJZG65Y=")
  },
  {
    byte_vector_from_string("The quick brown"),
    string("KRUGKIDROVUWG2Z" "AMJZG653O")
  },
  {
    byte_vector_from_string("The quick brown "),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEA====" "==")
  },
  {
    byte_vector_from_string("The quick brown f"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTA==" "==")
  },
  {
    byte_vector_from_string("The quick brown fo"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG6=" "==")
  },
  {
    byte_vector_from_string("The quick brown fox"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "A=")
  },
  {
    byte_vector_from_string("The quick brown fox "),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BA")
  },
  {
    byte_vector_from_string("The quick brown fox j"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANI======")
  },
  {
    byte_vector_from_string("The quick brown fox ju"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2Q====")
  },
  {
    byte_vector_from_string("The quick brown fox jum"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W2===")
  },
  {
    byte_vector_from_string("The quick brown fox jump"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24A=")
  },
  {
    byte_vector_from_string("The quick brown fox jumps"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DT")
  },
  {
    byte_vector_from_string("The quick brown fox jumps "),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEA===" "===")
  },
  {
    byte_vector_from_string("The quick brown fox jumps o"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXQ=" "===")
  },
  {
    byte_vector_from_string("The quick brown fox jumps ov"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "===")
  },
  {
    byte_vector_from_string("The quick brown fox jumps ove"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZI=")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLS")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over "),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEA======")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over t"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2A====")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over th"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQ===")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZI=")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the "),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJA")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the l"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANQ==" "====")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the la"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQQ" "====")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the laz"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U===")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6I=")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy "),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JA")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy d"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMQ======")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy do"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXQ====")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy dog"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWO===")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy dog "),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIA=")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy dog 1"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBR")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy dog 12"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGI=" "=====")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy dog 123"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "Q====")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy dog 1234"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TI===")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy dog 12345"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINI=")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy dog 123456"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJW")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy dog 1234567"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG4======")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy dog 12345679"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44Q====")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy dog 123456790"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TA===")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy dog 123456790 "),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIA=")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy dog 123456790 t"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDU")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy dog 123456790 ti"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNE" "======")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy dog 123456790 tim"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WQ====")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy dog 123456790 time"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK===")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy dog 123456790 times"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4Y=")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy dog 123456790 times."),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZO")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy dog 123456790 times. "),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEA======")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy dog 123456790 times. \x01"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEAAQ====")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy dog 123456790 times. \x01\x02"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEAAQE===")
  },
  {
    byte_vector_from_string("The quick brown fox jumps over the lazy dog 123456790 times. \x01\x02\x07"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEAAQEBY=")
  },
  {
    byte_vector_from_const_char_array("The quick brown fox jumps over the lazy dog 123456790 times. \x01\x02\x07\x00"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEAAQEBYA")
  },
  {
    byte_vector_from_const_char_array("The quick brown fox jumps over the lazy dog 123456790 times. \x01\x02\x07\x00 "),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEAAQEBYAE" "A======")
  },
  {
    byte_vector_from_const_char_array("The quick brown fox jumps over the lazy dog 123456790 times. \x01\x02\x07\x00 H"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEAAQEBYAE" "BEA====")
  },
  {
    byte_vector_from_const_char_array("The quick brown fox jumps over the lazy dog 123456790 times. \x01\x02\x07\x00 He"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEAAQEBYAE" "BEGK===")
  },
  {
    byte_vector_from_const_char_array("The quick brown fox jumps over the lazy dog 123456790 times. \x01\x02\x07\x00 Hel"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEAAQEBYAE" "BEGK3A=")
  },
  {
    byte_vector_from_const_char_array("The quick brown fox jumps over the lazy dog 123456790 times. \x01\x02\x07\x00 Hell"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEAAQEBYAE" "BEGK3DM")
  },
  {
    byte_vector_from_const_char_array("The quick brown fox jumps over the lazy dog 123456790 times. \x01\x02\x07\x00 Hello"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEAAQEBYAE" "BEGK3DMN4======")
  },
  {
    byte_vector_from_const_char_array("The quick brown fox jumps over the lazy dog 123456790 times. \x01\x02\x07\x00 Hello "),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEAAQEBYAE" "BEGK3DMN4QA====")
  },
  {
    byte_vector_from_const_char_array("The quick brown fox jumps over the lazy dog 123456790 times. \x01\x02\x07\x00 Hello B"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEAAQEBYAE" "BEGK3DMN4QEE===")
  },
  {
    byte_vector_from_const_char_array("The quick brown fox jumps over the lazy dog 123456790 times. \x01\x02\x07\x00 Hello Ba"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEAAQEBYAE" "BEGK3DMN4QEEYI=")
  },
  {
    byte_vector_from_const_char_array("The quick brown fox jumps over the lazy dog 123456790 times. \x01\x02\x07\x00 Hello Bas"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEAAQEBYAE" "BEGK3DMN4QEEYLT")
  },
  {
    byte_vector_from_const_char_array("The quick brown fox jumps over the lazy dog 123456790 times. \x01\x02\x07\x00 Hello Base"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEAAQEBYAE" "BEGK3DMN4QEEYLT" "MU======")
  },
  {
    byte_vector_from_const_char_array("The quick brown fox jumps over the lazy dog 123456790 times. \x01\x02\x07\x00 Hello Base3"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEAAQEBYAE" "BEGK3DMN4QEEYLT" "MUZQ====")
  },
  {
    byte_vector_from_const_char_array("The quick brown fox jumps over the lazy dog 123456790 times. \x01\x02\x07\x00 Hello Base32"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEAAQEBYAE" "BEGK3DMN4QEEYLT" "MUZTE===")
  },
  {
    byte_vector_from_const_char_array("The quick brown fox jumps over the lazy dog 123456790 times. \x01\x02\x07\x00 Hello Base32 "),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEAAQEBYAE" "BEGK3DMN4QEEYLT" "MUZTEIA=")
  },
  {
    byte_vector_from_const_char_array("The quick brown fox jumps over the lazy dog 123456790 times. \x01\x02\x07\x00 Hello Base32 W"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEAAQEBYAE" "BEGK3DMN4QEEYLT" "MUZTEICX")
  },
  {
    byte_vector_from_const_char_array("The quick brown fox jumps over the lazy dog 123456790 times. \x01\x02\x07\x00 Hello Base32 Wo"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEAAQEBYAE" "BEGK3DMN4QEEYLT" "MUZTEICXN4=====" "=")
  },
  {
    byte_vector_from_const_char_array("The quick brown fox jumps over the lazy dog 123456790 times. \x01\x02\x07\x00 Hello Base32 Wor"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEAAQEBYAE" "BEGK3DMN4QEEYLT" "MUZTEICXN5ZA===" "=")
  },
  {
    byte_vector_from_const_char_array("The quick brown fox jumps over the lazy dog 123456790 times. \x01\x02\x07\x00 Hello Base32 Worl"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEAAQEBYAE" "BEGK3DMN4QEEYLT" "MUZTEICXN5ZGY==" "=")
  },
  {
    byte_vector_from_const_char_array("The quick brown fox jumps over the lazy dog 123456790 times. \x01\x02\x07\x00 Hello Base32 World"),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEAAQEBYAE" "BEGK3DMN4QEEYLT" "MUZTEICXN5ZGYZA" "=")
  },
  {
    byte_vector_from_const_char_array("The quick brown fox jumps over the lazy dog 123456790 times. \x01\x02\x07\x00 Hello Base32 World."),
    string("KRUGKIDROVUWG2Z" "AMJZG653OEBTG66" "BANJ2W24DTEBXXM" "ZLSEB2GQZJANRQX" "U6JAMRXWOIBRGIZ" "TINJWG44TAIDUNF" "WWK4ZOEAAQEBYAE" "BEGK3DMN4QEEYLT" "MUZTEICXN5ZGYZB" "O")
  }
};

TEST(Base32Test, base32_encode_test)
{
  int i = 0;
  for (auto& tc : Base32_TestVectors) {
    const auto& decoded_bytes = get<0>(tc);
    auto encoded_string = string(get<1>(tc));

    // Chromium base32.c/base32.h does not support RFC4648 base32 padding.
    // Remove padding, give/expect unpadding base32 string.
    while (!encoded_string.empty() && *encoded_string.rbegin() == '=')
      encoded_string.pop_back();

    // Expected encoded string length plus null terminator.
    string encoded_string_actual;
    encoded_string_actual.resize(encoded_string.size() + 1);

    bool result = base32_encode(
      &encoded_string_actual[0],
      static_cast<int>(encoded_string_actual.size()),
      decoded_bytes.empty() ? nullptr : &decoded_bytes[0],
      static_cast<int>(decoded_bytes.size() * 8),
      0
    );

    encoded_string_actual.resize(strlen(encoded_string_actual.c_str()));

    EXPECT_TRUE(result)
      << "Test vector " << i << ": base32_encode must be successful.";
    EXPECT_TRUE(encoded_string_actual == encoded_string)
      << "base32_encode: Test vector " << i << ": expected and actual encoded strings must match.";
    i++;
  }
}

TEST(Base32Test, base32_decode_test)
{
  int i = 0;
  for (auto& tc : Base32_TestVectors) {
    const auto& decoded_bytes = get<0>(tc);
    string encoded_string(get<1>(tc).begin(), get<1>(tc).end());

    // Chromium base32.c/base32.h does not support RFC4648 base32 padding.
    // Remove padding, give/expect unpadding base32 string.
    while (!encoded_string.empty() && *encoded_string.rbegin() == '=')
      encoded_string.pop_back();

    byte_vector decoded_string_actual;
    decoded_string_actual.resize(decoded_bytes.size());

    bool result = base32_decode(
      decoded_string_actual.empty() ? nullptr : &decoded_string_actual[0],
      static_cast<int>(decoded_string_actual.size()) * 8,
      encoded_string.c_str(),
      0
    );

    EXPECT_TRUE(result)
      << "Test vector " << i << ": base32_decode must be successful.";
    EXPECT_TRUE(decoded_string_actual == decoded_bytes)
      << "base32_decode: Test vector " << i << ": expected and actual encoded strings must match.";
    i++;
  }
}
