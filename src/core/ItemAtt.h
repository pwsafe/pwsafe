/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// ItemAtt.h
//-----------------------------------------------------------------------------

#ifndef __ITEMATT_H
#define __ITEMATT_H

#include "Util.h"
#include "Match.h"
#include "Item.h"
#include "os/UUID.h"
#include "StringX.h"

#include <time.h> // for time_t

//-----------------------------------------------------------------------------

/*
* CItemAtt is a class that contains an attachment associated with a password entry
*
* What makes this class interesting is that all fields are kept encrypted
* from the moment of construction, and are decrypted by the appropriate
* accessor (Get* member function).
*
* All this is to protect the data in memory, and has nothing to do with
* how the records are written to disk.
*/

class BlowFish;
class PWSfile;

class CItemAtt : public CItem
{
public:
  // a bitset for indicating a subset of an item's fields: 
  typedef std::bitset<LAST_SEARCHABLE - START + 1> AttFieldBits;

  //Construction
  CItemAtt();
  CItemAtt(const CItemAtt& stuffhere);

  ~CItemAtt();

  int Read(PWSfile *in);
  int Write(PWSfile *out) const;

  int Import(const stringT &fname);
  int Export(const stringT &fname) const;

  bool HasContent() const {return IsFieldSet(CONTENT);}

  // Convenience: Get the name associated with FieldType
  static stringT FieldName(FieldType ft);
  // Convenience: Get the untranslated (English) name of a FieldType
  static stringT EngFieldName(FieldType ft);

  // Setters and Getters
  void CreateUUID(); // for new
  void SetUUID(const pws_os::CUUID &uuid);
  void SetTitle(const StringX &title);
  void SetCTime(time_t t);
  void SetContent(const unsigned char *content, size_t clen);

  StringX GetTitle() const {return GetField(ATTTITLE);}
  void GetUUID(uuid_array_t &) const;
  const pws_os::CUUID GetUUID() const;
  StringX GetFileName() const {return GetField(FILENAME);}    // set via Import()
  StringX GetFilePath() const { return GetField(FILEPATH); }  // set via Import()
  StringX GetMediaType() const {return GetField(MEDIATYPE);}  // set via Import()
  size_t GetContentLength() const; // Number of bytes stored
  size_t GetContentSize() const; // size needed for GetContent (!= len due to block cipher)
  bool GetContent(unsigned char *content, size_t csize) const;

  time_t GetCTime(time_t &t) const;

  StringX GetTime(int whichtime, PWSUtil::TMC result_format) const;
  void SetTime(const int whichtime); // V30
  bool SetTime(const int whichtime, const stringT &time_str); // V30

  StringX GetFileCTime() const { return GetTime(FILECTIME, PWSUtil::TMC_LOCALE); }
  StringX GetFileMTime() const { return GetTime(FILEMTIME, PWSUtil::TMC_LOCALE); }
  StringX GetFileATime() const { return GetTime(FILEATIME, PWSUtil::TMC_LOCALE); }

  //  These populate (and return) time_t instead of giving a character string
  time_t GetFileCTime(time_t &t) const { CItem::GetTime(FILECTIME, t); return t; }
  time_t GetFileMTime(time_t &t) const { CItem::GetTime(FILEMTIME, t); return t; }
  time_t GetFileATime(time_t &t) const { CItem::GetTime(FILEATIME, t); return t; }

  void SetFileCTime() { SetTime(FILECTIME); }
  void SetFileCTime(time_t t) { CItem::SetTime(FILECTIME, t); }
  bool SetFileCTime(const stringT &time_str) { return SetTime(FILECTIME, time_str); }
  void SetFileMTime() { SetTime(FILEMTIME); }
  void SetFileMTime(time_t t) { CItem::SetTime(FILEMTIME, t); }
  bool SetFileMTime(const stringT &time_str) { return SetTime(FILEMTIME, time_str); }
  void SetFileATime() { SetTime(FILEATIME); }
  void SetFileATime(time_t t) { CItem::SetTime(FILEATIME, t); }
  bool SetFileATime(const stringT &time_str) { return SetTime(FILEATIME, time_str); }

  EntryStatus GetStatus() const {return m_entrystatus;}
  void ClearStatus() {m_entrystatus = ES_CLEAN;}
  void SetStatus(const EntryStatus es) {m_entrystatus = es;}

  long GetOffset() const {return m_offset;}
  void SetOffset(long offset) {m_offset = offset;}
  unsigned GetRefcount() const {return m_refcount;}
  void IncRefcount() {m_refcount++;}
  void DecRefcount() {ASSERT(m_refcount > 0); m_refcount--;}

  CItemAtt& operator=(const CItemAtt& second);

  bool operator==(const CItemAtt &that) const;
  bool operator!=(const CItemAtt &that) const {return !operator==(that);}

  // Predicate to determine if item matches given criteria
  bool Matches(const stringT &stValue, int iObject,
    int iFunction) const;  // string values
  bool Matches(time_t time1, time_t time2, int iObject,
    int iFunction) const;  // time values

  bool HasUUID() const                     { return IsFieldSet(ATTUUID);   }
  bool IsTitleSet() const                  { return IsFieldSet(ATTTITLE);     }
  bool IsCreationTimeSet() const           { return IsFieldSet(ATTCTIME);     }

private:
  bool SetField(unsigned char type, const unsigned char *data, size_t len);
  size_t WriteIfSet(FieldType ft, PWSfile *out, bool isUTF8) const;

  EntryStatus m_entrystatus;
  long m_offset; // location on file, for lazy evaluation
  unsigned m_refcount; // how many CItemData objects refer to this?
};
#endif /* __ITEMATT_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
