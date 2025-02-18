/*
* Copyright (c) 2003-2025 Rony Shapiro <ronys@pwsafe.org>.
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
 * UnknownFields are used from forward compatibility. If there's a field type
 * we don't recognize, we just store it as-is, so that we can write it when saving.
 *
*/

class BlowFish;

class CItem
{
public:
  // field types, per formatV{2,3,4}.txt. Any value > 0xff is internal only!
  enum FieldType {
    START = 0x00, GROUPTITLE = 0x00 /* reusing deprecated NAME for Group.Title combination */,
    NAME = 0x00,
    UUID = 0x01,
    GROUP = 0x02,
    TITLE = 0x03,
    USER = 0x04,
    NOTES = 0x05,
    PASSWORD = 0x06,
    CTIME = 0x07,                 // Entry 'C'reation time
    PMTIME = 0x08,                // last 'P'assword 'M'odification time
    ATIME = 0x09,                 // last 'A'ccess time
    XTIME = 0x0a,                 // password e'X'piry time
    RESERVED = 0x0b               /* MUST NOT USE */,
    RMTIME = 0x0c,                // last 'R'ecord 'M'odification time
    URL = 0x0d,
    AUTOTYPE = 0x0e,
    PWHIST = 0x0f,
    POLICY = 0x10,                // string encoding of item-specific password policy
    XTIME_INT = 0x11,
    RUNCMD = 0x12,
    DCA = 0x13,                   // doubleclick action (enum)
    EMAIL = 0x14,
    PROTECTED = 0x15,
    SYMBOLS = 0x16,               // string of item-specific password symbols
    SHIFTDCA = 0x17,              // shift-doubleclick action (enum)
    POLICYNAME = 0x18,            // named non-default password policy for item
    KBSHORTCUT = 0x19,            // Keyboard shortcuts
    ATTREF = 0x1a,                // UUID of attachment (v4)
    TWOFACTORKEY = 0x1b,          // Two-Factor Key
    CCNUM = 0x1c,                 // Credit card number
    CCEXP = 0x1d,                 // Credit card expiration date
    CCVV = 0x1e,                  // CVV / CVV2
    CCPIN = 0x1f,                 // Credit card PIN code
    TOTPCONFIG = 0x21,            // TOTP Config
    TOTPLENGTH = 0x22,            // TOTP Length
    TOTPTIMESTEP = 0x23,          // TOTP Time Step
    TOTPSTARTTIME = 0x24,         // TOTP Start Time
    LAST_USER_FIELD,              // All "user" fields MUST be before this for entry compare

    BASEUUID = 0x41,              // Base UUID of Alias or Shortcut (v4)
    ALIASUUID = 0x42,             // UUID indicates this is an Alias (v4)
    SHORTCUTUUID = 0x43,          // UUID indicates this is a Shortcut (v4)
    LAST_DATA,                    // Start of unknown fields!
    LAST_ITEM_DATA_FIELD = 0x5f,  // beyond this is for other CItem subclasses

    START_ATT = 0x60,
    ATTUUID = 0x60,
    ATTTITLE = 0x61,
    ATTCTIME = 0x62,
    MEDIATYPE = 0x63,
    FILENAME = 0x64,
    FILEPATH = 0x65,
    FILECTIME = 0x66,
    FILEMTIME = 0x67,
    FILEATIME = 0x68,
    LAST_SEARCHABLE = 0x6f,       // also last-filterable
    ATTEK = 0x70,
    ATTAK = 0x71,
    ATTIV = 0x72,
    CONTENT = 0x73,
    CONTENTHMAC = 0x74,
    LAST_ATT,

    UNKNOWN_TESTING = 0xdf,       // for testing forward compatibility (unknown field handling)
    END = 0xff,

    // Internal fields only - used in filters
    ENTRYSIZE = 0x100,
    ENTRYTYPE = 0x101,
    ENTRYSTATUS  = 0x102,
    PASSWORDLEN = 0x103,

    // 'UNKNOWNFIELDS' should be last
    UNKNOWNFIELDS = 0x104,
    LAST_FIELD
  };

  static bool IsTimeField(int ft)
  {
    return ft == CTIME || ft == PMTIME || ft == ATIME || ft == XTIME || ft == RMTIME ||
      ft == TOTPSTARTTIME ||
      ft == ATTCTIME || ft == FILECTIME || ft == FILEMTIME || ft == FILEATIME;
  }

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

  virtual ~CItem();

  void SetUnknownField(unsigned char type, size_t length,
                       const unsigned char *ufield);
  size_t NumberUnknownFields() const {return m_URFL.size();}

  CItem& operator=(const CItem& second);
  virtual void Clear();
  void ClearField(int ft) {m_fields.erase(ft);}

  void CopyTime(int ft, const CItem & src)
  {
    ASSERT(IsTimeField(ft));
    time_t t;
    src.GetTime(ft, t);
    SetTime(ft, t);
  }

  bool operator==(const CItem &that) const;

  size_t GetSize() const;
  void GetSize(size_t &isize) const {isize = GetSize();}
    
  void push_length(std::vector<char> &v, uint32 s) const;
  template< typename T> void push(std::vector<char> &v, char type, T value) const
  {
    if (value != 0) {
      v.push_back(type);
      push_length(v, sizeof(value));
      v.insert(v.end(), reinterpret_cast<char *>(&value), reinterpret_cast<char *>(&value) + sizeof(value));
    }
  }
  void push(std::vector<char> &v, char type, const StringX &str) const;

protected:
  typedef std::map<int, CItemField> FieldMap;
  typedef FieldMap::const_iterator FieldConstIter;
  typedef FieldMap::iterator FieldIter;

  FieldMap   m_fields;

  // Save unknown record fields on read to put back on write unchanged
  std::vector<CItemField> m_URFL;

  void SetField(int ft, const unsigned char *value, size_t length);
  void SetField(int ft, const StringX &value);
  bool SetTextField(int ft, const unsigned char *value, size_t length);
  bool SetTimeField(int ft, const unsigned char *value, size_t length);

  void GetField(const CItemField &field, unsigned char *value, size_t &length) const;
  void GetField(const CItemField &field, std::vector<unsigned char> &v) const;
  void GetField(const int ft, std::vector<unsigned char> &v) const;
  uint8_t GetFieldAsByte(const CItemField& field, uint8_t default_value = 0) const;
  uint8_t GetFieldAsByte(const int ft, uint8_t default_value = 0) const;
  StringX GetField(int ft) const;
  StringX GetField(const CItemField &field) const;

  void SetTime(int whichtime, time_t t);
  void GetTime(int whichtime, time_t &t) const;

  bool IsFieldSet(int ft) const {return m_fields.find(ft) != m_fields.end();}

  void GetUnknownField(unsigned char &type, size_t &length,
                       unsigned char * &pdata, const CItemField &item) const;
  bool IsItemDataField(unsigned char type) const
  {return type >= START && type < LAST_DATA;}
  bool IsItemAttField(unsigned char type) const
  {return type >= START_ATT && type < LAST_ATT;}

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
};

#endif /* __ITEM_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
