#pragma once

#include <vector>

/*
GF:
21-aug-2008 GF: Version 1.0
28-aug-2008 GF: Fixed DT_RIGHT
        added GetMinMaxInfo()
10-sep-2008 GF: added ReadWriteToRegistry(int DialogID)
18-Feb-2009 GF: minmax size was client instead of GetWindowRect. Fixed.


This class is a very convenient helper for resizable dialogs.

Useage:
 // --- DIALOG H FILE ---
  class CMyDialog : public CDialog
 {
  virtual BOOL OnInitDialog();
  afx_msg void OnSize(UINT nType, int cx, int cy);

  GFResizeDialogHelper m_Resizer;
 }

 // --- DIALOG CPP FILE ---
 BOOL CMyDialog::OnInitDialog()
 {
  ...
  m_Resizer.Init(this);
  m_Resizer.AddCtrl(IDOK, ... (see below) );
 }

 void CMyDialog::OnSize(UINT nType, int cx, int cy)
 {
  CDialog::OnSize(nType, cx, cy);
  m_Resizer.OnSize(cx,cy);
 }

 // optionally let GFDialogResizer stop users
 // from making the window smaller than in the
 // dialog editor:
 void CMyDialog::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
 {
  m_Resizer.GetMinMaxInfo(lpMMI);
 }


// Usage of AddCtrl:
 int id:
  The ID of the control (IDC_...)
 int align:
  The alignment of the control. If you align it
  Horizontally:
   DT_LEFT, the left-coordinate will stay where it is.
   DT_CENTER, the center of the control will stay where it was, relative to the dialog.
   DT_RIGHT, the right border of the control will keep the distance to the right border of the dialog.
  Vertically:
   DT_TOP,  the top position will stay
   DT_VCENTER, the vertical center will stay where it was relative to the dialog.
   DT_BOTTOM, the control will keep the distance to the bottom of the dialog
 BOOL keep_dx
  Should the control keep its width?
 BOOL keep_dy
  Should the control keep its height?
*/

#define GRIPPER_SQUARE_SIZE 20

class GFResizeDialogHelper
{
public:
  GFResizeDialogHelper(void);
  ~GFResizeDialogHelper(void);

  void Init(CDialog* pDialog);
  void AddCtrl(int id, int align=DT_LEFT|DT_TOP, BOOL keep_dx=TRUE, BOOL keep_dy=TRUE);
  void OnSize(int cx, int cy);
  void GetMinMaxInfo(MINMAXINFO* pMMI); // get a minmax info, so the dialog won't get smaller than in the resource editor

protected:

 // --- information about a ctrl ---
 struct CTRL_ALIGN
 {
   CTRL_ALIGN(): idWnd(0), align(DT_TOP|DT_LEFT), keep_dx(TRUE), keep_dy(TRUE)
       {place.SetRect(0,0,0,0);}
   CTRL_ALIGN(const CTRL_ALIGN& c)
       {*this = c;}
   CTRL_ALIGN& operator=(const CTRL_ALIGN& c)
       {
        idWnd = c.idWnd;
        align=c.align;
        keep_dx=c.keep_dx;
        keep_dy=c.keep_dy;
        place=c.place;
        return *this;
       }
  // data
   int idWnd;
   int align; // combination of: DT_LEFT,DT_CENTER,DT_RIGHT|DT_TOP,DT_VCENTER,DT_BOTTOM - where is this control "sticky"
   BOOL keep_dx, keep_dy; // keep size, or scale?
   CRect place;
 };

 CDialog* m_pDialog;
 CSize m_OrigSize;  // client size for child alignment
 CSize m_OrigWndSize; // for minmax info
 std::vector<CTRL_ALIGN> m_Ctrls;


 CSize  m_CurSize;
};
