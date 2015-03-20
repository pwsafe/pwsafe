/*
* Copyright (c) 2003-2015 Rony Shapiro <ronys@users.sourceforge.net>.
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
  typedef unsigned char key256T[32];
  typedef unsigned char contentHMACT[32];

  enum FieldType {
    ATTUUID = 0x61,
    TITLE = 0x03,
    CTIME = 0x07,
    MEDIATYPE = 0x62,
    FILENAME = 0x63,
    ATTEK = 0x64,
    ATTAK = 0x65,
    ATTIV = 0x66,
    CONTENT = 0x67,
    CONTENTHMAC = 0x68,
    END = 0xff
};

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
  void SetEK(const key256T &key);
  void SetAK(const key256T &key);
  void SetIV(const unsigned char *IV, unsigned int blocksize);
  void SetHMAC(const contentHMACT &hm);
  void SetContent(const unsigned char *content, size_t clen);

  StringX GetTitle() const {return GetField(TITLE);}
  void GetUUID(uuid_array_t &) const;
  const pws_os::CUUID GetUUID() const;
  StringX GetFileName() const {return GetField(FILENAME);}
  time_t GetCTime(time_t &t) const;
  void GetEK(key256T &key) const {return GetKey(ATTEK, key);}
  void GetAK(key256T &key) const {return GetKey(ATTAK, key);}
  void GetIV(unsigned char *IV, unsigned int &blocksize) const;
  void GetHMAC(contentHMACT &hm) const;
  size_t GetContentLength() const; // Number of bytes stored
  size_t GetContentSize() const; // size needed for GetContent (!= len due to block cipher)
  bool GetContent(unsigned char *content, size_t csize) const;


  CItemAtt& operator=(const CItemAtt& second);

  bool operator==(const CItemAtt &that) const;
  bool operator!=(const CItemAtt &that) const {return !operator==(that);}


  bool HasUUID() const                     { return IsFieldSet(ATTUUID);   }
  bool IsTitleSet() const                  { return IsFieldSet(TITLE);     }
  bool IsCreationTimeSet() const           { return IsFieldSet(CTIME);     }


  EntryStatus GetStatus() const {return m_entrystatus;}
  void ClearStatus() {m_entrystatus = ES_CLEAN;}
  void SetStatus(const EntryStatus es) {m_entrystatus = es;}


private:
  void GetKey(FieldType ft, key256T &key) const;
  bool SetField(unsigned char type, const unsigned char *data, size_t len);
  size_t WriteIfSet(FieldType ft, PWSfile *out, bool isUTF8) const;

  EntryStatus m_entrystatus;
  long m_offset; // location on file, for lazy evaluation
};

#endif /* __ITEMATT_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
