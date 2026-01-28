/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// CustomFields.cpp
//-----------------------------------------------------------------------------

#include "CustomFields.h"
#include "UTF8Conv.h"
#include "StringXStream.h"

#include <iomanip>
#include <unordered_set>

static bool ReadHexValue(const unsigned char *data, size_t len, unsigned int &value)
{
  StringX hexstr;
  hexstr.reserve(len);
  for (size_t i = 0; i < len; i++)
    hexstr += static_cast<wchar_t>(data[i]);

  iStringXStream iss(hexstr);
  iss >> std::hex >> value;
  return !iss.fail() && iss.eof();
}

static bool IsSeparator(const unsigned char *data, size_t remaining)
{
  return remaining >= 6 &&
         data[0] == '0' && data[1] == '0' &&
         data[2] == '0' && data[3] == '0' &&
         data[4] == '0' && data[5] == '0';
}

StringX CustomField::GetName() const
{
  for (const auto &prop : m_props) {
    if (prop.first == PROP_NAME)
      return prop.second;
  }
  return _T("");
}

StringX CustomField::GetValue() const
{
  for (const auto &prop : m_props) {
    if (prop.first == PROP_VALUE)
      return prop.second;
  }
  return _T("");
}

bool CustomField::GetSensitive(bool &sensitive) const
{
  for (const auto &prop : m_props) {
    if (prop.first == PROP_SENSITIVE) {
      if (prop.second.length() == 1)
        sensitive = (prop.second[0] != _T('0'));
      else
        sensitive = false;
      return true;
    }
  }
  sensitive = false;
  return false;
}

bool CustomField::IsSensitive() const
{
  bool sensitive = false;
  GetSensitive(sensitive);
  return sensitive;
}

bool CustomField::HasProperty(unsigned char id) const
{
  for (const auto &prop : m_props) {
    if (prop.first == id)
      return true;
  }
  return false;
}

void CustomField::SetName(const StringX &name)
{
  SetProperty(PROP_NAME, name);
}

void CustomField::SetValue(const StringX &value)
{
  SetProperty(PROP_VALUE, value);
}

void CustomField::SetSensitive(bool sensitive)
{
  StringX value;
  value += sensitive ? _T('1') : _T('0');
  SetProperty(PROP_SENSITIVE, value);
}

void CustomField::SetProperty(unsigned char id, const StringX &value)
{
  for (auto &prop : m_props) {
    if (prop.first == id) {
      prop.second = value;
      return;
    }
  }
  m_props.push_back(CustomFieldProperty(id, value));
}

CustomFieldList::CustomFieldList(const StringX &data) : m_numErr(0)
{
  if (data.empty())
    return;

  CUTF8Conv utf8conv;
  const unsigned char *utf8 = nullptr;
  size_t utf8Len = 0;
  if (!utf8conv.ToUTF8(data, utf8, utf8Len)) {
    m_numErr = 1;
    return;
  }

  std::unordered_set<StringX> names;
  size_t pos = 0;
  CustomField current;
  while (pos < utf8Len) {
    if (IsSeparator(utf8 + pos, utf8Len - pos)) {
      if (!current.Empty()) {
        if (!current.HasProperty(CustomField::PROP_NAME))
          m_numErr++;
        else {
          const StringX name = current.GetName();
          if (name.empty() || !names.insert(name).second)
            m_numErr++;
        }
        // Auto-add missing value property as empty string.
        if (!current.HasProperty(CustomField::PROP_VALUE))
          current.SetProperty(CustomField::PROP_VALUE, _T(""));
        push_back(current);
        current = CustomField();
      }
      pos += 6;
      continue;
    }

    if (utf8Len - pos < 6) {
      m_numErr++;
      break;
    }

    unsigned int prop_id = 0;
    unsigned int value_len = 0;
    if (!ReadHexValue(utf8 + pos, 2, prop_id) ||
        !ReadHexValue(utf8 + pos + 2, 4, value_len)) {
      m_numErr++;
      break;
    }
    if (prop_id == 0 || prop_id > 0xff) {
      m_numErr++;
      break;
    }
    pos += 6;

    size_t value_len_sz = static_cast<size_t>(value_len);
    if (pos + value_len_sz > utf8Len) {
      m_numErr++;
      break;
    }

    StringX value;
    // Special-case sensitivity: avoid UTF-8 decode for 1-byte value and normalize
    // 0x00 or '0' -> '0', any other byte -> '1'.
    if (prop_id == CustomField::PROP_SENSITIVE && value_len_sz == 1) {
      const unsigned char byte_value = utf8[pos];
      value += (byte_value == 0 || byte_value == '0') ? _T('0') : _T('1');
    } else {
      VectorX<unsigned char> buf(utf8 + pos, utf8 + pos + value_len_sz);
      buf.push_back(0); // null terminate for FromUTF8.
      if (!utf8conv.FromUTF8(&buf[0], value_len_sz, value)) {
        m_numErr++;
        break;
      }
    }
    pos += value_len_sz;

    if (current.HasProperty(static_cast<unsigned char>(prop_id))) {
      m_numErr++;
      continue;
    }
    if (prop_id == CustomField::PROP_SENSITIVE) {
      if (value_len_sz != 1)
        m_numErr++;
    }
    current.SetProperty(static_cast<unsigned char>(prop_id), value);
  }

  if (!current.Empty()) {
    if (!current.HasProperty(CustomField::PROP_NAME))
      m_numErr++;
    else {
      const StringX name = current.GetName();
      if (name.empty() || !names.insert(name).second)
        m_numErr++;
    }
    // Auto-add missing value property as empty string.
    if (!current.HasProperty(CustomField::PROP_VALUE))
      current.SetProperty(CustomField::PROP_VALUE, _T(""));
    push_back(current);
  }
}

CustomFieldList::operator StringX() const
{
  if (empty())
    return _T("");

  CUTF8Conv utf8conv;
  StringX out;
  bool wrote_field = false;

  for (auto field_iter = begin(); field_iter != end(); ++field_iter) {
    const CustomField &field = *field_iter;
    if (field.Empty())
      continue;

    if (wrote_field)
      out += _T("000000");

    const auto &props = field.GetProperties();
    for (const auto &prop : props) {
      if (prop.first == 0) {
        ASSERT(0);
        continue;
      }

      const unsigned char *utf8 = nullptr;
      size_t utf8Len = 0;
      if (!utf8conv.ToUTF8(prop.second, utf8, utf8Len)) {
        ASSERT(0);
        continue;
      }
      if (utf8Len > 0xffff) {
        ASSERT(0);
        continue;
      }

      oStringXStream oss;
      oss.fill(charT('0'));
      oss << std::hex << std::setw(2) << static_cast<int>(prop.first)
          << std::setw(4) << static_cast<unsigned int>(utf8Len);
      out += oss.str().c_str();
      out += prop.second;
    }
    wrote_field = true;
  }

  return out;
}
