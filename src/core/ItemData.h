/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ItemData.h
//-----------------------------------------------------------------------------

#ifndef __ITEMDATA_H
#define __ITEMDATA_H

#include "Util.h"
#include "Match.h"
#include "Item.h"
#include "PWSprefs.h"
#include "PWPolicy.h"
#include "os/UUID.h"
#include "StringX.h"

#include <time.h> // for time_t
#include <bitset>
#include <vector>
#include <string>
#include <map>

//-----------------------------------------------------------------------------

/*
* CItemData is a class that contains the data present in a password entry
*
* 'Name' is the pre-2.x field, that had both the entry title and the
* username rolled-in together, separated by SPLTCHR (defined in util.h).
* In 2.0 and later, this field is unused, and the title and username
* are stored in separate fields.
*
* What makes this class interesting is that all fields are kept encrypted
* from the moment of construction, and are decrypted by the appropriate
* accessor (Get* member function).
*
* All this is to protect the data in memory, and has nothing to do with
* how the records are written to disk.
*/

class PWSfile;
class PWSfileV4;

class CItemData : public CItem
{
public:

  // Password Policy stuff: Either PWPolicy (+ optionally symbols) is not empty
  // or PolicyName is not empty. Both cannot be set. All can be empty.

  // SubGroup Object - same as FieldType

  // Entry type (note: powers of 2)
  enum EntryType {ET_INVALID      = -1,
                  ET_NORMAL       =  0, 
                  ET_ALIASBASE    =  1, ET_ALIAS    = 2, 
                  ET_SHORTCUTBASE =  4, ET_SHORTCUT = 8,
                  ET_LAST
  };

  // a bitset for indicating a subset of an item's fields: 
  typedef std::bitset<LAST_DATA> FieldBits;

  static bool IsTextField(unsigned char t);

  //Construction
  CItemData();
  CItemData(const CItemData& stuffhere);

  ~CItemData();

  int Read(PWSfile *in);
  int Write(PWSfile *out) const;
  int Write(PWSfileV4 *out) const;
  int WriteCommon(PWSfile *out) const;

  // Convenience: Get the name associated with FieldType
  static stringT FieldName(FieldType ft);
  // Convenience: Get the untranslated (English) name of a FieldType
  static stringT EngFieldName(FieldType ft);

  //Data retrieval
  StringX GetName() const {return GetField(NAME);} // V17 - deprecated: replaced by GetTitle & GetUser
  StringX GetTitle() const {return GetField(TITLE);} // V20
  StringX GetUser() const  {return GetField(USER);}  // V20
  StringX GetPassword() const {return GetField(PASSWORD);}
  size_t GetPasswordLength() const {return GetField(PASSWORD).length();}
  StringX GetNotes(TCHAR delimiter = 0) const;
  void GetUUID(uuid_array_t &, FieldType ft = END) const; // V20
  const pws_os::CUUID GetUUID(FieldType ft = END) const; // V20 - see comment in .cpp re return type
  const pws_os::CUUID GetBaseUUID() const {return GetUUID(CItemData::BASEUUID);}
  const pws_os::CUUID GetAttUUID() const {return GetUUID(CItemData::ATTREF);}
  StringX GetGroup() const {return GetField(GROUP);} // V20
  StringX GetURL() const {return GetField(URL);} // V30
  StringX GetAutoType() const {return GetField(AUTOTYPE);} // V30
  StringX GetATime() const {return GetTime(ATIME, PWSUtil::TMC_ASC_UNKNOWN);}  // V30
  StringX GetCTime() const {return GetTime(CTIME, PWSUtil::TMC_ASC_UNKNOWN);}  // V30
  StringX GetXTime() const {return GetTime(XTIME, PWSUtil::TMC_ASC_UNKNOWN);}  // V30
  StringX GetPMTime() const {return GetTime(PMTIME, PWSUtil::TMC_ASC_UNKNOWN);}  // V30
  StringX GetRMTime() const {return GetTime(RMTIME, PWSUtil::TMC_ASC_UNKNOWN);}  // V30
  StringX GetATimeL() const {return GetTime(ATIME, PWSUtil::TMC_LOCALE);}  // V30
  StringX GetCTimeL() const {return GetTime(CTIME, PWSUtil::TMC_LOCALE);}  // V30
  StringX GetXTimeL() const {return GetTime(XTIME, PWSUtil::TMC_LOCALE_DATE_ONLY);}  // V30
  StringX GetPMTimeL() const {return GetTime(PMTIME, PWSUtil::TMC_LOCALE);}  // V30
  StringX GetRMTimeL() const {return GetTime(RMTIME, PWSUtil::TMC_LOCALE);}  // V30
  StringX GetATimeN() const {return GetTime(ATIME, PWSUtil::TMC_ASC_NULL);}  // V30
  StringX GetCTimeN() const {return GetTime(CTIME, PWSUtil::TMC_ASC_NULL);}  // V30
  StringX GetXTimeN() const {return GetTime(XTIME, PWSUtil::TMC_ASC_NULL);}  // V30
  StringX GetPMTimeN() const {return GetTime(PMTIME, PWSUtil::TMC_ASC_NULL);}  // V30
  StringX GetRMTimeN() const {return GetTime(RMTIME, PWSUtil::TMC_ASC_NULL);}  // V30
  StringX GetATimeExp() const {return GetTime(ATIME, PWSUtil::TMC_EXPORT_IMPORT);}  // V30
  StringX GetCTimeExp() const {return GetTime(CTIME, PWSUtil::TMC_EXPORT_IMPORT);}  // V30
  StringX GetXTimeExp() const {return GetTime(XTIME, PWSUtil::TMC_EXPORT_IMPORT);}  // V30
  StringX GetPMTimeExp() const {return GetTime(PMTIME, PWSUtil::TMC_EXPORT_IMPORT);}  // V30
  StringX GetRMTimeExp() const {return GetTime(RMTIME, PWSUtil::TMC_EXPORT_IMPORT);}  // V30
  StringX GetATimeXML() const {return GetTime(ATIME, PWSUtil::TMC_XML);}  // V30
  StringX GetCTimeXML() const {return GetTime(CTIME, PWSUtil::TMC_XML);}  // V30
  StringX GetXTimeXML() const {return GetTime(XTIME, PWSUtil::TMC_XML);}  // V30
  StringX GetPMTimeXML() const {return GetTime(PMTIME, PWSUtil::TMC_XML);}  // V30
  StringX GetRMTimeXML() const {return GetTime(RMTIME, PWSUtil::TMC_XML);}  // V30
  //  These populate (and return) time_t instead of giving a character string
  time_t GetATime(time_t &t) const {CItem::GetTime(ATIME, t); return t;}  // V30
  time_t GetCTime(time_t &t) const {CItem::GetTime(CTIME, t); return t;}  // V30
  time_t GetXTime(time_t &t) const {CItem::GetTime(XTIME, t); return t;}  // V30
  time_t GetPMTime(time_t &t) const {CItem::GetTime(PMTIME, t); return t;}  // V30
  time_t GetRMTime(time_t &t) const {CItem::GetTime(RMTIME, t); return t;}  // V30
  int32 GetXTimeInt(int32 &xint) const; // V30
  StringX GetXTimeInt() const; // V30
  StringX GetPWHistory() const;  // V30
  StringX GetPreviousPassword() const;
  void GetPWPolicy(PWPolicy &pwp) const;
  StringX GetPWPolicy() const {return GetField(POLICY);}
  StringX GetRunCommand() const {return GetField(RUNCMD);}
  int16 GetDCA(int16 &iDCA, bool bShift = false) const;
  StringX GetDCA(bool bShift = false) const;
  int16 GetShiftDCA(int16 &iDCA) const {return GetDCA(iDCA, true);}
  StringX GetShiftDCA() const {return GetDCA(true);}
  StringX GetEmail() const {return GetField(EMAIL);}
  StringX GetProtected() const;
  void GetProtected(unsigned char &ucprotected) const;
  bool IsProtected() const;
  StringX GetSymbols() const    {return GetField(SYMBOLS);}
  StringX GetPolicyName() const {return GetField(POLICYNAME);}
  int32 GetKBShortcut(int32 &iKBShortcut) const;
  StringX GetKBShortcut() const;

  StringX GetFieldValue(FieldType ft) const;

  // Following encapsulates difference between Alias and Shortcut w.r.t. field 'ownership':
  StringX GetEffectiveFieldValue(FieldType ft, const CItemData *pbci) const;

  // GetPlaintext returns all fields separated by separator, if delimiter is != 0, then
  // it's used for multi-line notes and to replace '.' within the Title field.
  StringX GetPlaintext(const TCHAR &separator, const FieldBits &bsExport,
                       const TCHAR &delimiter, const CItemData *pcibase) const;
  std::string GetXML(unsigned id, const FieldBits &bsExport, TCHAR m_delimiter,
                     const CItemData *pcibase, bool bforce_normal_entry,
                     bool &bXMLErrorsFound) const;

  void CreateUUID(FieldType ft = CItemData::END); // V20 - generate UUID for new item
  void SetName(const StringX &name,
               const StringX &defaultUsername); // V17 - deprecated - replaced by SetTitle & SetUser
  void SetTitle(const StringX &title, TCHAR delimiter = 0);
  void SetUser(const StringX &user) {CItem::SetField(USER, user);} // V20
  void SetPassword(const StringX &password) {CItem::SetField(PASSWORD, password);}
  void UpdatePassword(const StringX &password); // use when password changed!
  void SetNotes(const StringX &notes, TCHAR delimiter = 0);
  void SetUUID(const pws_os::CUUID &uuid, FieldType ft = CItemData::UUID);
  void SetBaseUUID(const pws_os::CUUID &uuid) {SetUUID(uuid, CItemData::BASEUUID);}
  void SetAttUUID(const pws_os::CUUID &uuid) {SetUUID(uuid, CItemData::ATTREF);}
  void ClearAttUUID() {ClearField(CItemData::ATTREF);}
  void SetGroup(const StringX &group) {CItem::SetField(GROUP, group);} // V20
  void SetURL(const StringX &url) {CItem::SetField(URL, url);} // V30
  void SetAutoType(const StringX &autotype) {CItem::SetField(AUTOTYPE, autotype);} // V30
  void SetATime() {SetTime(ATIME);}  // V30
  void SetATime(time_t t) {CItem::SetTime(ATIME, t);}  // V30
  bool SetATime(const stringT &time_str) {return SetTime(ATIME, time_str);}  // V30
  void SetCTime() {SetTime(CTIME);}  // V30
  void SetCTime(time_t t) {CItem::SetTime(CTIME, t);}  // V30
  bool SetCTime(const stringT &time_str) {return SetTime(CTIME, time_str);}  // V30
  void SetXTime() {SetTime(XTIME);}  // V30
  void SetXTime(time_t t) {CItem::SetTime(XTIME, t);}  // V30
  bool SetXTime(const stringT &time_str) {return SetTime(XTIME, time_str);}  // V30
  void SetPMTime() {SetTime(PMTIME);}  // V30
  void SetPMTime(time_t t) {CItem::SetTime(PMTIME, t);}  // V30
  bool SetPMTime(const stringT &time_str) {return SetTime(PMTIME, time_str);}  // V30
  void SetRMTime() {SetTime(RMTIME);}  // V30
  void SetRMTime(time_t t) {CItem::SetTime(RMTIME, t);}  // V30
  bool SetRMTime(const stringT &time_str) {return SetTime(RMTIME, time_str);}  // V30
  void SetXTimeInt(int32 xint); // V30
  bool SetXTimeInt(const stringT &xint_str); // V30
  void SetPWHistory(const StringX &PWHistory);  // V30
  void SetPWPolicy(const PWPolicy &pwp);
  bool SetPWPolicy(const stringT &cs_pwp);
  void SetRunCommand(const StringX &sx_RunCommand) {CItem::SetField(RUNCMD, sx_RunCommand);}
  void SetDCA(int16 iDCA, bool bShift = false);
  bool SetDCA(const stringT &cs_DCA, bool bShift = false);
  void SetShiftDCA(int16 iDCA) {SetDCA(iDCA, true);}
  bool SetShiftDCA(const stringT &cs_DCA) {return SetDCA(cs_DCA, true);}
  void SetEmail(const StringX &sx_email) {CItem::SetField(EMAIL, sx_email);}
  void SetProtected(bool bOnOff);
  void SetSymbols(const StringX &sx_symbols) {CItem::SetField(SYMBOLS, sx_symbols);}
  void SetPolicyName(const StringX &sx_PolicyName) {CItem::SetField(POLICYNAME,
                                                                    sx_PolicyName);}
  void SetKBShortcut(const StringX &sx_KBShortcut);
  void SetKBShortcut(int32 iKBShortcut);

  void SetFieldValue(FieldType ft, const StringX &value);

  CItemData& operator=(const CItemData& second);

  void Clear() override;

  bool operator==(const CItemData &that) const;
  bool operator!=(const CItemData &that) const {return !operator==(that);}

  // Check record for correct password history
  bool ValidatePWHistory(); // return true if OK, false if there's a problem

  bool IsExpired() const;
  bool WillExpire(const int numdays) const;

  // Predicate to determine if item matches given criteria
  bool Matches(const stringT &stValue, int iObject, 
               int iFunction) const;  // string values
  bool Matches(int num1, int num2, int iObject,
               int iFunction) const;  // integer values
  bool MatchesTime(time_t time1, time_t time2, int iObject,
               int iFunction) const;  // time values
  bool Matches(int16 dca, int iFunction, bool bShift = false) const;  // DCA values
  bool Matches(EntryType etype, int iFunction) const;  // Entrytype values
  bool Matches(EntryStatus estatus, int iFunction) const;  // Entrystatus values

  bool HasUUID() const; // UUID type matches entry type and is set
  bool IsGroupSet() const                  { return IsFieldSet(GROUP);     }
  bool IsUserSet() const                   { return IsFieldSet(USER);      }
  bool IsNotesSet() const                  { return IsFieldSet(NOTES);     }
  bool IsURLSet() const                    { return IsFieldSet(URL);       }
  bool IsRunCommandSet() const             { return IsFieldSet(RUNCMD);    }
  bool IsEmailSet() const                  { return IsFieldSet(EMAIL);     }
  bool IsTitleSet() const                  { return IsFieldSet(TITLE);     }
  bool IsPasswordSet() const               { return IsFieldSet(PASSWORD);  }
  bool IsCreationTimeSet() const           { return IsFieldSet(CTIME);     }
  bool IsModificationTimeSet() const       { return IsFieldSet(PMTIME);    }
  bool IsLastAccessTimeSet() const         { return IsFieldSet(ATIME);     }
  bool IsExpiryDateSet() const             { return IsFieldSet(XTIME);     }
  bool IsRecordModificationTimeSet() const { return IsFieldSet(RMTIME);    }
  bool IsAutoTypeSet() const               { return IsFieldSet(AUTOTYPE);  }
  bool IsPasswordHistorySet() const        { return IsFieldSet(PWHIST);    }
  bool IsPasswordPolicySet() const         { return IsFieldSet(POLICY);    }
  bool IsPasswordExpiryIntervalSet() const { return IsFieldSet(XTIME_INT); }
  bool IsDCASet() const                    { return IsFieldSet(DCA);       }
  bool IsShiftDCASet() const               { return IsFieldSet(SHIFTDCA);  }
  bool IsProtectionSet() const             { return IsFieldSet(PROTECTED); }
  bool IsSymbolsSet() const                { return IsFieldSet(SYMBOLS);   }
  bool IsPolicyNameSet() const             { return IsFieldSet(POLICYNAME);}
  bool IsKBShortcutSet() const             { return IsFieldSet(KBSHORTCUT);}
    
  bool IsGroupEmpty() const                { return !IsGroupSet();         }
  bool IsUserEmpty() const                 { return !IsUserSet();          }
  bool IsNotesEmpty() const                { return !IsNotesSet();         }
  bool IsURLEmpty() const                  { return !IsURLSet();           }
  bool IsRunCommandEmpty() const           { return !IsRunCommandSet();    }
  bool IsEmailEmpty() const                { return !IsEmailSet();         }
  bool IsPolicyEmpty() const               { return !IsPasswordPolicySet();}

  bool IsFieldValueEmpty(FieldType ft, const CItemData *pbci) const
  { return GetEffectiveFieldValue(ft, pbci).empty(); }

  bool HasAttRef() const                   { return IsFieldSet(ATTREF);    }

  void SerializePlainText(std::vector<char> &v,
                          const CItemData *pcibase = nullptr) const;
  bool DeSerializePlainText(const std::vector<char> &v);

  EntryType GetEntryType() const {return m_entrytype;}

  bool IsNormal() const {return (m_entrytype == ET_NORMAL);}
  bool IsAliasBase() const {return (m_entrytype == ET_ALIASBASE);}
  bool IsShortcutBase() const {return (m_entrytype == ET_SHORTCUTBASE);}
  bool IsAlias() const {return (m_entrytype == ET_ALIAS);}
  bool IsShortcut() const {return (m_entrytype == ET_SHORTCUT);}
  bool IsBase() const {return IsAliasBase() || IsShortcutBase();}
  bool IsDependent() const {return IsAlias() || IsShortcut();}

  void SetEntryType(EntryType et);
  void SetNormal() {SetEntryType(ET_NORMAL);}
  void SetAliasBase() {SetEntryType(ET_ALIASBASE);}
  void SetShortcutBase() {SetEntryType(ET_SHORTCUTBASE);}
  void SetAlias() {SetEntryType(ET_ALIAS);}
  void SetShortcut() {SetEntryType(ET_SHORTCUT);}

  void SetDuplicateTimes(const CItemData &src);

  EntryStatus GetStatus() const {return m_entrystatus;}
  void ClearStatus() {m_entrystatus = ES_CLEAN;}
  void SetStatus(const EntryStatus es) {m_entrystatus = es;}

  bool IsURLEmail() const
  {return GetURL().find(_T("mailto:")) != StringX::npos;}

  bool IsURLEmail(const CItemData *pbci) const
  { return GetEffectiveFieldValue(URL, pbci).find(_T("mailto:")) != StringX::npos; }

private:
  EntryType m_entrytype;
  EntryStatus m_entrystatus;

  // move from pre-2.0 name to post-2.0 title+user
  void SplitName(const StringX &name,
                 StringX &title, StringX &username);
  StringX GetTime(int whichtime, PWSUtil::TMC result_format) const; // V30
  void SetTime(const int whichtime); // V30
  bool SetTime(const int whichtime, const stringT &time_str); // V30

  // Laziness is a Virtue:
  bool SetField(unsigned char type, const unsigned char *data, size_t len);

  // for V3 Alias or Shortcut, the base UUID is encoded in password
  void ParseSpecialPasswords();
  void SetSpecialPasswords();

  void UpdatePasswordHistory(); // used by UpdatePassword()

  int WriteUnknowns(PWSfile *out) const;
  size_t WriteIfSet(FieldType ft, PWSfile *out, bool isUTF8) const;
};

inline bool CItemData::IsTextField(unsigned char t)
{
  return !(
    t == UUID       ||
    t == CTIME      || t == PMTIME || t == ATIME    || t == XTIME     || t == RMTIME ||
    t == XTIME_INT  ||
    t == RESERVED   || t == DCA    || t == SHIFTDCA || t == PROTECTED ||
    t == KBSHORTCUT || t == ATTREF || t == BASEUUID || t == ALIASUUID ||
    t == SHORTCUTUUID ||
    t >= LAST_DATA);
}
#endif /* __ITEMDATA_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
