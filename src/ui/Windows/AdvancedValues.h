/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// AdvancedValues.h : header file
//

#pragma once

#include "core/ItemData.h"
#include <bitset>

namespace WZAdvanced {
  enum AdvType {INVALID = -1,
                COMPARE = 0,
                MERGE,
                SYNCH,
                EXPORT_TEXT,
                EXPORT_ENTRYTEXT,
                EXPORT_XML,
                EXPORT_ENTRYXML,
                COMPARESYNCH,
                LAST};
}

struct st_SaveAdvValues {
  st_SaveAdvValues()
  : subgroup_name(L""),
    subgroup_set(BST_UNCHECKED), subgroup_case(BST_UNCHECKED), 
    treatwhitespaceasempty(BST_CHECKED),
    subgroup_object(0), subgroup_function(0)
  {
    bsFields.set();
  }

  st_SaveAdvValues(const st_SaveAdvValues &adv)
    : bsFields(adv.bsFields), subgroup_name(adv.subgroup_name),
    subgroup_set(adv.subgroup_set), subgroup_object(adv.subgroup_object),
    subgroup_function(adv.subgroup_function), subgroup_case(adv.subgroup_case),
    treatwhitespaceasempty(adv.treatwhitespaceasempty)
  {
  }

  st_SaveAdvValues &operator =(const st_SaveAdvValues &adv)
  {
    if (this != &adv) {
      bsFields = adv.bsFields;
      subgroup_name = adv.subgroup_name;
      subgroup_set = adv.subgroup_set;
      subgroup_object = adv.subgroup_object;
      subgroup_function = adv.subgroup_function;
      subgroup_case = adv.subgroup_case;
      treatwhitespaceasempty = adv.treatwhitespaceasempty;
    }
    return *this;
  }

  void Clear() {
    bsFields.set();
    subgroup_set = subgroup_case = BST_UNCHECKED;
    treatwhitespaceasempty = BST_CHECKED;
    subgroup_object = subgroup_function = 0;
    subgroup_name = L"";
  }

  CItemData::FieldBits bsFields;
  CString subgroup_name;
  int subgroup_set, subgroup_object, subgroup_function, subgroup_case;
  int treatwhitespaceasempty;
};
