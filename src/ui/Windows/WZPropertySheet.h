/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "AdvancedValues.h"
#include "ThisMfcApp.h"
#include "DboxMain.h"

#include "core/StringX.h"

class CWZAdvanced;
class CWZSelectDB;
class CWZFinish;

class CWZPropertySheet : public CPropertySheet
{
public:
  DECLARE_DYNAMIC(CWZPropertySheet)

  CWZPropertySheet(UINT nID, CWnd* pDbx, WZAdvanced::AdvType iadv_type, 
                   st_SaveAdvValues *pst_SADV);
  ~CWZPropertySheet();

  UINT GetID() const {return m_nID;}
  UINT GetButtonID() const {return m_nButtonID;}
  StringX GetPassKey() const {return m_passkey;}
  StringX GetExportPassKey() const { return m_exportpasskey; }
  StringX GetOtherDBFile() const {return m_filespec;}
  st_SaveAdvValues *GetAdvValues() const {return m_pst_SADV;}
  wchar_t GetDelimiter() const {return m_delimiter;}
  bool GetAdvanced() const {return m_bAdvanced;}
  bool GetExportDBFilters() const { return m_bExportDBFilters; }
  bool GetCompleted() const {return m_bCompleted;}

  void SetPassKey(const StringX passkey) {m_passkey = passkey;}
  void SetExportPassKey(const StringX passkey) { m_exportpasskey = passkey; }
  void SetOtherDB(const StringX filespec) {m_filespec = filespec;}
  void SetAdvValues(st_SaveAdvValues *pst_SADV) {m_pst_SADV = pst_SADV;}
  void SetDelimiter(const wchar_t &delimiter) {m_delimiter = delimiter;}
  void SetAdvanced(const bool &bAdvanced) {m_bAdvanced = bAdvanced;}
  void SetExportDBFilters(const bool &bExportDBFilters)
  { m_bExportDBFilters = bExportDBFilters; }
  void SetCompleted(const bool &bCompleted) {m_bCompleted = bCompleted;}

  void WZPSHMakeOrderedItemList(OrderedItemList &OIL)
  {app.GetMainDlg()->MakeOrderedItemList(OIL);}

  CItemData *WZPSHgetSelectedItem()
  {return app.GetMainDlg()->getSelectedItem();}

  int WZPSHTestSelection(const bool bAdvanced,
                         const std::wstring &subgroup_name,
                         const int &subgroup_object,
                         const int &subgroup_function,
                         const OrderedItemList *pOIL)
  {return app.GetMainDlg()->TestSelection(bAdvanced, subgroup_name,
                                subgroup_object, subgroup_function, pOIL);}

  void WZPSHSetUpdateWizardWindow(CWnd *pWnd)
  {app.GetMainDlg()->SetUpdateWizardWindow(pWnd);}

  bool WZPSHDoCompare(PWScore *pothercore, const bool bAdvanced, CReport *prpt,
                      bool *pbCancel)
  {return app.GetMainDlg()->DoCompare(pothercore, bAdvanced, prpt, pbCancel);}

  std::wstring WZPSHDoMerge(PWScore *pothercore, const bool bAdvanced, CReport *prpt,
                            bool *pbCancel)
  {return app.GetMainDlg()->DoMerge(pothercore, bAdvanced, prpt, pbCancel);}

  void WZPSHDoSynchronize(PWScore *pothercore,
                          const bool bAdvanced, int &numExported, CReport *prpt,
                          bool *pbCancel)
  {app.GetMainDlg()->DoSynchronize(pothercore, bAdvanced, numExported, prpt, pbCancel);}

  int WZPSHDoExportText(const StringX &sx_Filename, const UINT nID,
                        const wchar_t &delimiter, const bool bAdvanced, 
                        int &numExported, CReport *prpt)
  {return app.GetMainDlg()->DoExportText(sx_Filename, nID, delimiter, bAdvanced, numExported,
                               prpt);}

  int WZPSHDoExportXML(const StringX &sx_Filename, const UINT nID,
                       const wchar_t &delimiter, const bool bAdvanced,
                       int &numExported, CReport *prpt)
  {return app.GetMainDlg()->DoExportXML(sx_Filename, nID, delimiter, bAdvanced, numExported,
                              prpt);}

  int WZPSHDoExportDB(const StringX &sx_Filename, const UINT nID,
                      const bool bExportDBFilters,
                      const StringX &sx_ExportKey, int &numExported, CReport *prpt)
  {return app.GetMainDlg()->DoExportDB(sx_Filename, nID,
                                       bExportDBFilters, sx_ExportKey, numExported, prpt);}

  void WZPSHViewReport(CReport &prpt)
  {app.GetMainDlg()->ViewReport(prpt);}

  StringX WZPSHGetCurFile()
  {return app.GetMainDlg()->GetCurFile();}

  bool WZPSHExitRequested() const
  {return app.GetMainDlg()->ExitRequested();}

  int WZPSHCheckPasskey(const StringX &filename, const StringX &passkey,
                        PWScore *pcore)
  {return app.GetMainDlg()->CheckPasskey(filename, passkey, pcore);}

  void WZPSHUpdateGUIDisplay()
  {app.GetMainDlg()->UpdateGUIDisplay();}

  CString WZPSHShowCompareResults(const StringX sx_Filename1, const StringX sx_Filename2,
                                  PWScore *pothercore, CReport *prpt)
  {return app.GetMainDlg()->ShowCompareResults(sx_Filename1, sx_Filename2, pothercore, prpt);}

  void SetNumProcessed(const int numProcessed) {m_numProcessed = numProcessed;}
  int GetNumProcessed() {return m_numProcessed;}

  // This is needed for Export to DB
  void SetDBVersion(const PWSfile::VERSION current_ver)
  {DB_version = current_ver;}
  PWSfile::VERSION GetDBVersion() {return DB_version;}

  // Needs to be public for access by DboxMain (MainFile.cpp)
  virtual INT_PTR DoModal();

protected:
  virtual BOOL PreTranslateMessage(MSG *pMsg);

  // Following override to reset idle timeout on any event
  virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
  // Following override to stop accelerators interfering
  virtual void PreSubclassWindow();

  DECLARE_MESSAGE_MAP()

private:
  CWZAdvanced *m_pp_advanced;
  CWZSelectDB *m_pp_selectdb;
  CWZFinish   *m_pp_finish;

  UINT m_nID, m_nButtonID;

  StringX m_passkey, m_exportpasskey;
  StringX m_filespec;
  st_SaveAdvValues *m_pst_SADV;
  wchar_t m_delimiter;
  bool m_bAdvanced, m_bExportDBFilters, m_bCompleted;
  int m_numProcessed;

  PWSfile::VERSION DB_version;
};
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
