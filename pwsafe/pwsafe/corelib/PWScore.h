// PWScore.h
//-----------------------------------------------------------------------------

#ifndef PWScore_h
#define PWScore_h
#include <afxtempl.h> // for CList
#include "ItemData.h"
#include "MyString.h"
#include "PWSfile.h"

class PWScore {
 public:

  enum {
    CANT_OPEN_FILE = -10,
    USER_CANCEL,
    WRONG_PASSWORD,
    NOT_SUCCESS,
    ALREADY_OPEN,
    SUCCESS = 0,
   };

  PWScore();
  ~PWScore();

  // Following used to read/write databases
  CMyString GetCurFile() const {return m_currfile;}
  void SetCurFile(const CMyString &file) {m_currfile = file;}
  bool GetUseDefUser() const {return m_usedefuser;}
  void SetUseDefUser(bool v) {m_usedefuser = v;}
  CMyString GetDefUsername() const {return m_defusername;}
  void SetDefUsername(const CMyString &du) {m_defusername = du;}

  void ClearData();
  void NewFile(const CMyString &passkey);
  int WriteCurFile() {return WriteFile(m_currfile);}
  int WriteFile(const CMyString &filename);
  bool FileExists(const CMyString &filename) {return PWSfile::FileExists(filename);}
  int ReadCurFile(const CMyString &passkey)
    {return ReadFile(m_currfile, passkey);}
  int ReadFile(const CMyString &filename, const CMyString &passkey);
  int CheckPassword(const CMyString &filename, CMyString &passkey);
  void ChangePassword(const CMyString & newPassword);

  POSITION GetFirstEntryPosition() const
    {return m_pwlist.GetHeadPosition();}
  POSITION AddEntryToTail(const CItemData &item)
    {m_changed = true; return m_pwlist.AddTail(item);}
  CItemData GetEntryAt(POSITION pos) const
    {return m_pwlist.GetAt(pos);}
  CItemData &GetEntryAt(POSITION pos)
    {return m_pwlist.GetAt(pos);}
  CItemData GetNextEntry(POSITION &pos) const
    {return m_pwlist.GetNext(pos);}
  CItemData &GetNextEntry(POSITION &pos)
    {return m_pwlist.GetNext(pos);}
  int GetNumEntries() const {return m_pwlist.GetCount();}
  void RemoveEntryAt(POSITION pos)
    {m_changed = true; m_pwlist.RemoveAt(pos);}
 // Find in m_pwlist by title and user name, exact match
  POSITION Find(const CMyString &a_title, const CMyString &a_user);

  bool IsChanged() const {return m_changed;}
  void SetChanged(bool changed) {m_changed = changed;} // use sparingly...

  // Following moved from Util.{h,cpp} and constified
 public:
  void MakeName(CMyString&, const CMyString &, const CMyString &) const; // used also by AddDlg, hence public

  void MakeFullNames(const CMyString &defusername);
  void DropDefUsernames(const CMyString &defusername);
  void SetBlankToDef();
  void SetBlankToName(const CMyString &username);

  int SplitName(const CMyString &name,
		CMyString &title, CMyString &username);

 private:
  CMyString m_currfile; // current pw db filespec
  bool m_usedefuser;
  CMyString m_defusername;

  // the password database
  CList<CItemData,CItemData> m_pwlist;
  bool m_changed;
};

#endif // PWScore_h
