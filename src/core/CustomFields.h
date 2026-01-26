/*
* Copyright (c) 2003-2026 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// CustomFields.h
//-----------------------------------------------------------------------------

#ifndef __CUSTOMFIELDS_H
#define __CUSTOMFIELDS_H

#include "StringX.h"

#include <utility>
#include <vector>

//-----------------------------------------------------------------------------

/*
* CustomField and CustomFieldList provide a structured representation of the
* "Custom Text Field" entry (0x30) stored as a TLV-like UTF-8 string in the
* database record.
*/

using CustomFieldProperty = std::pair<unsigned char, StringX>;

class CustomField
{
public:
  enum PropertyId {
    PROP_NAME = 0x01,
    PROP_VALUE = 0x02,
    PROP_SENSITIVE = 0x03
  };

  StringX GetName() const;
  StringX GetValue() const;
  bool GetSensitive(bool &sensitive) const;
  bool IsSensitive() const;
  bool HasProperty(unsigned char id) const;

  void SetName(const StringX &name);
  void SetValue(const StringX &value);
  void SetSensitive(bool sensitive);
  void SetProperty(unsigned char id, const StringX &value);

  const std::vector<CustomFieldProperty> &GetProperties() const { return m_props; }
  bool Empty() const { return m_props.empty(); }

private:
  std::vector<CustomFieldProperty> m_props;
};

class CustomFieldList : public std::vector<CustomField>
{
public:
  CustomFieldList() : m_numErr(0) {}
  CustomFieldList(const StringX &data);

  // Callers should check getErr() before using parsed data.
  size_t getErr() const { return m_numErr; }
  operator StringX() const;

private:
  size_t m_numErr;
};

#endif /* __CUSTOMFIELDS_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
