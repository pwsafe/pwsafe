/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// attachments.h
//-----------------------------------------------------------------------------

#ifndef __ATTACHMENTS_H
#define __ATTACHMENTS_H

#include "UUIDGen.h"
#include "StringX.h"
#include "sha1.h"

#include <time.h> // for time_t
#include <vector>

extern void trashMemory(void* buffer, size_t length);

/*
Name                         Value        Type
-----------------------------------------------------
UUID of this entry           0x01        UUID
UUID of associated entry     0x02        UUID
Flags                        0x03        2 characters
Attachment filename.ext      0x04        Text
Attachment original path     0x05        Text
Attachment description       0x06        Text
Attachment original size     0x07        4 bytes
Blocksize used to read data  0x08        4 bytes
Attachment compressed size0  0x09        4 bytes
Attachment Create Time       0x0a        time_t
Attachment Last Access Time  0x0b        time_t
Attachment Modifed Time      0x0c        time_t

Data - Multiple data records 0x80        Text
Data - Last record           0x81        Text

Attachment compressed size1  0xc0        4 bytes
Attachment CRC               0xc1        4 bytes
Attachment SHA1 odigest      0xc2        20 characters
Attachment SHA1 cdigest      0xc3        20 characters
Attachment Date added        0xc4        time_t

End of Entry                 0xff        [empty]
*/

// flags - Note: ATT_ERASEONDBCLOSE not yet implemented
#define ATT_EXTRACTTOREMOVEABLE 0x80
#define ATT_ERASEPGMEXISTS      0x40
#define ATT_ERASEONDBCLOSE      0x20
// Unused                       0x1f

// uiflags - Internal indicators (not in record in file)
#define ATT_ATTACHMENT_FLGCHGD  0x80
#define ATT_ATTACHMENT_DELETED  0x40
// Unused                       0x3f

// Max file size (memory allocation restrictions) - 32MB
#define ATT_MAXSIZE             32

// Function calls to GetAttachment
enum {OPENFILE, GETPRE, GETDATA, GETPOST, CLOSEFILE};

// Values for matching criteria for Export to XML
enum {ATTGROUP, ATTTITLE, ATTGROUPTITLE, ATTUSER, ATTPATH, ATTFILENAME, ATTDESCRIPTION};

// Attachment Record

struct ATRecord {
  ATRecord()
  : uncsize(0), cmpsize(0), blksize(0), CRC(0), flags(0), uiflags(0),
    ctime(0), atime(0), mtime(0), dtime(0),
    filename(_T("")), path(_T("")), description(_T(""))
  {
    memset(entry_uuid, 0, sizeof(uuid_array_t));
    memset(attmt_uuid, 0, sizeof(uuid_array_t));
    memset(odigest, 0, SHA1::HASHLEN);
    memset(cdigest, 0, SHA1::HASHLEN);
  }

  ~ATRecord()
  {
  }

  ATRecord(const ATRecord &atr)
    : uncsize(atr.uncsize), cmpsize(atr.cmpsize), blksize(atr.blksize), CRC(atr.CRC),
    flags(atr.flags), uiflags(atr.uiflags),
    ctime(atr.ctime), atime(atr.atime), mtime(atr.mtime), dtime(atr.dtime),
    filename(atr.filename), path(atr.path), description(atr.description)
  {
    memcpy(entry_uuid, atr.entry_uuid, sizeof(uuid_array_t));
    memcpy(attmt_uuid, atr.attmt_uuid, sizeof(uuid_array_t));
    memcpy(odigest, atr.odigest, SHA1::HASHLEN);
    memcpy(cdigest, atr.cdigest, SHA1::HASHLEN);
  }

  ATRecord &operator=(const ATRecord &atr)
  {
    if (this != &atr) {
      uncsize = atr.uncsize;
      cmpsize = atr.cmpsize;
      blksize = atr.blksize;

      CRC = atr.CRC;
      flags = atr.flags;
      uiflags = atr.uiflags;
      ctime = atr.ctime;
      atime = atr.atime;
      mtime = atr.mtime;
      dtime = atr.dtime;
      filename = atr.filename;
      path = atr.path;
      description = atr.description;
      memcpy(entry_uuid, atr.entry_uuid, sizeof(uuid_array_t));
      memcpy(attmt_uuid, atr.attmt_uuid, sizeof(uuid_array_t));
      memcpy(odigest, atr.odigest, SHA1::HASHLEN);
      memcpy(cdigest, atr.cdigest, SHA1::HASHLEN);
    }
    return *this;
  }

  void Clear() {
    uncsize = cmpsize = blksize = 0;
    flags = uiflags = 0;
    CRC = 0;
    ctime = atime = mtime = dtime = 0;
    filename = path = description = _T("");
    memset(entry_uuid, 0, sizeof(uuid_array_t));
    memset(attmt_uuid, 0, sizeof(uuid_array_t));
    memset(odigest, 0, SHA1::HASHLEN);
    memset(cdigest, 0, SHA1::HASHLEN);
  }

  unsigned int uncsize;
  unsigned int cmpsize;
  unsigned int blksize;
  unsigned int CRC;
  time_t ctime;
  time_t atime;
  time_t mtime;
  time_t dtime;
  BYTE flags;
  BYTE uiflags;   // Internal flags not in record in file
  unsigned char odigest[SHA1::HASHLEN];
  unsigned char cdigest[SHA1::HASHLEN];
  StringX filename;
  StringX path;
  StringX description;

  uuid_array_t attmt_uuid;
  uuid_array_t entry_uuid;
};

// Attachment Extended Record
struct ATRecordEx {
  ATRecordEx()
  : sxGroup(_T("")), sxTitle(_T("")), sxUser(_T(""))
  {
    atr.Clear();
  }

  ATRecordEx(const ATRecordEx &atrex)
    : atr(atrex.atr), sxGroup(atrex.sxGroup),
      sxTitle(atrex.sxTitle), sxUser(atrex.sxUser)
  {
  }

  ATRecordEx &operator=(const ATRecordEx &atrex)
  {
    if (this != &atrex) {
      atr = atrex.atr;
      sxGroup = atrex.sxGroup;
      sxTitle = atrex.sxTitle;
      sxUser = atrex.sxUser;
    }
    return *this;
  }

  void Clear() {
    atr.Clear();
    sxGroup = sxTitle = sxUser = _T("");
  }

  ATRecord atr;
  StringX sxGroup;
  StringX sxTitle;
  StringX sxUser;
};

// Attachment Filter structure
struct ATFilter {
  ATFilter()
  : set(0), object(0), function(0),
    value(_T(""))
  {}

  ATFilter(const ATFilter &af)
    : set(af.set), object(af.object), function(af.function),
    value(af.value)
  {}

  ATFilter &operator=(const ATFilter &af)
  {
    if (this != &af) {
      set = af.set;
      object = af.object;
      function = af.function;
      value = af.value;
    }
    return *this;
  }

  void Clear()  {
    set = object = function = 0;
    value = _T("");
  }

  int set;
  int object;
  int function;
  stringT value;
};

// Attachment progress structure
// Supported functions
#define ATT_PROGRESS_START       0x00

#define ATT_PROGRESS_PROCESSFILE 0x10
#define ATT_PROGRESS_SEARCHFILE  0x11
#define ATT_PROGRESS_EXTRACTFILE 0x12
#define ATT_PROGRESS_EXPORTFILE  0x13

#define ATT_PROGRESS_ERROR       0x80
#define ATT_PROGRESS_END         0xFF

struct ATTProgress {
  ATTProgress()
  : function(0), value(0), function_text(_T(""))
  {}

  ATTProgress(const ATTProgress &atpg)
    : atr(atpg.atr), function(atpg.function), value(atpg.value),
    function_text(atpg.function_text)
  {}

  ATTProgress &operator=(const ATTProgress &atpg)
  {
    if (this != &atpg) {
      atr = atpg.atr;
      function = atpg.function;
      value = atpg.value;
      function_text = atpg.function_text;
    }
    return *this;
  }

  void Clear()  {
    atr.Clear();
    function = value = 0;
    function_text = _T("");
  }

  ATRecord atr;
  StringX function_text;
  int function;
  int value;
};

typedef std::vector<ATRecord> ATRVector;
typedef std::vector<ATRecordEx> ATRExVector;
typedef std::vector<ATFilter> ATFVector;

typedef std::vector<ATRecord>::iterator ATRViter;
typedef std::vector<ATRecordEx>::iterator ATRExViter;
typedef std::vector<ATFilter>::iterator ATFViter;


#endif /* __ATTACHMENTS_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
