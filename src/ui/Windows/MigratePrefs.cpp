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

  // For Startup Options that effect migration to Local AppData (MFC version only)
  enum ConfigOptions {
    CFG_CONFIGFILE_SUPPLIED  = 0x80,
    CFG_USERNAME_SUPPLIED    = 0x40,
    CFG_HOSTNAME_SUPPLIED    = 0x20,
    CFG_UNUSED               = 0x1f};

static BYTE s_CFGOpt;

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
   */
  SysInfo *si = SysInfo::GetInstance();

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
      
  const std::wstring wsExecDirCfgFile = wsExecDir + L"pwsafe.cfg";
  return pws_os::FileExists(wsExecDirCfgFile);
}

bool PerformConfigMigration()
{
  ASSERT(!SysInfo::IsUnderU3()); // Do not touch U3 installations

  std::wstring wsExecDir = pws_os::getexecdir();
  std::wstring wsExDrive, wsExDir, wsExFileName, wsExExt;

  pws_os::splitpath(wsExecDir, wsExDrive, wsExDir, wsExFileName, wsExExt);
  wsExDrive += L"\\";

  UINT uiDT = GetDriveType(wsExDrive.c_str());
  // Do not touch if not on local or remote disk
  if (uiDT != DRIVE_FIXED && uiDT != DRIVE_REMOTE)
    return true;

  CGeneralMsgBox gmb;  // Note: CGeneralMsgBox is not re-useable.
  CXMLprefs *pXML_Config(NULL);
  PWSprefs::ConfigOption configoption;  // Note value meaningless at this point!
  std::wstring wsCnfgFile = PWSprefs::GetConfigFile(configoption);
  std::wstring wsUserCfgDir = pws_os::getuserprefsdir();
  std::wstring wsDefaultCfgFile = wsUserCfgDir + L"pwsafe.cfg";
  std::wstring wsExecDirCfgFile = wsExecDir + L"pwsafe.cfg";
  bool bRetVal(false);
  bool bExecCFRO;
  bool bExecCFExists = pws_os::FileExists(wsExecDirCfgFile, bExecCFRO);
  bool bDfltCFExists = pws_os::FileExists(wsDefaultCfgFile);

  // Set up XML "keys": host/user, ensure that they start with letter,
  // and otherwise conforms with http://www.w3.org/TR/2000/REC-xml-20001006#NT-Name
  SysInfo *si = SysInfo::GetInstance();
  std::wstring hostname = si->GetEffectiveHost();
  PWSprefs::XMLify(L'H', hostname);
  std::wstring username = si->GetEffectiveUser();
  PWSprefs::XMLify(L'u', username);
  std::wstring wsHU, wsHKCU, wsHKCU_PREF;
  wsHKCU = hostname;
  wsHKCU += L"\\";
  wsHKCU += username;
  wsHKCU_PREF = wsHKCU + L"\\Preferences";

  ASSERT((wsCnfgFile.empty() && s_CFGOpt == 0) || 
         (!wsCnfgFile.empty() && (s_CFGOpt & CFG_CONFIGFILE_SUPPLIED) != 0));

  if (s_CFGOpt != 0) {
    // User supplied config file, hostname and/or username on Command Line
    // Do NOT migrate - but need to check on where the config file is exactly, and
    // if not found - ask the user what to do!
    if ((s_CFGOpt & CFG_CONFIGFILE_SUPPLIED) != 0) {
      std::wstring wsCnfgDrive, wsCnfgDir, wsCnfgFileName, wsCnfgExt;
      pws_os::splitpath(wsCnfgFile, wsCnfgDrive, wsCnfgDir, wsCnfgFileName, wsCnfgExt);
      if (wsCnfgDrive.empty() && wsCnfgDir.empty()) {
        // Relative file name
        // Try in Installation directory first
        wsCnfgFile = wsExecDir + wsCnfgFileName + wsCnfgExt;
        if (pws_os::FileExists(wsCnfgFile)) {
          bRetVal = true;
        } else {
          // Not in Installation directory - try Local APPDATA directory
          wsCnfgFile = wsUserCfgDir + wsCnfgFileName + wsCnfgExt;
          if (pws_os::FileExists(wsCnfgFile)) {
            bRetVal = true;
          }
        }
      } else {
        // Fullpath to config file supplied by user
        if (pws_os::FileExists(wsCnfgFile))
          bRetVal = true;
      }

      if (!bRetVal) {
        // Got here because we couldn't find user specified configuration file - tell them
        CGeneralMsgBox gmb;
        gmb.SetMsg(IDS_CANTFINDCONFIG);
        gmb.AddButton(IDS_CONTINUE, IDS_CONTINUE);
        gmb.AddButton(IDS_EXIT, IDS_EXIT, TRUE, TRUE);
        if (gmb.DoModal() == IDS_EXIT) {
          goto exit;
        }

        // OK - user said ignore file - so go with the default and ignore any other
        // values (host/user) they may have specified
        bRetVal = true;
        wsCnfgFile = L"";
        s_CFGOpt = 0;
      }
    } else {
      // No config file specified - see if already migrated
      if (!bDfltCFExists && !bExecCFExists) {
        // No specified config file, no old & no new config file - just exit
        // but clear irrelevant flags as no config file exists yet!
        s_CFGOpt = 0;
        return true;
      }
    }

    // We have the config file - now check if user supplied host/user exists
    if ((s_CFGOpt & CFG_USERNAME_SUPPLIED) != 0 ||
        (s_CFGOpt & CFG_HOSTNAME_SUPPLIED) != 0) {
      pXML_Config = new CXMLprefs(wsCnfgFile.empty() ? wsDefaultCfgFile.c_str() :
                                             wsCnfgFile.c_str());
      if (!pXML_Config->Load()) {
        if (!pXML_Config->getReason().empty()) {
          CGeneralMsgBox gmb;
          gmb.SetMsg(pXML_Config->getReason().c_str());
          gmb.AddButton(IDS_CONTINUE, IDS_CONTINUE);
          gmb.AddButton(IDS_EXIT, IDS_EXIT, TRUE, TRUE);
          if (gmb.DoModal() == IDS_EXIT) {
            bRetVal = false;
            goto exit;
          }
          // Problem loading XML file but user says continue!
          bRetVal = true;
        }
      }

      // Are the supplied host/user already in the config file?
      wsHU = pXML_Config->Get(wsHKCU, L"LastUpdated", L"");
      time_t tt;
      if (!VerifyXMLDateTimeString(wsHU, tt)) {
        // Oh dear - tell the user
        CGeneralMsgBox gmb;
        gmb.SetMsg(IDS_CANTFINDUSERHOST);
        gmb.AddButton(IDS_CONTINUE, IDS_CONTINUE);
        gmb.AddButton(IDS_EXIT, IDS_EXIT, TRUE, TRUE);
        if (gmb.DoModal() == IDS_EXIT) {
          bRetVal = false;
          goto exit;
        }
        // OK - user said ignore supplied host and/or user
        // Reset both fields even though we may not have found only one of them
        const std::wstring ws_rhost = si->GetRealHost();
        const std::wstring ws_ruser = si->GetRealUser();
        si->SetEffectiveHost(ws_rhost);
        si->SetEffectiveUser(ws_ruser);
      }
    }
    // Right - we have got here and either have a valid config file or the default
    // or used their specified host/user or not.
    // So do NOT migrate and now leave
    goto exit;
  }

  // User has not specified a config file or a host/user value.
  // Check if they have a config file in their Local APPDATA directory
  if (bDfltCFExists) {
    bRetVal = true;
    goto exit;
  }

  // OK not there - is it in the Installation directory?
  if (!bExecCFExists) {
    //  No - use default - which means PWS will create it as a first time user
    bRetVal = true;
    goto exit;
  }

  // OK must be only in Installation directory
  pXML_Config = new CXMLprefs(wsExecDirCfgFile.c_str());
  if (!pXML_Config->Load()) {
    // But we couldn't load it - use default
    bRetVal = true;
    goto exit;
  }

  // OK - in Installation directory and we can read it but
  // are we (host/user) already in the config file?
  wsHU = pXML_Config->Get(wsHKCU, L"LastUpdated", L"");
  time_t tt;
  if (!VerifyXMLDateTimeString(wsHU, tt)) {
    //  No - use default
    bRetVal = true;
    goto exit;
  }

  // OK - in Installation directory, we can read it and we (host/user) are in it
  // Migrate - but first check to see if they have already declined the migration
  int iUserSaidNo = pXML_Config->Get(wsHKCU_PREF, _T("DoNotMigrateToAPPDATA"), FALSE);
  if (iUserSaidNo == TRUE) {
    // They already said no - so leave now
    wsCnfgFile = wsExecDirCfgFile;
    bRetVal = true;
    goto exit;
  }

  // They didn't previously decline, so ask the user if they want to do it now!?
  if (gmb.AfxMessageBox(IDS_CONFIRM_MIG2APPDATA, MB_YESNO|MB_ICONQUESTION) == IDNO) {
    // But they have now!
    wsCnfgFile = wsExecDirCfgFile;
    // Must set config file before first call to "PWSprefs::GetInstance()"
    PWSprefs::SetConfigFile(wsCnfgFile);
    // GetInstance will force reading in of preferences in order to set this flag
    PWSprefs::GetInstance()->SetPref(PWSprefs::DoNotMigrateToAPPDATA, true);
    // And save in case they cancel the startup
    PWSprefs::GetInstance()->SaveApplicationPreferences();
    bRetVal = true;
    goto exit;
  }

  /**
   *  MIGRATE
  **/

  bRetVal = false;
  bool bNoMoreNodes(false);
  // We have current config file. Create the new one from it just containing our host/user
  bool rc = pXML_Config->MigrateSettings(wsDefaultCfgFile, hostname, username);
  if (rc) {
    // That worked but we can't use same CXMLprefs to remove us from the one
    // in the Installation directory - so cleanup and reload.
    pXML_Config->Unlock();
    delete pXML_Config;
    pXML_Config = NULL;

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
    }

    // Now remove this hostname/username from old configuration file in the
    // installation directory (as long as everything OK and it is not R/O)
    if (rc && !bExecCFRO) {
      rc = pXML_Config->RemoveHostnameUsername(hostname, username, bNoMoreNodes);
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
      }
    }
  }

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
    pXML_Config = NULL;
  }

  // Set config file - blank means use new default
  PWSprefs::SetConfigFile(wsCnfgFile);
  return bRetVal;
}
