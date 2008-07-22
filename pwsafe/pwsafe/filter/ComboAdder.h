/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * Silly helper struct for the common operation of adding CStrings
 * from an STL container class to a CComboBox.
 * doit() is a "specialization" for std::vector<CString>
 *
 * Usage:
 * ComboAdder ca(combo_box);
 * for_each(list.begin(), list.end(), ca);
 *
 * or simply
 *
 * ComboAdder ca(combo_box);
 * ca.doit(vec_str);
 */
#pragma once
#include <algorithm>

struct ComboAdder
{
  ComboAdder(CComboBox &cb) : m_cb(cb) {}
  void operator()(const CString &str) {m_cb.AddString(str);}
  void doit(const std::vector<CString> vec)
  { std::for_each(vec.begin(), vec.end(), *this); }
private:
  CComboBox &m_cb;
};
