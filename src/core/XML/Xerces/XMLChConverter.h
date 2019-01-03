#ifndef __XMLCH_CONVERTER_H__
#define __XMLCH_CONVERTER_H__

/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#ifdef WCHAR_INCOMPATIBLE_XMLCH

#include "../../UTF8Conv.h"

#include <map>
#include <algorithm>

struct _XMLChDeallocator {
  typedef std::map<const char*, XMLCh*> XMLChStrings;
  XMLChStrings allocations;

  XMLCh* Ansi2Xml(const char* s) {
    XMLChStrings::iterator itr = allocations.find(s);
    if (itr != allocations.end())
      return itr->second;

    XMLCh* newstr = XMLString::transcode(s);
    allocations[s] = newstr;
    return newstr;
  }

  //this should get used for dynamic strings, so we don't need to
  //optimize the lookups with its own map
  XMLCh* WChar2Xml(const wchar_t* ws) {
    CUTF8Conv conv;
    const unsigned char* data;
    size_t len;
    if (conv.ToUTF8(StringX(ws), data, len)) { //conv would release data
      const char* utf8str = reinterpret_cast<const char *>(data);
      XMLCh* newstr = XMLString::transcode(utf8str);
      allocations[utf8str] = newstr;
      return newstr;
    }
    return 0;
  }

  static StringX Xml2StringX(const XMLCh* xs) {
    char *astr = XMLString::transcode(xs);
    StringX ws;
    if (astr) {
      //transcode() apparently converts the string to "Native code page"
      //Hopefully, that overlaps just fine with utf-8
      CUTF8Conv conv;
      if (!conv.FromUTF8(reinterpret_cast<unsigned char *>(astr), strlen(astr), ws)) {
        ws = _T("");
      }
      XMLString::release(&astr);
    }
    return ws;
  }

  static stringT Xml2StringT(const XMLCh* xs) {
    char *astr = XMLString::transcode(xs);
    StringX ws;
    if (astr) {
      CUTF8Conv conv;
      //This is wrong if the second param of FromUTF8 is the number of chars
      if (!conv.FromUTF8(reinterpret_cast<unsigned char *>(astr), strlen(astr), ws)) {
        ws = _T("");
      }
      XMLString::release(&astr);
    }
    return stringT(ws.begin(), ws.end());
  }

  void Release(XMLChStrings::value_type val) {
    XMLString::release(&val.second);
  }

  void Clear() {
    using namespace std;
    for_each(allocations.begin(), allocations.end(), bind1st(mem_fun(&_XMLChDeallocator::Release), this));
      allocations.clear();
  }

  ~_XMLChDeallocator() {
    Clear();
  }
};

#define USES_XMLCH_STR _XMLChDeallocator __xmlch_deallocator;
#define USES_XMLCH_STR_END __xmlch_deallocator.Clear();

#define _A2X(s) __xmlch_deallocator.Ansi2Xml(s)
#define _W2X(ws) __xmlch_deallocator.WChar2Xml(ws)
#define _X2SX(x) _XMLChDeallocator::Xml2StringX(x)
#define _X2ST(x) _XMLChDeallocator::Xml2StringT(x)

#else

#define _A2X(s) L##s
#define _W2X(ws) ws
#define _X2SX(x) x
#define _X2ST(x) x

#define USES_XMLCH_STR
#define USES_XMLCH_STR_END

#endif

#endif
