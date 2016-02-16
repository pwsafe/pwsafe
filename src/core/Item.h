/*
* Copyright (c) 2003-2016 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// Item.h
//-----------------------------------------------------------------------------

#ifndef __ITEM_H
#define __ITEM_H

#include "ItemField.h"
#include "Util.h"
#include "StringX.h"

#include <vector>
#include <string>
#include <map>

//-----------------------------------------------------------------------------

/**
 * CItem is the base class for items stored in the database.
 * CItemData is one such item, representing a password entry
 * CItemAtt is another, representing a attachment (new in V4)
 *
 * What makes this class interesting is that all fields are kept encrypted
 * from the moment of construction, and are decrypted by the appropriate
 * accessor. This encryption is orthogonal to the encryption of data on disk.
 *
 * Since the number of fields is relatively large and evolves over time, we
 * keep them in a map that's keyed on their type. For convenience, setters
 * and getters can be defined in derived classes. These also convert the
 * raw bytes (stored encrypted) to/from the relevant representation, e.g.,
 * string, time, etc.
 *
 * UnknownFields are used from forward compatability. If there's a field type
 * we don't recognize, we just store it as-is, so that we can write it when saving.
 *
*/

class BlowFish;

struct DisplayInfoBase
{
  // Following used by display methods of the GUI
  DisplayInfoBase() {}
  virtual ~DisplayInfoBase() {}
  virtual DisplayInfoBase *clone() const = 0; // virtual c'tor idiom
};

class CItem
{
public:
  typedef std::vector<CItemField> UnknownFields;
  typedef UnknownFields::const_iterator UnknownFieldsConstIter;

  // Status returns from "ProcessInputRecordField"
  enum {SUCCESS = 0, FAILURE, END_OF_FILE = 8};

  // Entry status (note: powers of 2)
  // A status can (currently) have values:
  //   0 (normal), 1 (added), 2 (modified) or 4 (deleted).
  enum EntryStatus {ES_INVALID      = -1,
                    ES_CLEAN        =  0,
                    ES_ADDED        =  1,  // Added    but not yet saved to disk copy
                    ES_MODIFIED     =  2,  // Modified but not yet saved to disk copy
                    ES_DELETED      =  4,  // Deleted  but not yet removed from disk copy
                    ES_LAST};

  //Construction
  CItem();
  CItem(const CItem& stuffhere);

  ~CItem();

  void SetUnknownField(unsigned char type, size_t length,
                       const unsigned char *ufield);
  size_t NumberUnknownFields() const {return m_URFL.size();}

  CItem& operator=(const CItem& second);
  // Following used by display methods - we just keep it handy
  DisplayInfoBase *GetDisplayInfo() const {return m_display_info;}
  void SetDisplayInfo(DisplayInfoBase *di) {delete m_display_info; m_display_info = di;}
  void Clear();
  void ClearField(int ft) {m_fields.erase(ft);}

  bool operator==(const CItem &that) const;

  size_t GetSize() const;
  void GetSize(size_t &isize) const {isize = GetSize();}

protected:
  typedef std::map<int, CItemField> FieldMap;
  typedef FieldMap::const_iterator FieldConstIter;
  typedef FieldMap::iterator FieldIter;

  FieldMap   m_fields;

  // Save unknown record fields on read to put back on write unchanged
  UnknownFields m_URFL;

  void SetField(int ft, const unsigned char *value, size_t length);
  void SetField(int ft, const StringX &value);
  bool SetTextField(int ft, const unsigned char *value, size_t length);
  bool SetTimeField(int ft, const unsigned char *value, size_t length);

  void GetField(const CItemField &field, unsigned char *value,
                size_t &length) const;
  StringX GetField(int ft) const;
  StringX GetField(const CItemField &field) const;

  void SetTime(const int whichtime, time_t t);
  void GetTime(int whichtime, time_t &t) const;

  bool IsFieldSet(int ft) const {return m_fields.find(ft) != m_fields.end();}

  void GetUnknownField(unsigned char &type, size_t &length,
                       unsigned char * &pdata, const CItemField &item) const;
private:
  // Helper function for operator==
  bool CompareFields(const CItemField &fthis,
                     const CItem &that, const CItemField &fthat) const;

  // Create local Encryption/Decryption object
  BlowFish *MakeBlowFish() const;

  // random key for storing stuff in memory
  // We need to keep the key because it's easier to copy
  // than the BlowFish object for copy c'tor and assignment
  unsigned char m_key[32];
  mutable BlowFish *m_blowfish = nullptr;

  // Following used by display methods - we just keep it handy
  DisplayInfoBase *m_display_info = nullptr;
};

#endif /* __ITEM_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
