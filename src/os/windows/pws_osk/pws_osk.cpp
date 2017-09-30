/*
* Copyright (c) 2009-2017 David Kelvin <c-273@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

// pws_osk.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"

#include "pws_osk.h"

#include <algorithm>

// Internal routines
static void SetupDeadKeyMaps();
static void ClearDeadKeyMaps();

// Data

// Keyboard Layout Data including:
// Standard Character arrays (1-4), Standard Character Arrays (5-12)
// Extended Character Arrays (13-16) & Multi-character arrays
#include "OSK_KB_Data.inc"

// Array to match KLIDs to Virtual keyboard structures
#include "OSK_KLID2VKBDX_Table.inc"

// Arrays of DeadKey data
#include "OSK_DeadKey_Data.inc"

// Arrays to match 'Scancode + Shiftstate' to Multi-character value
//  (2, 3 or 4 characters)
#include "OSK_KB_SCSS2MC2Data.inc"
#include "OSK_KB_SCSS2MC3Data.inc"
#include "OSK_KB_SCSS2MC4Data.inc"

// Arrays to match 'KLID + Scancode + Shiftstate' to Multi-character value
//  (2, 3 or 4 characters)
#include "OSK_KB_KLID2SCSS2MC2Data.inc"
#include "OSK_KB_KLID2SCSS2MC3Data.inc"
#include "OSK_KB_KLID2SCSS2MC4Data.inc"

// This maps KLID to a map holding its deadkey
// and combined characters (if any)
static Map_IKLID2DK2SCSSCC m_mapIKLID2DK2SCSSCC;

// Maps that we keep and give the caller pointers to them
static Map_SCSS2MC  m_mapSCSS2MC2;
static Map_SCSS2MC  m_mapSCSS2MC3;
static Map_SCSS2MC  m_mapSCSS2MC4;
static MMap_DK2SCSSCC m_mmapDK2SCSSCC;

// One map per KLID that maps the deadkey and combined characters
//  for that keyboard: "static Map_IDK2SC m_map_IDK2SC_<KLID>;" e.g.
//  Map_IDK2SC m_map_IDK2SC_00011009; - Canadian Multilingual Standard
#include "OSK_Define_DeadKey_DataMaps.inc"

// Code
BOOL APIENTRY DllMain(HMODULE /* hModule */,
                      DWORD   ul_reason_for_call,
                      LPVOID  /* lpReserved */)
{
  switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
      // A process is loading the DLL.
      SetupDeadKeyMaps();
      break;
    case DLL_THREAD_ATTACH:
      // A process is creating a new thread.
      break;
    case DLL_THREAD_DETACH:
      // A thread exits normally.
      break;
    case DLL_PROCESS_DETACH:
      // A process unloads the DLL.
      // Clear deadkey maps
      ClearDeadKeyMaps();

      // Clear main map
      m_mapIKLID2DK2SCSSCC.clear();

      // Clear user multi-character maps
      m_mapSCSS2MC2.clear();
      m_mapSCSS2MC3.clear();
      m_mapSCSS2MC4.clear();

      // Clear user Deadkey Maps
      m_mmapDK2SCSSCC.clear();
      break;
  }

  return TRUE;
}

OSK_API int OSK_GetVersion()
{
  // Return current version to ensure caller and DLL are in step
  // with regard to calling functions and Implementation Structure
  return VK_DLL_VERSION;
}

OSK_API void OSK_ListKeyboards(UINT &uiKLID, UINT &uiCtrlID)
{
  // Provide list of supported keyboards in this DLL
  // Called with uiKLID = 0 to initialise and return first entry
  // Continues returning information. End of list signalled by returning
  // zero uiKLID.
  static int KLID_index = 0;

  if (uiKLID == 0)
    KLID_index = 0;
  else {
    KLID_index++;
    if (KLID_index >= NUM_KEYBOARDS) {
      uiKLID = 0;
      uiCtrlID = 0;
      return;
    }
  }

  uiKLID = IKLID2VKBD[KLID_index].uiKLID;
  uiCtrlID = IKLID2VKBD[KLID_index].pstIVKBD->uiCtrlID;
}

OSK_API BOOL OSK_GetKeyboardData(UINT uiKLID, st_KBImpl &stKBImpl)
{
  // Return user's keyboard data
  m_mapSCSS2MC2.clear();
  m_mapSCSS2MC3.clear();
  m_mapSCSS2MC4.clear();
  m_mmapDK2SCSSCC.clear();

  stKBImpl.wcMC2 = (wchar_t *)&(wcMC2[0][0]);
  stKBImpl.wcMC3 = (wchar_t *)&(wcMC3[0][0]);
  stKBImpl.wcMC4 = (wchar_t *)&(wcMC4[0][0]);
  stKBImpl.wcDK  = (wchar_t *)&(wcDK[0]);

  bool bFound(false);
  int index(0), ivector(0);
  // Look for this KLID
  for (int i = 0; i < NUM_KEYBOARDS; i++) {
    if (IKLID2VKBD[i].uiKLID == uiKLID) {
      bFound = true;
      index = i;
      break;
    }
  }

  if (!bFound) {
    // Not found - zero calling Implementation structure
    // and return FALSE
    for (int i = 0; i < MAX_ROWS; i++) {
      stKBImpl.stVKBD.stSC2CHAR[i].SC = 0;
      stKBImpl.stVKBD.stSC2CHAR[i].bsDeadKey.reset();
      for (int j = 0; j < 16; j++) {
        stKBImpl.stVKBD.stSC2CHAR[i].wcChar[j] = 0;
      }
    }
    stKBImpl.pmapSCSS2MC2 = NULL;
    stKBImpl.pmapSCSS2MC3 = NULL;
    stKBImpl.pmapSCSS2MC4 = NULL;
    stKBImpl.pmmapDK2SCSSCC = NULL;
    return FALSE;
  }

  st_IVKBD * pstIVKBD = IKLID2VKBD[index].pstIVKBD;
  stKBImpl.stVKBD.uiCtrlID = pstIVKBD->uiCtrlID;
  stKBImpl.stVKBD.numScanCodes = pstIVKBD->numScanCodes;

  // Build keyboard data (expand offsets to actual key characters)
  bool bLCtrl(false), bAltGr(false), bRCtrl(false);
  for (int i = 0; i < (int)pstIVKBD->numScanCodes; i++) {
    stKBImpl.stVKBD.stSC2CHAR[i].SC = pstIVKBD->stISC2CHAR[i].SC;
    std::bitset<16> bsDeadKey(pstIVKBD->stISC2CHAR[i].uiDeadKey);
    stKBImpl.stVKBD.stSC2CHAR[i].bsDeadKey = bsDeadKey;

    for (int j = 0; j < 4; j++) {
      stKBImpl.stVKBD.stSC2CHAR[i].wcChar[j]      = wc_Chars[pstIVKBD->stISC2CHAR[i].uiOffset1][j];
      stKBImpl.stVKBD.stSC2CHAR[i].wcChar[j + 4]  = wc_Chars[pstIVKBD->stISC2CHAR[i].uiOffset2][j];
      stKBImpl.stVKBD.stSC2CHAR[i].wcChar[j + 8]  = wc_Chars[pstIVKBD->stISC2CHAR[i].uiOffset3][j];
      stKBImpl.stVKBD.stSC2CHAR[i].wcChar[j + 12] = wc_Chars[pstIVKBD->stISC2CHAR[i].uiOffset4][j];
    }
    // Check if any valid characters to display with Left Control key
    if (!bLCtrl && pstIVKBD->stISC2CHAR[i].uiOffset2 != 0)
      bLCtrl = true;

    // Check if any valid characters to display with AltGr key
    if (!bAltGr && pstIVKBD->stISC2CHAR[i].uiOffset3 != 0)
      bAltGr = true;

    // Check if any valid characters to display with Right Control key
    if (!bRCtrl && pstIVKBD->stISC2CHAR[i].uiOffset4 != 0)
      bRCtrl = true;
  }

  // Set up which special key combinations are valid
  stKBImpl.stVKBD.bsValidSpecials.reset();
  {
    // First Base character
    bool bSpecials[4] = {false, false, false, false};
    for (int i = 0; i < (int)pstIVKBD->numScanCodes; i++) {
      if (!bSpecials[0] && stKBImpl.stVKBD.stSC2CHAR[i].wcChar[0] != 0)
          bSpecials[0] = true;
      if (!bSpecials[1] && stKBImpl.stVKBD.stSC2CHAR[i].wcChar[1] != 0 &&
          stKBImpl.stVKBD.stSC2CHAR[i].wcChar[1] != stKBImpl.stVKBD.stSC2CHAR[i].wcChar[0])
          bSpecials[1] = true;
      if (!bSpecials[2] && stKBImpl.stVKBD.stSC2CHAR[i].wcChar[2] != 0 &&
          stKBImpl.stVKBD.stSC2CHAR[i].wcChar[2] != stKBImpl.stVKBD.stSC2CHAR[i].wcChar[0])
          bSpecials[2] = true;
      if (!bSpecials[3] && stKBImpl.stVKBD.stSC2CHAR[i].wcChar[3] != 0 &&
          stKBImpl.stVKBD.stSC2CHAR[i].wcChar[3] != stKBImpl.stVKBD.stSC2CHAR[i].wcChar[0])
          bSpecials[3] = true;
    }
    if (bSpecials[0])
      stKBImpl.stVKBD.bsValidSpecials.set(b);
    if (bSpecials[1])
      stKBImpl.stVKBD.bsValidSpecials.set(bC);
    if (bSpecials[2])
      stKBImpl.stVKBD.bsValidSpecials.set(sb);
    if (bSpecials[3])
      stKBImpl.stVKBD.bsValidSpecials.set(sbC);
  }

  if (bLCtrl) {
    // Now Left Control
    bool bSpecials[4] = {false, false, false, false};
    for (int i = 0; i < (int)pstIVKBD->numScanCodes; i++) {
      if (!bSpecials[0] && stKBImpl.stVKBD.stSC2CHAR[i].wcChar[4] != 0)
          bSpecials[0] = true;
      if (!bSpecials[1] && stKBImpl.stVKBD.stSC2CHAR[i].wcChar[5] != 0 &&
          stKBImpl.stVKBD.stSC2CHAR[i].wcChar[5] != stKBImpl.stVKBD.stSC2CHAR[i].wcChar[4])
          bSpecials[1] = true;
      if (!bSpecials[2] && stKBImpl.stVKBD.stSC2CHAR[i].wcChar[6] != 0 &&
          stKBImpl.stVKBD.stSC2CHAR[i].wcChar[6] != stKBImpl.stVKBD.stSC2CHAR[i].wcChar[4])
          bSpecials[2] = true;
      if (!bSpecials[3] && stKBImpl.stVKBD.stSC2CHAR[i].wcChar[7] != 0 &&
          stKBImpl.stVKBD.stSC2CHAR[i].wcChar[7] != stKBImpl.stVKBD.stSC2CHAR[i].wcChar[4])
          bSpecials[3] = true;
    }
    if (bSpecials[0])
      stKBImpl.stVKBD.bsValidSpecials.set(l);
    if (bSpecials[1])
      stKBImpl.stVKBD.bsValidSpecials.set(lC);
    if (bSpecials[2])
      stKBImpl.stVKBD.bsValidSpecials.set(sl);
    if (bSpecials[3])
      stKBImpl.stVKBD.bsValidSpecials.set(slC);
  }

  if (bAltGr) {
    // Now AltGr
    bool bSpecials[4] = {false, false, false, false};
    for (int i = 0; i < (int)pstIVKBD->numScanCodes; i++) {
      if (!bSpecials[0] && stKBImpl.stVKBD.stSC2CHAR[i].wcChar[8] != 0)
          bSpecials[0] = true;
      if (!bSpecials[1] && stKBImpl.stVKBD.stSC2CHAR[i].wcChar[9] != 0 &&
          stKBImpl.stVKBD.stSC2CHAR[i].wcChar[9] != stKBImpl.stVKBD.stSC2CHAR[i].wcChar[8])
          bSpecials[1] = true;
      if (!bSpecials[2] && stKBImpl.stVKBD.stSC2CHAR[i].wcChar[10] != 0 &&
          stKBImpl.stVKBD.stSC2CHAR[i].wcChar[10] != stKBImpl.stVKBD.stSC2CHAR[i].wcChar[8])
          bSpecials[2] = true;
      if (!bSpecials[3] && stKBImpl.stVKBD.stSC2CHAR[i].wcChar[11] != 0 &&
          stKBImpl.stVKBD.stSC2CHAR[i].wcChar[11] != stKBImpl.stVKBD.stSC2CHAR[i].wcChar[8])
          bSpecials[3] = true;
    }
    if (bSpecials[0])
      stKBImpl.stVKBD.bsValidSpecials.set(g);
    if (bSpecials[1])
      stKBImpl.stVKBD.bsValidSpecials.set(gC);
    if (bSpecials[2])
      stKBImpl.stVKBD.bsValidSpecials.set(sg);
    if (bSpecials[3])
      stKBImpl.stVKBD.bsValidSpecials.set(sgC);
  }

  if (bRCtrl) {
    // Lastly Right Control
    bool bSpecials[4] = {false, false, false, false};
    for (int i = 0; i < (int)pstIVKBD->numScanCodes; i++) {
      if (!bSpecials[0] && stKBImpl.stVKBD.stSC2CHAR[i].wcChar[12] != 0)
          bSpecials[0] = true;
      if (!bSpecials[1] && stKBImpl.stVKBD.stSC2CHAR[i].wcChar[13] != 0 &&
          stKBImpl.stVKBD.stSC2CHAR[i].wcChar[13] != stKBImpl.stVKBD.stSC2CHAR[i].wcChar[12])
          bSpecials[1] = true;
      if (!bSpecials[2] && stKBImpl.stVKBD.stSC2CHAR[i].wcChar[14] != 0 &&
          stKBImpl.stVKBD.stSC2CHAR[i].wcChar[14] != stKBImpl.stVKBD.stSC2CHAR[i].wcChar[12])
          bSpecials[2] = true;
      if (!bSpecials[3] && stKBImpl.stVKBD.stSC2CHAR[i].wcChar[15] != 0 &&
          stKBImpl.stVKBD.stSC2CHAR[i].wcChar[15] != stKBImpl.stVKBD.stSC2CHAR[i].wcChar[12])
          bSpecials[3] = true;
    }
    if (bSpecials[0])
      stKBImpl.stVKBD.bsValidSpecials.set(r);
    if (bSpecials[1])
      stKBImpl.stVKBD.bsValidSpecials.set(rC);
    if (bSpecials[2])
      stKBImpl.stVKBD.bsValidSpecials.set(sr);
    if (bSpecials[3])
      stKBImpl.stVKBD.bsValidSpecials.set(srC);
  }

  bFound = false;
  // Now look to see if any 2-character 'characters'
  for (int i = 0; i < NUM_MC2; i++) {
    if (MC2[i].uiKLID == uiKLID) {
      bFound = true;
      ivector = i;
      break;
    }
  }

  if (bFound) {
    // Set up map for this keyboard
    const Vct_ISCSS2MC * pvctISCSS2MC = MC2[ivector].pvctISCSS2MC;
    CIter_Vct_ISCSS2MC citer_mc2;
    for (citer_mc2 = pvctISCSS2MC->begin(); citer_mc2 != pvctISCSS2MC->end(); citer_mc2++) {
      m_mapSCSS2MC2.insert(std::make_pair(citer_mc2->uiSCSS, citer_mc2->uiOffset));
    }
    stKBImpl.pmapSCSS2MC2 = &m_mapSCSS2MC2;
  } else
    stKBImpl.pmapSCSS2MC2 = NULL;

  bFound = false;
  // Now look to see if any 3-character 'characters'
  for (int i = 0; i < NUM_MC3; i++) {
    if (MC3[i].uiKLID == uiKLID) {
      bFound = true;
      ivector = i;
      break;
    }
  }

  if (bFound) {
    // Set up map for this keyboard
    const Vct_ISCSS2MC * pvctISCSS2MC = MC3[ivector].pvctISCSS2MC;
    CIter_Vct_ISCSS2MC citer_mc3;
    for (citer_mc3 = pvctISCSS2MC->begin(); citer_mc3 != pvctISCSS2MC->end(); citer_mc3++) {
      m_mapSCSS2MC3.insert(std::make_pair(citer_mc3->uiSCSS, citer_mc3->uiOffset));
    }
    stKBImpl.pmapSCSS2MC3 = &m_mapSCSS2MC3;
  } else
    stKBImpl.pmapSCSS2MC3 = NULL;

  bFound = false;
  // Now look to see if any 4-character 'characters'
  for (int i = 0; i < NUM_MC4; i++) {
    if (MC4[i].uiKLID == uiKLID) {
      bFound = true;
      ivector = i;
      break;
    }
  }

  if (bFound) {
    // Set up map for this keyboard
    const Vct_ISCSS2MC * pvctISCSS2MC = MC4[ivector].pvctISCSS2MC;
    CIter_Vct_ISCSS2MC citer_mc4;
    for (citer_mc4 = pvctISCSS2MC->begin(); citer_mc4 != pvctISCSS2MC->end(); citer_mc4++) {
      m_mapSCSS2MC4.insert(std::make_pair(citer_mc4->uiSCSS, citer_mc4->uiOffset));
    }
    stKBImpl.pmapSCSS2MC4 = &m_mapSCSS2MC4;
  } else
    stKBImpl.pmapSCSS2MC4 = NULL;

  // Now look to see if this keyboard has any Dead Keys
  Iter_Map_IKLID2DK2SCSSCC iter = m_mapIKLID2DK2SCSSCC.find(uiKLID);

  if (iter != m_mapIKLID2DK2SCSSCC.end()) {
    // Build the DeadKey multi-map for the caller
    // It is a multi-map as a single Dead Key may have a number of
    // associated composite characters - e.g. an accent on multiple vowels
    Map_IDK2SCSSCC *pmapIDK2SCSSCC = iter->second;
    Iter_Map_IDK2SCSSCC iter_xdk;
    for (iter_xdk = pmapIDK2SCSSCC->begin(); iter_xdk != pmapIDK2SCSSCC->end(); iter_xdk++) {
      wchar_t dk = iter_xdk->first;
      const Vct_IDeadkeys * pvIDeadKeys = iter_xdk->second;
      CIter_Vct_IDeadkeys citer_vidks;
      for (citer_vidks = pvIDeadKeys->begin();  citer_vidks != pvIDeadKeys->end(); citer_vidks++) {
        st_SCSSCC stSCSSCC;
        stSCSSCC.SC = citer_vidks->SC;
        stSCSSCC.SS = citer_vidks->SS;
        stSCSSCC.wcCC = wcDK[citer_vidks->uiDKOffset];
        m_mmapDK2SCSSCC.insert(std::make_pair(dk, stSCSSCC));
      }
    }
    stKBImpl.pmmapDK2SCSSCC = &m_mmapDK2SCSSCC;
  } else
    stKBImpl.pmmapDK2SCSSCC = NULL;

  return TRUE;
}

// Internal routines
static void SetupDeadKeyMaps()
{
  // Include code to insert data into the maps
#include "OSK_Insert_DeadKey_DataMaps.inc"
}

// Functor for for_each
struct Clear_Map {
  void operator()(std::pair<unsigned int, Map_IDK2SCSSCC *> p)
  {
    p.second->clear();
  }
};

static void ClearDeadKeyMaps()
{
  // Clear the deadkey_combined character maps
  for_each(m_mapIKLID2DK2SCSSCC.begin(), m_mapIKLID2DK2SCSSCC.end(), Clear_Map());
}
