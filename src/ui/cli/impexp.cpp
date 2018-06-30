/*
 * Created by Saurav Ghosh
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#include "./impexp.h"

#include "./argutils.h"
#include "../../os/file.h"

#include "core/PWSdirs.h"

#include <string>

using namespace std;

static int ImportText(PWScore &core, const stringT &fname);
static int ImportXML(PWScore &core, const stringT &fname);

int Import(PWScore &core, const UserArgs &ua)
{
  return ua.Format == UserArgs::XML?
      ImportXML(core, ua.opArg): ImportText(core, ua.opArg);
}

int Export(PWScore &core, const UserArgs &ua)
{
  CItemData::FieldBits all(~0UL);
  int N;
  return ua.Format == UserArgs::XML?
    core.WriteXMLFile(std2stringx(ua.opArg), all, L"", 0, 0, L' ', L"", N):
      core.WritePlaintextFile(std2stringx(ua.opArg), all, L"", 0, 0, L' ', N);
}

static int
ImportText(PWScore &core, const stringT &fname)
{
  int numImported(0), numSkipped(0), numPWHErrors(0), numRenamed(0), numNoPolicy(0);
  std::wstring strError;
  wchar_t delimiter = L' ';
  wchar_t fieldSeparator = L'\t';
  StringX ImportedPrefix;
  bool bImportPSWDsOnly = false;

  // Create report as we go
  CReport rpt;
  rpt.StartReport(L"Import_Text", core.GetCurFile().c_str());
  wstring str(L"Text file being imported: ");
  str += core.GetCurFile().c_str();
  rpt.WriteLine(str.c_str());
  rpt.WriteLine();

  Command *pcmd = NULL;
  int rc = core.ImportPlaintextFile(ImportedPrefix, std2stringx(fname), fieldSeparator,
                                    delimiter, bImportPSWDsOnly,
                                    strError,
                                    numImported, numSkipped,
                                    numPWHErrors, numRenamed, numNoPolicy,
                                    rpt, pcmd);
  switch (rc) {
  case PWScore::CANT_OPEN_FILE:
  case PWScore::INVALID_FORMAT:
  case PWScore::FAILURE:
    delete pcmd;
    break;
  case PWScore::SUCCESS:
  case PWScore::OK_WITH_ERRORS:
    // deliberate fallthrough
  default:
    {
      rc = core.Execute(pcmd);

      rpt.WriteLine();
      wstring op(bImportPSWDsOnly ? L"Updated " : L"Imported ");
      wstring entries(numImported == 1 ? L" entry" : L" entries");
      wostringstream os;
      os << op << numImported << entries;
      rpt.WriteLine(os.str().c_str());

      if (numSkipped != 0) {
        wostringstream oss;
        entries = (numSkipped == 1 ?  L" entry" : L" entries");
        oss << L"\nSkipped " << numSkipped << entries;
        rpt.WriteLine(oss.str().c_str());
      }

      if (numPWHErrors != 0) {
        wostringstream oss;
        entries = (numPWHErrors == 1 ?  L" entry" : L" entries");
        oss << L"\nwith Password History errors" << numPWHErrors << entries;
        rpt.WriteLine(oss.str().c_str());
      }

      if (numRenamed != 0) {
        wostringstream oss;
        entries = (numRenamed == 1 ?  L" entry" : L" entries");
        oss << L"\nRenamed " << numRenamed << entries;
        rpt.WriteLine(oss.str().c_str());
      }
      break;
    }
  } // switch
  rpt.EndReport();
  return rc;
}

static int
ImportXML(PWScore &core, const stringT &fname)
{
  const std::wstring XSDfn(L"pwsafe.xsd");
  std::wstring XSDFilename = PWSdirs::GetXMLDir() + XSDfn;

#if USE_XML_LIBRARY == MSXML || USE_XML_LIBRARY == XERCES
  if (!pws_os::FileExists(XSDFilename)) {
    wcerr << L"Can't find XML Schema Definition file"
          << XSDFilename << endl
          << L"Can't import without it." << endl;
    return PWScore::XML_FAILED_IMPORT;
  }
#endif

  std::wstring ImportedPrefix;
  std::wstring dir;
  std::wstring strXMLErrors, strSkippedList, strPWHErrorList, strRenameList;
  int numValidated(0), numImported(0), numSkipped(0), numRenamed(0), numPWHErrors(0);
	int numNoPolicy(0), numRenamedPolicies(0), numShortcutsRemoved(0), numEmptyGroupsImported{0};
  bool bImportPSWDsOnly = false;


  // Create report as we go
  CReport rpt;
  std::wstring str_text;
  rpt.StartReport(L"Import_XML", core.GetCurFile().c_str());
  str_text = L"XML file being imported: ";
  str_text += fname.c_str();
  rpt.WriteLine(str_text);
  rpt.WriteLine();
  std::vector<StringX> vgroups;
  Command *pcmd = NULL;

  int rc = core.ImportXMLFile(ImportedPrefix.c_str(), fname.c_str(),
                              XSDFilename.c_str(), bImportPSWDsOnly,
                              strXMLErrors, strSkippedList, strPWHErrorList,
                              strRenameList, numValidated, numImported,
                              numSkipped, numPWHErrors, numRenamed,
                              numNoPolicy, numRenamedPolicies, numShortcutsRemoved,
							  numEmptyGroupsImported,
                              rpt, pcmd);

  switch (rc) {
  case PWScore::XML_FAILED_VALIDATION:
    rpt.WriteLine(strXMLErrors.c_str());
    str_text = L"File ";
    str_text += fname.c_str();
    str_text += L" failed to validate.";
    delete pcmd;
    break;
  case PWScore::XML_FAILED_IMPORT:
    rpt.WriteLine(strXMLErrors.c_str());
    str_text = L"File ";
    str_text += fname.c_str();
    str_text += L" validated but had errors during import.";
    delete pcmd;
    break;
  case PWScore::SUCCESS:
  case PWScore::OK_WITH_ERRORS:
    if (pcmd != NULL)
      rc = core.Execute(pcmd);

    if (!strXMLErrors.empty() ||
        numRenamed > 0 || numPWHErrors > 0) {
      wstring csErrors;
      if (!strXMLErrors.empty())
        csErrors = strXMLErrors + L"\n";

      if (!csErrors.empty()) {
        rpt.WriteLine(csErrors.c_str());
      }

      wstring cs_renamed, cs_PWHErrors, cs_skipped;
      if (numSkipped > 0) {
        rpt.WriteLine(wstring(L"The following records were skipped:"));
        wostringstream oss;
        oss << L" / skipped " << numSkipped;
        cs_skipped = oss.str();
        rpt.WriteLine(strSkippedList.c_str());
        rpt.WriteLine();
      }
      if (numPWHErrors > 0) {
        rpt.WriteLine(wstring(L"The following records had errors in their Password History:"));
        wostringstream oss;
        oss << L" / with Password History errors " << numPWHErrors;
        cs_PWHErrors = oss.str();
        rpt.WriteLine(strPWHErrorList.c_str());
        rpt.WriteLine();
      }
      if (numRenamed > 0) {
        rpt.WriteLine(wstring(L"The following records were renamed as an entry already exists in your database or in the Import file:"));
        wostringstream oss;
        oss << L" / renamed " << numRenamed;
        cs_renamed = oss.str();
        rpt.WriteLine(strRenameList.c_str());
        rpt.WriteLine();
      }

      wostringstream os2;
      os2 << L"File " << fname << L" was imported (entries validated"
          << numValidated << L" / imported " << numImported << L")"
          << cs_skipped <<  cs_renamed << cs_PWHErrors;
      str_text = os2.str();
    } else {
      const wstring validate(numValidated == 1 ? L" entry" : L" entries");
      const wstring import(numImported == 1 ? L" entry" : L" entries");
      wostringstream oss;
      oss << L"Validated " << numValidated << validate << L'\t'
          << L"Imported " << numImported << import << endl;
      str_text = oss.str();
    }

    break;
  default:
    ASSERT(0);
  } // switch

    // Finish Report
  rpt.WriteLine(str_text);
  rpt.EndReport();
  return rc;
}

