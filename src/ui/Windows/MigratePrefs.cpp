/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * PerformConfigMigration() should be called if (a) we detected the preference file
 * in the old location (exec. dir), and (b) user chose to migrate.
 * To be more accurate: In addition to being in the exec dir, the current
 * username/hostname combination should be in the file, since when there are
 * several u/h prefs, we migrate only the current one.
 */

#include "stdafx.h"
#include "GeneralMsgBox.h"
#include "corelib/VerifyFormat.h"
#include "corelib/PWSprefs.h"
#include "corelib/PWSdirs.h"
#include "corelib/SysInfo.h"
#include "corelib/XMLprefs.h"
#include "os/dir.h"
#include "os/file.h"

bool OfferConfigMigration()
{
  /**
   * Offer the user the option of migrating config files iff ALL
   * of the following are true:
   * 1. Config file is currently in executable directory
   * 2. This is NOT a U3 installation
   * 3. The executable directory is on a fixed or network drive
   * 4. The user did NOT override the config file, user name or host name
   *    via command line (-g, -u, -h).
   * 5. There isn't a config file already in the APPDATA location
   * 6. The APPDATA location exists
   */
  const SysInfo *si = SysInfo::GetInstance();

  // start with quickest checks
  if (si->IsUnderU3() || PWSprefs::UserSetCfgFile() ||
      (si->GetRealHost() != si->GetEffectiveHost()) ||
      (si->GetRealUser() != si->GetEffectiveUser()))
    return false;

  std::wstring wsExecDir = pws_os::getexecdir();
  std::wstring wsExDrive, wsExDir, wsExFileName, wsExExt;

  pws_os::splitpath(wsExecDir, wsExDrive, wsExDir, wsExFileName, wsExExt);
  wsExDrive += L"\\";

  UINT uiDT = GetDriveType(wsExDrive.c_str());
  // Do not touch if not on local or remote (network) disk
  if (uiDT != DRIVE_FIXED && uiDT != DRIVE_REMOTE)
    return false;
      
  const std::wstring wsUserCfgDir = pws_os::getuserprefsdir(); // empty if couldn't find/create
  if (wsUserCfgDir.empty())
    return false;
  const std::wstring wsExecDirCfgFile = wsExecDir + PWSprefs::cfgFileName;
  const std::wstring wsDefaultCfgFile = wsUserCfgDir + PWSprefs::cfgFileName;
  return (pws_os::FileExists(wsExecDirCfgFile) &&
          !pws_os::FileExists(wsDefaultCfgFile));
}

bool PerformConfigMigration()
{
  /**
   *
   * We're here after the application's started and the conditions
   * listed above (in OfferConfigMigration) hold.
   * This constrains what we can assume and what we have to check.
   */

  ASSERT(OfferConfigMigration()); // should not be here otherwise!
  if (!OfferConfigMigration()) return false; // I mean it!

  CGeneralMsgBox gmb;  // Note: CGeneralMsgBox is not re-useable.
  CXMLprefs *pXML_Config(NULL);
  PWSprefs::ConfigOption configoption;  // Note value meaningless at this point!
  std::wstring wsCnfgFile = PWSprefs::GetConfigFile(configoption);
  const std::wstring wsExecDir = pws_os::getexecdir();
  const std::wstring wsUserCfgDir = pws_os::getuserprefsdir();

  if (wsUserCfgDir.empty()) // couldn't find or create !?
    return false;

  std::wstring wsDefaultCfgFile = wsUserCfgDir + PWSprefs::cfgFileName;
  std::wstring wsExecDirCfgFile = wsExecDir + PWSprefs::cfgFileName;
  bool bRetVal(false);
  bool bExecCFRO(false);
  pws_os::FileExists(wsExecDirCfgFile, bExecCFRO);

  /**
   *  MIGRATE
  **/

  bRetVal = false;
  bool bNoMoreNodes(false);
  // We have current config file. Create the new one from it just containing our host/user
  const SysInfo *si = SysInfo::GetInstance();
  bool rc = pXML_Config->MigrateSettings(wsDefaultCfgFile,
                                         si->GetEffectiveHost(), si->GetEffectiveUser());
  if (rc) {
    // That worked but we can't use same CXMLprefs to remove us from the one
    // in the Installation directory - so cleanup and reload.
    pXML_Config->Unlock();
    delete pXML_Config;

    // Since we now have new config file, remove host/user from old.
    pXML_Config = new CXMLprefs(wsExecDirCfgFile.c_str());
    if (!pXML_Config->Load()) {
      rc = false;
      if (!pXML_Config->getReason().empty()) {
        CGeneralMsgBox gmb;
        gmb.SetMsg(pXML_Config->getReason().c_str());
        gmb.AddButton(IDS_CONTINUE, IDS_CONTINUE);
        gmb.AddButton(IDS_EXIT, IDS_EXIT, TRUE, TRUE);
        if (gmb.DoModal() == IDS_EXIT) {
          goto exit;
        }

        // Problem loading XML file but user says continue rather than Exit PWS!
        // But we will not remove them from the old file and we will
        // delete the new file - better luck next time!
        pws_os::DeleteAFile(wsDefaultCfgFile);
      }
    } // Load failed

    // Now remove this hostname/username from old configuration file in the
    // installation directory (as long as everything OK and it is not R/O)
    if (rc && !bExecCFRO) {
      rc = pXML_Config->RemoveHostnameUsername(si->GetEffectiveHost(),
                                               si->GetEffectiveUser(), bNoMoreNodes);
      if (rc) {
        // Save it
        pXML_Config->Store();

        // However, if no more host/user nodes in this file - delete the
        // configuration file from the installation directory!
        if (bNoMoreNodes) {
          pws_os::DeleteAFile(wsExecDirCfgFile);
        }

        // Use new config file !!!
        wsCnfgFile = L"";
        bRetVal = true;
      } // RemoveHostnameUsername
    } // rc && !bExecCFRO
  } // MigrateSettings

  // If this all worked, now copy autoload_filters.xml if it exists and not
  // already in the new location.
  // This is ONLY done when we migrate the user's settings.
  if (bRetVal == true) {
    bool bCopyAutoloadFilters(false), bALFRO(false);
    std::wstring wsOldAutoLoadFilters = wsExecDir + L"autoload_filters.xml";
    std::wstring wsNewAutoLoadFilters = wsUserCfgDir + L"autoload_filters.xml";
    if (pws_os::FileExists(wsOldAutoLoadFilters, bALFRO) &&
        !pws_os::FileExists(wsNewAutoLoadFilters)) {
      bCopyAutoloadFilters = pws_os::CopyAFile(wsOldAutoLoadFilters,
                                               wsNewAutoLoadFilters);

     // If we have copied it, there are no more nodes in the old configuration file
     // and it isn't read only - delete it from the installation directory
     if (bCopyAutoloadFilters && bNoMoreNodes && !bALFRO)
       pws_os::DeleteAFile(wsOldAutoLoadFilters);
    }
  }

  // Migration all done!

exit:
  // Clean up
  if (pXML_Config != NULL) {
    pXML_Config->Unlock();
    delete pXML_Config;
  }

  // Set config file - blank means use new default
  PWSprefs::SetConfigFile(wsCnfgFile);
  return bRetVal;
}
