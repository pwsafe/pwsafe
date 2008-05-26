/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ItemData.h
//-----------------------------------------------------------------------------

#if !defined ItemData_h
#define ItemData_h

#include "Util.h"
#include "Match.h"
#include "ItemField.h"
#include "UUIDGen.h"
#include <time.h> // for time_t
#include <bitset>
#include <vector>
#include <string>


struct PWPolicy {
  WORD flags;
  int length;
  int digitminlength;
  int lowerminlength;
  int symbolminlength;
  int upperminlength;

  PWPolicy() : flags(0), length(0), 
    digitminlength(0), lowerminlength(0),
    symbolminlength(0), upperminlength(0) {}

  // copy c'tor and assignment operator, standard idioms
  PWPolicy(const PWPolicy &that)
    : flags(that.flags), length(that.length),
    digitminlength(that.digitminlength),
    lowerminlength(that.lowerminlength),
    symbolminlength(that.symbolminlength),
    upperminlength(that.upperminlength) {}

  PWPolicy &operator=(const PWPolicy &that)
  { if (this != &that) {
    flags = that.flags;
    length = that.length;
    digitminlength = that.digitminlength;
    lowerminlength = that.lowerminlength;
    symbolminlength = that.symbolminlength;
    upperminlength = that.upperminlength;
  }
  return *this;
  }
  void Empty()
  { flags = 0; length = 0;
  digitminlength = 0; lowerminlength = 0;
  symbolminlength = 0; upperminlength = 0;
  }
};

typedef std::vector<CItemField> UnknownFields;
typedef UnknownFields::const_iterator UnknownFieldsConstIter;

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

class BlowFish;

class CItemData
{
public:
  enum FieldType {
    START = 0x00, GROUPTITLE = 0x00 /* reusing depreciated NAME for Group.Title combination */,
    NAME = 0x00, UUID = 0x01, GROUP = 0x02, TITLE = 0x03, USER = 0x04, NOTES = 0x05,
    PASSWORD = 0x06, CTIME = 0x07, PMTIME = 0x08, ATIME = 0x09, XTIME = 0x0a,
    RESERVED = 0x0b /* cannot use */, RMTIME = 0x0c, URL = 0x0d, AUTOTYPE = 0x0e,
    PWHIST = 0x0f, POLICY = 0x10, XTIME_INT = 0x11,
    LAST,        // Start of unknown fields!
    END = 0xff}; // field types, per formatV{2,3}.txt

    // SubGroup Object - same as FieldType

    // status returns from "ProcessInputRecordField"
    enum {SUCCESS = 0, FAILURE, END_OF_FILE = 8};

    // entry type
    enum EntryType {ET_INVALID = -1,
      ET_NORMAL = 0, 
      ET_ALIASBASE = 1, ET_ALIAS = 2, 
      ET_SHORTCUTBASE = 4, ET_SHORTCUT = 8};

    // a bitset for indicating a subset of an item's fields: 
    typedef std::bitset<LAST> FieldBits;

    static void SetSessionKey(); // call exactly once per session

    static bool IsTextField(unsigned char t);

    //Construction
    CItemData();

    CItemData(const CItemData& stuffhere);

    ~CItemData();

    //Data retrieval
    CMyString GetName() const; // V17 - deprecated - replaced by GetTitle & GetUser
    CMyString GetTitle() const; // V20
    CMyString GetUser() const; // V20
    CMyString GetPassword() const;
    CMyString GetNotes(TCHAR delimiter = 0) const;
    void GetUUID(uuid_array_t &) const; // V20
    CMyString GetGroup() const; // V20
    CMyString GetURL() const; // V30
    CMyString GetAutoType() const; // V30
    CMyString GetATime() const {return GetTime(ATIME, TMC_ASC_UNKNOWN);}  // V30
    CMyString GetCTime() const {return GetTime(CTIME, TMC_ASC_UNKNOWN);}  // V30
    CMyString GetXTime() const {return GetTime(XTIME, TMC_ASC_UNKNOWN);}  // V30
    CMyString GetPMTime() const {return GetTime(PMTIME, TMC_ASC_UNKNOWN);}  // V30
    CMyString GetRMTime() const {return GetTime(RMTIME, TMC_ASC_UNKNOWN);}  // V30
    CMyString GetATimeL() const {return GetTime(ATIME, TMC_LOCALE);}  // V30
    CMyString GetCTimeL() const {return GetTime(CTIME, TMC_LOCALE);}  // V30
    CMyString GetXTimeL() const {return GetTime(XTIME, TMC_LOCALE);}  // V30
    CMyString GetPMTimeL() const {return GetTime(PMTIME, TMC_LOCALE);}  // V30
    CMyString GetRMTimeL() const {return GetTime(RMTIME, TMC_LOCALE);}  // V30
    CMyString GetATimeN() const {return GetTime(ATIME, TMC_ASC_NULL);}  // V30
    CMyString GetCTimeN() const {return GetTime(CTIME, TMC_ASC_NULL);}  // V30
    CMyString GetXTimeN() const {return GetTime(XTIME, TMC_ASC_NULL);}  // V30
    CMyString GetPMTimeN() const {return GetTime(PMTIME, TMC_ASC_NULL);}  // V30
    CMyString GetRMTimeN() const {return GetTime(RMTIME, TMC_ASC_NULL);}  // V30
    CMyString GetATimeExp() const {return GetTime(ATIME, TMC_EXPORT_IMPORT);}  // V30
    CMyString GetCTimeExp() const {return GetTime(CTIME, TMC_EXPORT_IMPORT);}  // V30
    CMyString GetXTimeExp() const {return GetTime(XTIME, TMC_EXPORT_IMPORT);}  // V30
    CMyString GetPMTimeExp() const {return GetTime(PMTIME, TMC_EXPORT_IMPORT);}  // V30
    CMyString GetRMTimeExp() const {return GetTime(RMTIME, TMC_EXPORT_IMPORT);}  // V30
    CMyString GetATimeXML() const {return GetTime(ATIME, TMC_XML);}  // V30
    CMyString GetCTimeXML() const {return GetTime(CTIME, TMC_XML);}  // V30
    CMyString GetXTimeXML() const {return GetTime(XTIME, TMC_XML);}  // V30
    CMyString GetPMTimeXML() const {return GetTime(PMTIME, TMC_XML);}  // V30
    CMyString GetRMTimeXML() const {return GetTime(RMTIME, TMC_XML);}  // V30
    //  These populate the time structure instead of giving a character string
    void GetATime(time_t &t) const {GetTime(ATIME, t);}  // V30
    void GetCTime(time_t &t) const {GetTime(CTIME, t);}  // V30
    void GetXTime(time_t &t) const {GetTime(XTIME, t);}  // V30
    void GetPMTime(time_t &t) const {GetTime(PMTIME, t);}  // V30
    void GetRMTime(time_t &t) const {GetTime(RMTIME, t);}  // V30
    void GetXTimeInt(int &xint) const; // V30
    CMyString GetXTimeInt() const; // V30
    CMyString GetPWHistory() const;  // V30
    void GetPWPolicy(PWPolicy &pwp) const;
    CMyString GetPWPolicy() const;
    // GetPlaintext returns all fields separated by separator, if delimiter is != 0, then
    // it's used for multi-line notes and to replace '.' within the Title field.
    CMyString GetPlaintext(const TCHAR &separator, const FieldBits &bsExport,
      const TCHAR &delimiter, const CItemData *cibase) const;
    std::string GetXML(unsigned id, const FieldBits &bsExport, TCHAR m_delimiter,
                       const CItemData *cibase) const;
    void GetUnknownField(unsigned char &type, unsigned int &length,
                         unsigned char * &pdata,
                         const unsigned int &num) const;
    void GetUnknownField(unsigned char &type, unsigned int &length,
                         unsigned char * &pdata,
                         const UnknownFieldsConstIter &iter) const;
    void SetUnknownField(const unsigned char type,
                         const unsigned int length,
                         const unsigned char * ufield);
    unsigned int NumberUnknownFields() const
    {return (unsigned int)m_URFL.size();}
    void ClearUnknownFields()
    {return m_URFL.clear();}
    UnknownFieldsConstIter GetURFIterBegin() const {return m_URFL.begin();}
    UnknownFieldsConstIter GetURFIterEnd() const {return m_URFL.end();}

    void CreateUUID(); // V20 - generate UUID for new item
    void SetName(const CMyString &name,
      const CMyString &defaultUsername); // V17 - deprecated - replaced by SetTitle & SetUser
    void SetTitle(const CMyString &title, TCHAR delimiter = 0);
    void SetUser(const CMyString &user); // V20
    void SetPassword(const CMyString &password);
    void SetNotes(const CMyString &notes, TCHAR delimiter = 0);
    void SetUUID(const uuid_array_t &UUID); // V20
    void SetGroup(const CMyString &group); // V20
    void SetURL(const CMyString &URL); // V30
    void SetAutoType(const CMyString &autotype); // V30
    void SetATime() {SetTime(ATIME);}  // V30
    void SetATime(time_t t) {SetTime(ATIME, t);}  // V30
    bool SetATime(const CString &time_str) {return SetTime(ATIME, time_str);}  // V30
    void SetCTime() {SetTime(CTIME);}  // V30
    void SetCTime(time_t t) {SetTime(CTIME, t);}  // V30
    bool SetCTime(const CString &time_str) {return SetTime(CTIME, time_str);}  // V30
    void SetXTime() {SetTime(XTIME);}  // V30
    void SetXTime(time_t t) {SetTime(XTIME, t);}  // V30
    bool SetXTime(const CString &time_str) {return SetTime(XTIME, time_str);}  // V30
    void SetPMTime() {SetTime(PMTIME);}  // V30
    void SetPMTime(time_t t) {SetTime(PMTIME, t);}  // V30
    bool SetPMTime(const CString &time_str) {return SetTime(PMTIME, time_str);}  // V30
    void SetRMTime() {SetTime(RMTIME);}  // V30
    void SetRMTime(time_t t) {SetTime(RMTIME, t);}  // V30
    bool SetRMTime(const CString &time_str) {return SetTime(RMTIME, time_str);}  // V30
    void SetXTimeInt(int &xint); // V30
    bool SetXTimeInt(const CString &xint_str); // V30
    void SetPWHistory(const CMyString &PWHistory);  // V30
    void SetPWPolicy(const PWPolicy &pwp);
    bool SetPWPolicy(const CString &cs_pwp);
    CItemData& operator=(const CItemData& second);
    // Following used by display methods - we just keep it handy
    void *GetDisplayInfo() const {return m_display_info;}
    void SetDisplayInfo(void *di) {m_display_info = di;}
    void Clear();
    // check record for mandatory fields, silently fix if missing
    int ValidateUUID(const unsigned short &nMajor, const unsigned short &nMinor,
      uuid_array_t &uuid_array);
    int ValidatePWHistory();

    // Predicate to determine if item matches given criteria
    bool Matches(const CString &string1, const int &iObject, 
                 const int &iFunction) const;  // string values
    bool Matches(const int &num1, const int &num2, const int &iObject,
                 const int &iFunction) const;  // intger values
    bool Matches(const time_t &time1, const time_t &time2, const int &iObject,
                 const int &iFunction) const;  // time values
    bool Matches(const EntryType &etype1, 
                 const int &iFunction) const;  // Entrytype values

    BOOL IsURLEmpty() const {return m_URL.IsEmpty();}
    void SerializePlainText(std::vector<char> &v, CItemData *cibase = NULL) const;
    bool DeserializePlainText(const std::vector<char> &v);
    bool SetField(int type, unsigned char *data, int len);

    EntryType GetEntryType() const
    {return m_entrytype;}

    bool IsNormal() const
    {return (m_entrytype == ET_NORMAL);}
    bool IsAliasBase() const
    {return (m_entrytype == ET_ALIASBASE);}
    bool IsShortcutBase() const
    {return (m_entrytype == ET_SHORTCUTBASE);}
    bool IsAlias() const
    {return (m_entrytype == ET_ALIAS);}
    bool IsShortcut() const
    {return (m_entrytype == ET_SHORTCUT);}

    void SetNormal()
    {m_entrytype = ET_NORMAL;}
    void SetAliasBase()
    {m_entrytype = ET_ALIASBASE;}
    void SetShortcutBase()
    {m_entrytype = ET_SHORTCUTBASE;}
    void SetAlias()
    {m_entrytype = ET_ALIAS;}
    void SetShortcut()
    {m_entrytype = ET_SHORTCUT;}

    bool IsURLEmail()
    {return GetURL().Find(_T("mailto:")) != -1;}

private:
  CItemField m_Name;
  CItemField m_Title;
  CItemField m_User;
  CItemField m_Password;
  CItemField m_Notes;
  CItemField m_UUID;
  CItemField m_Group;
  CItemField m_URL;
  CItemField m_AutoType;
  CItemField m_tttATime;  // last 'A'ccess time
  CItemField m_tttCTime;  // 'C'reation time
  CItemField m_tttXTime;  // password 'L'ifetime
  CItemField m_tttPMTime; // last 'P'assword 'M'odification time
  CItemField m_tttRMTime; // last 'R'ecord 'M'odification time
  CItemField m_PWHistory;
  CItemField m_PWPolicy;
  CItemField m_XTimeInterval;

  // Save unknown record fields on read to put back on write unchanged
  UnknownFields m_URFL;

  enum EntryType m_entrytype;

  // random key for storing stuff in memory, just to remove dependence
  // on passphrase
  static bool IsSessionKeySet;
  static unsigned char SessionKey[64];
  //The salt value
  unsigned char m_salt[SaltLength];
  // Following used by display methods - we just keep it handy
  void *m_display_info;

  // move from pre-2.0 name to post-2.0 title+user
  void SplitName(const CMyString &name,
    CMyString &title, CMyString &username);
  CMyString GetTime(int whichtime, int result_format) const; // V30
  void GetTime(int whichtime, time_t &t) const; // V30
  void SetTime(const int whichtime); // V30
  void SetTime(const int whichtime, time_t t); // V30
  bool SetTime(const int whichtime, const CString &time_str); // V30

  // Create local Encryption/Decryption object
  BlowFish *MakeBlowFish() const;
  // Laziness is a Virtue:
  CMyString GetField(const CItemField &field) const;
  void GetField(const CItemField &field, unsigned char *value,
    unsigned int &length) const;
  void GetUnknownField(unsigned char &type, unsigned int &length,
    unsigned char * &pdata, const CItemField &item) const;
  void SetField(CItemField &field, const CMyString &value);
  void SetField(CItemField &field, const unsigned char *value,
    unsigned int length);
};

inline bool CItemData::IsTextField(unsigned char t)
{
  return !(t == UUID || t == CTIME || t == PMTIME ||
    t == ATIME || t == XTIME || t == RMTIME || t == XTIME_INT ||
    t >= LAST);
}
#endif
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:

