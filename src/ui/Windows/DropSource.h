/*
* Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "afxole.h"
// Drag and Drop source
// Note values to stop D&D between instances where data is of different lengths
enum {
  FROMCC = 0,       // From Column Chooser Dialog
  FROMHDR = 1,      // From ListCtrl Header
  FROMTREE_L = 2,   // From TreeCtrl - left  mouse D&D
  FROMTREE_R = 4,   // From TreeCtrl - right mouse D&D
  FROMTREE_RSC = 8  // From TreeCtrl - right mouse D&D - create Shortcut allowed
};

class CDataSource : public COleDataSource
{
public:
  CDataSource();
  virtual ~CDataSource();
  virtual DROPEFFECT StartDragging(BYTE *szData, DWORD dwLength,
    CLIPFORMAT cpfmt, RECT* rClient, CPoint* ptMousePos);

protected:
  virtual void CompleteMove() {};

private:
};
