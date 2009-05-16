/*
* Copyright (c) 2009 David Kelvin <c-273@users.sourceforge.net>.
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
static Map_IKLID2DK2SCSS m_mapIKLID2DK2SCSS;

// Maps that we keep and give the caller pointers to them
static Map_SCSS2Char m_mapSCSS2Char2;
static Map_SCSS2Char m_mapSCSS2Char3;
static Map_SCSS2Char m_mapSCSS2Char4;
static MMap_DK2SCSS  m_mmapDK2SCSS;

// One map per KLID that maps the deadkey and combined characters
//  for that keyboard: "static Map_IDK2SC m_map_IDK2SC_<KLID>;" e.g.
//  Map_IDK2SC m_map_IDK2SC_00011009; - Canadian Multilingual Standard
#include "OSK_Define_DeadKey_DataMaps.inc"

// Code
BOOL APIENTRY DllMain(HMODULE hModule,
                      DWORD   ul_reason_for_call,
                      LPVOID  lpReserved)
{
  switch (ul_reason_for_call)	{
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
      m_mapIKLID2DK2SCSS.clear();

      // Clear user multi-character maps
      m_mapSCSS2Char2.clear();
      m_mapSCSS2Char3.clear();
      m_mapSCSS2Char4.clear();

      // Clear user Deadkey Maps
      m_mmapDK2SCSS.clear();
      break;
  }

  return TRUE;
}

OSK_API int OSK_GetVersion()
{
  return PWS_VERSION;
}

OSK_API void OSK_ListKeyboards(UINT &uiKLID, UINT &uiCtrlID)
{
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
  m_mapSCSS2Char2.clear();
  m_mapSCSS2Char3.clear();
  m_mapSCSS2Char4.clear();
  m_mmapDK2SCSS.clear();

  stKBImpl.wcMC2 = (wchar_t *)&(wcMC2[0][0]);
  stKBImpl.wcMC3 = (wchar_t *)&(wcMC3[0][0]);
  stKBImpl.wcMC4 = (wchar_t *)&(wcMC4[0][0]);
  stKBImpl.wcDK  = (wchar_t *)&(wcDK[0]);

  bool bFound(false);
  int index(0), ivector(0);
  for (int i = 0; i < NUM_KEYBOARDS; i++) {
    if (IKLID2VKBD[i].uiKLID == uiKLID) {
      bFound = true;
      index = i;
      break;
    }
  }

  if (!bFound) {
    for (int i = 0; i < MAX_ROWS; i++) {
      stKBImpl.stVKBD.stSC2CHAR[i].SC = 0;
      stKBImpl.stVKBD.stSC2CHAR[i].bsDeadKey = 0UL;
      for (int j = 0; j < 16; j++) {
        stKBImpl.stVKBD.stSC2CHAR[i].wcChar[j] = 0;
      }
    }
    stKBImpl.pmapSCSS2Char2 = NULL;
    stKBImpl.pmapSCSS2Char3 = NULL;
    stKBImpl.pmapSCSS2Char4 = NULL;
    stKBImpl.pmmapDK2SCSS   = NULL;
    return FALSE;
  }

  st_IVKBD * pstIVKBD = IKLID2VKBD[index].pstIVKBD;
  stKBImpl.stVKBD.uiCtrlID = pstIVKBD->uiCtrlID;
  stKBImpl.stVKBD.numScanCodes = pstIVKBD->numScanCodes;

  bool bLCtrl(false), bAltGr(false), bRCtrl(false);
  for (int i = 0; i < (int)pstIVKBD->numScanCodes; i++) {
    stKBImpl.stVKBD.stSC2CHAR[i].SC = pstIVKBD->stISC2CHAR[i].SC;
    stKBImpl.stVKBD.stSC2CHAR[i].bsDeadKey = (unsigned long)pstIVKBD->stISC2CHAR[i].uiDeadKey;

    for (int j = 0; j < 4; j++) {
      stKBImpl.stVKBD.stSC2CHAR[i].wcChar[j]      = wc_Chars[pstIVKBD->stISC2CHAR[i].uiOffset1][j];
      stKBImpl.stVKBD.stSC2CHAR[i].wcChar[j + 4]  = wc_Chars[pstIVKBD->stISC2CHAR[i].uiOffset2][j];
      stKBImpl.stVKBD.stSC2CHAR[i].wcChar[j + 8]  = wc_Chars[pstIVKBD->stISC2CHAR[i].uiOffset3][j];
      stKBImpl.stVKBD.stSC2CHAR[i].wcChar[j + 12] = wc_Chars[pstIVKBD->stISC2CHAR[i].uiOffset4][j];
    }
    if (!bLCtrl && pstIVKBD->stISC2CHAR[i].uiOffset2 != 0)
      bLCtrl = true;

    if (!bAltGr && pstIVKBD->stISC2CHAR[i].uiOffset3 != 0)
      bAltGr = true;

    if (!bRCtrl && pstIVKBD->stISC2CHAR[i].uiOffset4 != 0)
      bRCtrl = true;
  }

  // Set up which special key combinations are valid
  stKBImpl.stVKBD.bsValidSpecials.reset();
  {
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
  for (int i = 0; i < NUM_MC2; i++) {
    if (MC2[i].uiKLID == uiKLID) {
      bFound = true;
      ivector = i;
      break;
    }
  }

  if (bFound) {
    const Vct_ISCSS2MC * pvctISCSS2MC = MC2[ivector].pvctISCSS2MC;
    CIter_Vct_ISCSS2MC citer_mc2;
    for (citer_mc2 = pvctISCSS2MC->begin(); citer_mc2 != pvctISCSS2MC->end(); citer_mc2++) {
      m_mapSCSS2Char2.insert(std::make_pair(citer_mc2->uiSCSS, citer_mc2->uiOffset));
    }
    stKBImpl.pmapSCSS2Char2 = &m_mapSCSS2Char2;
  } else
    stKBImpl.pmapSCSS2Char2 = NULL;

  bFound = false;
  for (int i = 0; i < NUM_MC3; i++) {
    if (MC3[i].uiKLID == uiKLID) {
      bFound = true;
      ivector = i;
      break;
    }
  }

  if (bFound) {
    const Vct_ISCSS2MC * pvctISCSS2MC = MC3[ivector].pvctISCSS2MC;
    CIter_Vct_ISCSS2MC citer_mc3;
    for (citer_mc3 = pvctISCSS2MC->begin(); citer_mc3 != pvctISCSS2MC->end(); citer_mc3++) {
      m_mapSCSS2Char3.insert(std::make_pair(citer_mc3->uiSCSS, citer_mc3->uiOffset));
    }
    stKBImpl.pmapSCSS2Char3 = &m_mapSCSS2Char3;
  } else
    stKBImpl.pmapSCSS2Char3 = NULL;

  bFound = false;
  for (int i = 0; i < NUM_MC4; i++) {
    if (MC4[i].uiKLID == uiKLID) {
      bFound = true;
      ivector = i;
      break;
    }
  }

  if (bFound) {
    const Vct_ISCSS2MC * pvctISCSS2MC = MC4[ivector].pvctISCSS2MC;
    CIter_Vct_ISCSS2MC citer_mc4;
    for (citer_mc4 = pvctISCSS2MC->begin(); citer_mc4 != pvctISCSS2MC->end(); citer_mc4++) {
      m_mapSCSS2Char4.insert(std::make_pair(citer_mc4->uiSCSS, citer_mc4->uiOffset));
    }
    stKBImpl.pmapSCSS2Char4 = &m_mapSCSS2Char4;
  } else
    stKBImpl.pmapSCSS2Char4 = NULL;

  Iter_Map_IKLID2DK2SCSS iter = m_mapIKLID2DK2SCSS.find(uiKLID);

  if (iter != m_mapIKLID2DK2SCSS.end()) {
    // Build the DeadKey multi-map for the caller
    Map_IDK2SCSS *pmapIDK2SCSS = iter->second;
    Iter_Map_IDK2SCSS iter_xdk;
    for (iter_xdk = pmapIDK2SCSS->begin(); iter_xdk != pmapIDK2SCSS->end(); iter_xdk++) {
      wchar_t dk = iter_xdk->first;
      const Vct_IDeadkeys * pvIDeadKeys = iter_xdk->second;
      CIter_Vct_IDeadkeys citer_vidks;
      for (citer_vidks = pvIDeadKeys->begin();  citer_vidks != pvIDeadKeys->end(); citer_vidks++) {
        st_SCSS2DKChar stSCSS2DKChar;
        stSCSS2DKChar.SC = citer_vidks->SC;
        stSCSS2DKChar.SS = citer_vidks->SS;
        stSCSS2DKChar.wcDKchar = wcDK[citer_vidks->uiDKOffset];
        m_mmapDK2SCSS.insert(std::make_pair(dk, stSCSS2DKChar));
      }
    }
    stKBImpl.pmmapDK2SCSS   = &m_mmapDK2SCSS;
  } else
    stKBImpl.pmmapDK2SCSS   = NULL;

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
  void operator()(std::pair<unsigned int, Map_IDK2SCSS *> p)
  {
    p.second->clear();
  }
};

static void ClearDeadKeyMaps()
{
  // Clear the deadkey_combined character maps
  for_each(m_mapIKLID2DK2SCSS.begin(), m_mapIKLID2DK2SCSS.end(), Clear_Map());
}
