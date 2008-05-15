/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
/// \file PWCharPool.cpp
//-----------------------------------------------------------------------------

#include "PWCharPool.h"
#include "Util.h"
#include "corelib.h"
#include "PWSrand.h"
#include "trigram.h" // for pronounceable passwords
#include "os/typedefs.h"
#include "os/pws_tchar.h"
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

// Following macro get length of std_*_chars less the trailing \0
// compile time equivalent of strlen()
#define LENGTH(s) (sizeof(s)/sizeof(s[0]) - sizeof(s[0]))

const charT CPasswordCharPool::std_lowercase_chars[] = _T("abcdefghijklmnopqrstuvwxyz");
const size_t CPasswordCharPool::std_lowercase_len = LENGTH(std_lowercase_chars);

const charT CPasswordCharPool::std_uppercase_chars[] = _T("ABCDEFGHIJKLMNOPQRSTUVWXYZ");
const size_t CPasswordCharPool::std_uppercase_len = LENGTH(std_uppercase_chars);

const charT CPasswordCharPool::std_digit_chars[] = _T("0123456789");
const size_t CPasswordCharPool::std_digit_len = LENGTH(std_digit_chars);

const charT CPasswordCharPool::std_hexdigit_chars[] = _T("0123456789abcdef");
const size_t CPasswordCharPool::std_hexdigit_len = LENGTH(std_hexdigit_chars);

const charT CPasswordCharPool::std_symbol_chars[] = _T("+-=_@#$%^&;:,.<>/~\\[](){}?!|");
const size_t CPasswordCharPool::std_symbol_len = LENGTH(std_symbol_chars);

const charT CPasswordCharPool::easyvision_lowercase_chars[] = _T("abcdefghijkmnopqrstuvwxyz");
const size_t CPasswordCharPool::easyvision_lowercase_len = LENGTH(easyvision_lowercase_chars);

const charT CPasswordCharPool::easyvision_uppercase_chars[] = _T("ABCDEFGHJKLMNPQRTUVWXY");
const size_t CPasswordCharPool::easyvision_uppercase_len = LENGTH(easyvision_uppercase_chars);

const charT CPasswordCharPool::easyvision_digit_chars[] = _T("346789");
const size_t CPasswordCharPool::easyvision_digit_len = LENGTH(easyvision_digit_chars);

const charT CPasswordCharPool::easyvision_symbol_chars[] = _T("+-=_@#$%^&<>/~\\?");
const size_t CPasswordCharPool::easyvision_symbol_len = LENGTH(easyvision_symbol_chars);

const charT CPasswordCharPool::easyvision_hexdigit_chars[] = _T("0123456789abcdef");
const size_t CPasswordCharPool::easyvision_hexdigit_len = LENGTH(easyvision_hexdigit_chars);

//-----------------------------------------------------------------------------

CPasswordCharPool::CPasswordCharPool(uint pwlen,
                                     uint numlowercase, uint numuppercase,
                                     uint numdigits, uint numsymbols,
                                     bool usehexdigits, bool easyvision,
                                     bool pronounceable)
  : m_pwlen(pwlen),
  m_numlowercase(numlowercase), m_numuppercase(numuppercase),
  m_numdigits(numdigits), m_numsymbols(numsymbols),
  m_uselowercase(numlowercase > 0 ? TRUE : FALSE),
  m_useuppercase(numuppercase > 0 ? TRUE : FALSE),
  m_usedigits(numdigits > 0 ? TRUE : FALSE),
  m_usesymbols(numsymbols > 0 ? TRUE : FALSE),
  m_usehexdigits(usehexdigits), m_pronounceable(pronounceable)
{
  ASSERT(m_pwlen > 0);
  ASSERT(m_uselowercase || m_useuppercase || 
    m_usedigits || m_usesymbols || 
    m_usehexdigits || m_pronounceable);

  if (easyvision) {
    m_char_arrays[LOWERCASE] = (charT *)easyvision_lowercase_chars;
    m_char_arrays[UPPERCASE] = (charT *)easyvision_uppercase_chars;
    m_char_arrays[DIGIT] = (charT *)easyvision_digit_chars;
    m_char_arrays[SYMBOL] = (charT *)easyvision_symbol_chars;
    m_char_arrays[HEXDIGIT] = (charT *)easyvision_hexdigit_chars;
    m_lengths[LOWERCASE] = m_uselowercase ? easyvision_lowercase_len : 0;
    m_lengths[UPPERCASE] = m_useuppercase ? easyvision_uppercase_len : 0;
    m_lengths[DIGIT] = m_usedigits ? easyvision_digit_len : 0;
    m_lengths[SYMBOL] = m_usesymbols ? easyvision_symbol_len : 0;
    m_lengths[HEXDIGIT] = m_usehexdigits ? easyvision_hexdigit_len : 0;
  } else { // !easyvision
    m_char_arrays[LOWERCASE] = (charT *)std_lowercase_chars;
    m_char_arrays[UPPERCASE] = (charT *)std_uppercase_chars;
    m_char_arrays[DIGIT] = (charT *)std_digit_chars;
    m_char_arrays[SYMBOL] = (charT *)std_symbol_chars;
    m_char_arrays[HEXDIGIT] = (charT *)std_hexdigit_chars;
    m_lengths[LOWERCASE] = m_uselowercase ? std_lowercase_len : 0;
    m_lengths[UPPERCASE] = m_useuppercase ? std_uppercase_len : 0;
    m_lengths[DIGIT] = m_usedigits ? std_digit_len : 0;
    m_lengths[SYMBOL] = m_usesymbols ? std_symbol_len : 0;
    m_lengths[HEXDIGIT] = m_usehexdigits ? std_hexdigit_len : 0;
  }

  // See GetRandomCharType to understand what this does and why
  m_x[0] = 0;
  m_sumlengths = 0;
  for (int i = 0; i< NUMTYPES; i++) {
    m_x[i+1] = m_x[i] + m_lengths[i];
    m_sumlengths += m_lengths[i];
  }
  ASSERT(m_sumlengths > 0);
}

CPasswordCharPool::CharType CPasswordCharPool::GetRandomCharType(unsigned int rand) const
{
  /*
  * Following is needed in order to choose a char type with a probability
  * in proportion to its relative size, i.e., if chartype 'A' has 20 characters,
  * and chartype 'B' has 10, then the generated password will have twice as
  * many chars from 'A' than as from 'B'.
  * Think of m_x as the number axis. We treat the chartypes as intervals which
  * are laid out successively along the axis. Each number in m_x[] specifies
  * where one interval ends and the other begins. Choosing a chartype is then
  * reduced to seeing in which interval a random number falls.
  * The nice part is that this works correctly for non-selected chartypes
  * without any special logic.
  */
  int i;
  for (i = 0; i < NUMTYPES; i++) {
    if (rand < m_x[i+1]) {
      break;
    }
  }

  ASSERT(m_lengths[i] > 0 && i < NUMTYPES);
  return CharType(i);
}

charT CPasswordCharPool::GetRandomChar(CPasswordCharPool::CharType t, unsigned int rand) const
{
  ASSERT(t < NUMTYPES);
  ASSERT(m_lengths[t] > 0);
  rand %= m_lengths[t];

  charT retval = m_char_arrays[t][rand];
  return retval;
}

CMyString CPasswordCharPool::MakePassword() const
{
  ASSERT(m_pwlen > 0);
  ASSERT(m_uselowercase || m_useuppercase || m_usedigits ||
         m_usesymbols || m_usehexdigits || m_pronounceable);

  int lowercaseneeded, uppercaseneeded, digitsneeded, symbolsneeded;
  int hexdigitsneeded;

  CMyString password = _T("");

  // pronounceable passwords are handled separately:
  if (m_pronounceable)
    return MakePronounceable();

  bool pwRulesMet;
  CMyString temp;

  do {
    charT ch;
    CharType type;

    lowercaseneeded = m_numlowercase;
    uppercaseneeded = m_numuppercase;
    digitsneeded = m_numdigits;
    symbolsneeded = m_numsymbols;
    hexdigitsneeded = (m_usehexdigits) ? 1 : 0;

    // If following assertion doesn't hold, we'll never exit the do loop!
    ASSERT(int(m_pwlen) >= lowercaseneeded + uppercaseneeded +
      digitsneeded + symbolsneeded + hexdigitsneeded);

    temp = _T("");    // empty the password string

    for (uint x = 0; x < m_pwlen; x++) {
      unsigned int rand = PWSrand::GetInstance()->RangeRand((unsigned int)m_sumlengths);
      // The only reason for passing rand as a parameter is to
      // avoid having to generate two random numbers for each
      // character. Alternately, we could have had a m_rand
      // data member. Which solution is uglier is debatable.
      type = GetRandomCharType(rand);
      ch = GetRandomChar(type, rand);
      temp += ch;
      /*
      **  Decrement the appropriate needed character type count.
      */
      switch (type) {
        case LOWERCASE:
          lowercaseneeded--;
          break;

        case UPPERCASE:
          uppercaseneeded--;
          break;

        case DIGIT:
          digitsneeded--;
          break;

        case SYMBOL:
          symbolsneeded--;
          break;

        case HEXDIGIT:
          hexdigitsneeded--;
          break;

        default:
          ASSERT(0); // should never happen!
          break;
      }
    } // for

    /*
    * Make sure we have at least one representative of each required type
    * after the for loop. If not, try again. Arguably, recursion would have
    * been more elegant than a do loop, but this takes less stack...
    */
    pwRulesMet = (lowercaseneeded <= 0 && uppercaseneeded <= 0 &&
      digitsneeded <= 0 && symbolsneeded <= 0 && 
      hexdigitsneeded <= 0);

    if (pwRulesMet) {
      password = temp;
    }
    // Otherwise, do not exit, do not collect $200, try again...
  } while (!pwRulesMet);
  ASSERT(password.GetLength() == int(m_pwlen));
  return password;
}

static const struct {
  charT num; charT sym;
} leets[26] = {
  {charT('4'), charT('@')}, {charT('8'), charT('&')}, // a, b
  {0, charT('(')}, {0, 0},                            // c, d
  {charT('3'), 0}, {0, 0},                            // e, f
  {charT('6'), 0}, {0, charT('#')},                   // g, h
  {charT('1'), charT('!')}, {0, 0},                   // i, j
  {0, 0}, {charT('1'), charT('|')},                   // k, l
  {0, 0}, {0, 0},                                     // m, n
  {charT('0'), 0}, {0, 0},                            // o, p
  {0, 0}, {0, 0},                                     // q, r
  {charT('5'), charT('$')}, {charT('7'), charT('+')}, // s, t
  {0, 0}, {0, 0},                                     // u, v
  {0, 0}, {0, 0},                                     // w, x
  {0, 0}, {charT('2'), 0}};                           // y, z

class FillSC {
  // used in for_each to find Substitution Candidates
  // for "leet" alphabet for pronounceable passwords
  // with usesymbols and/or usedigits specified
public:
  FillSC(vector<int> &sc, bool digits, bool symbols)
    : m_sc(sc), m_digits(digits), m_symbols(symbols), m_i(0) {}
  void operator()(charT t) {
    if ((m_digits && leets[t - charT('a')].num != 0) ||
      (m_symbols &&leets[t - charT('a')].sym != 0))
      m_sc.push_back(m_i);
    m_i++;
  }
private:
  vector<int> &m_sc;
  bool m_digits, m_symbols;
  int m_i;
};

struct RandomWrapper {
  unsigned int operator()(unsigned int i)
  {return PWSrand::GetInstance()->RangeRand(i);}
};

static void leet_replace(stringT &password, unsigned int i,
                         bool usedigits, bool usesymbols)
{
  ASSERT(i < password.size());
  ASSERT(usedigits || usesymbols);

  charT digsub = usedigits ? leets[password[i] - charT('a')].num : 0;
  charT symsub = usesymbols ? leets[password[i] - charT('a')].sym : 0;

  // if both substitutions possible, select one randomly
  if (digsub != 0 && symsub != 0 && PWSrand::GetInstance()->RandUInt() % 2)
    digsub = 0;
  password[i] = (digsub != 0) ? digsub : symsub;
  ASSERT(password[i] != 0);
}

CMyString CPasswordCharPool::MakePronounceable() const
{
  /**
  * Following based on gpw.C from
  * http://www.multicians.org/thvv/tvvtools.html
  * Thanks to Tom Van Vleck, Morrie Gasser, and Dan Edwards.
  */
  charT c1, c2, c3;  /* array indices */
  long sumfreq;      /* total frequencies[c1][c2][*] */
  long ranno;        /* random number in [0,sumfreq] */
  long sum;          /* running total of frequencies */
  uint nchar;        /* number of chars in password so far */
  PWSrand *pwsrnd = PWSrand::GetInstance();
  stringT password(m_pwlen, 0);

  /* Pick a random starting point. */
  /* (This cheats a little; the statistics for three-letter
  combinations beginning a word are different from the stats
  for the general population.  For example, this code happily
  generates "mmitify" even though no word in my dictionary
  begins with mmi. So what.) */
  sumfreq = sigma;  // sigma calculated by loadtris
  ranno = (long)pwsrnd->RangeRand(sumfreq+1); // Weight by sum of frequencies
  sum = 0;
  for (c1 = 0; c1 < 26; c1++) {
    for (c2 = 0; c2 < 26; c2++) {
      for (c3 = 0; c3 < 26; c3++) {
        sum += tris[c1][c2][c3];
        if (sum > ranno) { // Pick first value
          password[0] = charT('a') + c1;
          password[1] = charT('a') + c2;
          password[2] = charT('a') + c3;
          c1 = c2 = c3 = 26; // Break all loops.
        } // if sum
      } // for c3
    } // for c2
  } // for c1

  /* Do a random walk. */
  nchar = 3;  // We have three chars so far.
  while (nchar < m_pwlen) {
    c1 = password[nchar-2] - charT('a'); // Take the last 2 chars
    c2 = password[nchar-1] - charT('a'); // .. and find the next one.
    sumfreq = 0;
    for (c3 = 0; c3 < 26; c3++)
      sumfreq += tris[c1][c2][c3];
    /* Note that sum < duos[c1][c2] because
    duos counts all digraphs, not just those
    in a trigraph. We want sum. */
    if (sumfreq == 0) { // If there is no possible extension..
      break;  // Break while nchar loop & print what we have.
    }
    /* Choose a continuation. */
    ranno = (long)pwsrnd->RangeRand(sumfreq+1); // Weight by sum of frequencies
    sum = 0;
    for (c3 = 0; c3 < 26; c3++) {
      sum += tris[c1][c2][c3];
      if (sum > ranno) {
        password[nchar++] = charT('a') + c3;
        c3 = 26;  // Break the for c3 loop.
      }
    } // for c3
  } // while nchar
  /*
  * password now has an all-lowercase pronounceable password
  * We now want to modify it per policy:
  * If m_usedigits and/or m_usesymbols, replace some chars with
  * corresponding 'leet' values
  * Also enforce m_useuppercase & m_uselowercase policies
  */

  if (m_usesymbols || m_usedigits) {
    // fill a vector with indices of substitution candidates
    vector<int> sc;
    FillSC fill_sc(sc, (m_usedigits == TRUE), (m_usesymbols == TRUE));
    for_each(password.begin(), password.end(), fill_sc);
    if (!sc.empty()) {
      // choose how many to replace (not too many, but at least one)
      unsigned int rn = pwsrnd->RangeRand(sc.size() - 1)/2 + 1;
      // replace some of them
      RandomWrapper rnw;
      random_shuffle(sc.begin(), sc.end(), rnw);
      for (unsigned int i = 0; i < rn; i++)
        leet_replace(password, sc[i], m_usedigits, m_usesymbols);
    }
  }
  // case
  uint i;
  if (m_uselowercase && !m_useuppercase)
    ; // nothing to do here
  else if (!m_uselowercase && m_useuppercase)
    for (i = 0; i < m_pwlen; i++) {
      if (_istalpha(password[i]))
        password[i] = (charT)_totupper(password[i]);
    }
  else if (m_uselowercase && m_useuppercase) // mixed case
    for (i = 0; i < m_pwlen; i++) {
      if (_istalpha(password[i]) && pwsrnd->RandUInt() % 2)
        password[i] = (charT)_totupper(password[i]);
    }

    CMyString retval = password.c_str();
    return retval;
}

bool CPasswordCharPool::CheckPassword(const CMyString &pwd, CMyString &error)
{
  const int MinLength = 8;
  int length = pwd.GetLength();
  // check for minimun length
  if (length < MinLength) {
    error.LoadString(IDSC_PASSWORDTOOSHORT);
    return false;
  }

  // check for at least one  uppercase and lowercase and  digit or other
  bool has_uc = false, has_lc = false, has_digit = false, has_other = false;

  for (int i = 0; i < length; i++) {
    charT c = pwd[i];
    if (_istlower(c)) has_lc = true;
    else if (_istupper(c)) has_uc = true;
    else if (_istdigit(c)) has_digit = true;
    else has_other = true;
  }

  if (has_uc && has_lc && (has_digit || has_other)) {
    return true;
  } else {
    error.LoadString(IDSC_PASSWORDPOOR);
    return false;
  }
}
