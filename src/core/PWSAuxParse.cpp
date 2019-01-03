/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
 * Implementation of utility functions that parse the two small
 * 'languages' used for 'autotype' and 'run' command processing.
 */

#include <vector>
#include <algorithm>

#include "PWSAuxParse.h"
#include "PWHistory.h"
#include "PWSprefs.h"
#include "core.h"
#include "ItemData.h"
#include "PWScore.h"

#include "os/dir.h"
#include "os/file.h"
#include "os/pws_tchar.h"
#include "os/KeySend.h"
#include "os/sleep.h"

// Internal structures, forward declarations

struct st_RunCommandTokens {
  StringX sxname;        // Variable name
  StringX sxindex;       // If index present, exact string user entered
  int index;             // If index present, numerical value
  bool is_variable;      // Variable or text
  bool has_brackets;     // Variable enclosed in curly brackets
};

static UINT ParseRunCommand(const StringX &sxInputString,
                            std::vector<st_RunCommandTokens> &v_rctokens,
                            bool &bDoAutoType, StringX &sxAutoType,
                            stringT &serrmsg, StringX::size_type &st_column);

static UINT ProcessIndex(const StringX &sxIndex, int &var_index,
                         StringX::size_type &st_column);

static void ParseNotes(StringX &sxNotes,
                       std::vector<StringX> &vsxnotes_lines)
{
  if (!sxNotes.empty()) {
    // Use \n and \r to tokenise this line
    StringX::size_type st_start(0), st_end(0);
    const StringX sxdelim = _T("\r\n");
    StringX sxline;
    StringX::size_type st_index;
    while (st_end != StringX::npos) {
      st_end = sxNotes.find(sxdelim, st_start);
      sxline = (sxNotes.substr(st_start, 
                   (st_end == StringX::npos) ? StringX::npos : st_end - st_start));
      st_index = 0;
      // Remove all tabs - \t
      for (;;) {
        st_index = sxline.find(_T("\\t"), st_index);
        if (st_index == StringX::npos)
          break;
        sxline.replace(st_index, 2, _T(""));
        st_index += 1;
      }
      vsxnotes_lines.push_back(sxline);
      st_start = ((st_end > (StringX::npos - sxdelim.size()))
                          ? StringX::npos : st_end + sxdelim.size());
    }
  }
}

//-----------------------------------------------------------------
// Externally visible functions
//-----------------------------------------------------------------

bool PWSAuxParse::GetEffectiveValues(const CItemData *pci, const CItemData *pbci,
                                     StringX &sx_group, StringX &sx_title, StringX &sx_user,
                                     StringX &sx_pswd, StringX &sx_lastpswd,
                                     StringX &sx_notes, StringX &sx_url,
                                     StringX &sx_email, StringX &sx_autotype, StringX &sx_runcmd)
{
  // The one place to get the values needed for AutoType & RunCmd based on entry type

  if (pci->IsDependent()) {
    ASSERT(pbci != nullptr);
    if (pbci == nullptr)
      return false;
  }

  sx_group    = pci->GetEffectiveFieldValue(CItem::GROUP, pbci);
  sx_title    = pci->GetEffectiveFieldValue(CItem::TITLE, pbci);
  sx_user     = pci->GetEffectiveFieldValue(CItem::USER, pbci);
  sx_pswd     = pci->GetEffectiveFieldValue(CItem::PASSWORD, pbci);
  sx_lastpswd = ::GetPreviousPassword(pci->GetEffectiveFieldValue(CItem::PWHIST, pbci));
  sx_notes    = pci->GetEffectiveFieldValue(CItem::NOTES, pbci);
  sx_url      = pci->GetEffectiveFieldValue(CItem::URL, pbci);
  sx_email    = pci->GetEffectiveFieldValue(CItem::EMAIL, pbci);
  sx_autotype = pci->GetEffectiveFieldValue(CItem::AUTOTYPE, pbci);
  sx_runcmd   = pci->GetEffectiveFieldValue(CItem::RUNCMD, pbci);

  return true;
}

StringX PWSAuxParse::GetExpandedString(const StringX &sxRun_Command,
                                       const StringX &sxCurrentDB, 
                                       const CItemData *pci, const CItemData *pbci,
                                       bool &bAutoType,
                                       StringX &sxAutotype, stringT &serrmsg, 
                                       StringX::size_type &st_column,
                                       bool &bURLSpecial)
{
  std::vector<st_RunCommandTokens> v_rctokens;
  std::vector<st_RunCommandTokens>::iterator rc_iter;
  StringX sxretval(_T(""));
  stringT spath, sdrive, sdir, sfname, sextn;
  stringT sdbdir;
  bURLSpecial = false;

  UINT uierr = ParseRunCommand(sxRun_Command, v_rctokens, 
                               bAutoType, sxAutotype, 
                               serrmsg, st_column);

  // if called with nullptr ci, then we just parse to validate
  if (uierr > 0 || pci == nullptr) {
    v_rctokens.clear();
    return sxretval;
  }

  // derive current db's directory and basename:
  spath = sxCurrentDB.c_str();
  pws_os::splitpath(spath, sdrive, sdir, sfname, sextn);
  sdbdir = pws_os::makepath(sdrive, sdir, _T(""), _T(""));

  StringX sx_group, sx_title, sx_user, sx_pswd, sx_lastpswd, sx_notes, sx_url, sx_email, sx_autotype;

  // GetEffectiveFieldValue() encapsulates what we take from where depending in the entry type (alias, shortcut, etc.)
  sx_group    = pci->GetEffectiveFieldValue(CItem::GROUP, pbci);
  sx_title    = pci->GetEffectiveFieldValue(CItem::TITLE, pbci);
  sx_user     = pci->GetEffectiveFieldValue(CItem::USER, pbci);
  sx_pswd     = pci->GetEffectiveFieldValue(CItem::PASSWORD, pbci);
  sx_lastpswd = ::GetPreviousPassword(pci->GetEffectiveFieldValue(CItem::PWHIST, pbci));
  sx_notes    = pci->GetEffectiveFieldValue(CItem::NOTES, pbci);
  sx_url      = pci->GetEffectiveFieldValue(CItem::URL, pbci);
  sx_email    = pci->GetEffectiveFieldValue(CItem::EMAIL, pbci);
  sx_autotype = pci->GetEffectiveFieldValue(CItem::AUTOTYPE, pbci);

  for (rc_iter = v_rctokens.begin(); rc_iter < v_rctokens.end(); rc_iter++) {
    st_RunCommandTokens &st_rctoken = *rc_iter;

    if (!st_rctoken.is_variable) {
      sxretval += st_rctoken.sxname.c_str();
      continue;
    }

    if (st_rctoken.sxname == _T("appdir")) {
      sxretval += pws_os::getexecdir().c_str();
    } else
    if (st_rctoken.sxname == _T("dbdir")) {
      sxretval += sdbdir.c_str();
    } else
    if (st_rctoken.sxname == _T("fulldb")) {
      sxretval += spath.c_str();
    } else
    if (st_rctoken.sxname == _T("dbname")) {
      sxretval += sfname.c_str();
    } else
    if (st_rctoken.sxname == _T("dbextn")) {
      sxretval += sextn.c_str();
    } else
    if (st_rctoken.sxname == _T("g") || st_rctoken.sxname == _T("group")) {
      sxretval += sx_group;
    } else
    if (st_rctoken.sxname == _T("G") || st_rctoken.sxname == _T("GROUP")) {
      StringX sxg = sx_group;
      StringX::size_type st_index;
      st_index = sxg.rfind(_T('.'));
      if (st_index != StringX::npos) {
        sxg = sxg.substr(st_index + 1);
      }
      sxretval += sxg;
    } else
    if (st_rctoken.sxname == _T("t") || st_rctoken.sxname == _T("title")) {
      sxretval += sx_title;
    } else
    if (st_rctoken.sxname == _T("u") || st_rctoken.sxname == _T("user")) {
      sxretval += sx_user;
    } else
    if (st_rctoken.sxname == _T("p") || st_rctoken.sxname == _T("password")) {
      sxretval += sx_pswd;
    } else
      if (st_rctoken.sxname == _T("e") || st_rctoken.sxname == _T("email")) {
      sxretval += sx_email;
    } else
    if (st_rctoken.sxname == _T("a") || st_rctoken.sxname == _T("autotype")) {
      // Do nothing - autotype variable handled elsewhere
    } else
    if (st_rctoken.sxname == _T("url")) {
      StringX sxurl = sx_url;
      if (sxurl.length() > 0) {
        // Remove 'Browse to' specifics
        StringX::size_type ipos;
        ipos = sxurl.find(_T("[alt]"));
        if (ipos != StringX::npos) {
          bURLSpecial = true;
          sxurl.erase(ipos, 5);
        }
        ipos = sxurl.find(_T("[ssh]"));
        if (ipos != StringX::npos) {
          bURLSpecial = true;
          sxurl.erase(ipos, 5);
        }
        ipos = sxurl.find(_T("{alt}"));
        if (ipos != StringX::npos) {
          bURLSpecial = true;
          sxurl.erase(ipos, 5);
        }
        ipos = sxurl.find(_T("[autotype]"));
        if (ipos != StringX::npos) {
          sxurl.erase(ipos, 10);
        }
        ipos = sxurl.find(_T("[xa]"));
        if (ipos != StringX::npos) {
          sxurl.erase(ipos, 4);
        }
        sxretval += sxurl;
      }
    } else
    if (st_rctoken.sxname == _T("n") || st_rctoken.sxname == _T("notes")) {
      if (st_rctoken.index == 0) {
        sxretval += sx_notes;
      } else {
        std::vector<StringX> vsxnotes_lines;
        ParseNotes(sx_notes, vsxnotes_lines);
        // If line there - use it; otherwise ignore it
        if (st_rctoken.index > 0 && st_rctoken.index <= static_cast<int>(vsxnotes_lines.size())) {
          sxretval += vsxnotes_lines[st_rctoken.index - 1];
        } else
        if (st_rctoken.index < 0 &&
            abs(st_rctoken.index) <= static_cast<int>(vsxnotes_lines.size())) {
          sxretval += vsxnotes_lines[vsxnotes_lines.size() + st_rctoken.index];
        }
      }
    } else {
      // Unknown variable name - rebuild it
      sxretval += _T("$");
      if (st_rctoken.has_brackets)
        sxretval += _T("(");
      sxretval += st_rctoken.sxname.c_str();
      if (st_rctoken.index != 0)
        sxretval += st_rctoken.sxindex.c_str();
      if (st_rctoken.has_brackets)
        sxretval += _T(")");
    }
  }
  v_rctokens.clear();
  return sxretval;
}

static bool GetSpecialCommand(const StringX &sx_autotype, size_t &n, WORD &wVK,
                              bool &bAlt, bool &bCtrl, bool &bShift)
{
  // Currently support special characters (note codes are NOT case sensitive)
  // In addition the following prefixes are supported
  // Alt = '!'; Ctrl = '^'; Shift = '+' such that {+Tab} == Shift+Tab
  /*
    {Enter} Enter key
    {Up}    Up-arrow key
    {Down}  Down-arrow down key
    {Left}  Left-arrow key
    {Right} Right-arrow key
    {Home}  Home key
    {End}   End key
    {PgUp}  Page-up key
    {PgDn}  Page-down key
    {Tab}   Tab key
    {Space} Space key
  */
  
  bAlt = bCtrl = bShift = false;
 
  TCHAR curChar = sx_autotype[n];
  if (curChar != TCHAR('{')) {
    ASSERT(0);
    return false;
  }

  // Find ending curly bracket
  StringX::size_type iEndBracket = sx_autotype.find_first_of(TCHAR('}'), n);
  if (iEndBracket == StringX::npos)
    return false;

  StringX sxCode = sx_autotype.substr(n + 1, iEndBracket - n - 1);
  ToUpper(sxCode);

  // Detect leading Alt, Ctrl or Shift
  StringX sxfound;
  std::size_t found = sxCode.find_first_of(_T("ABCDEFGHIJKLMNOPQRSTUVWXYZ"));

  if (found == StringX::npos)
    return false;

  if (found != 0) {
    // We have special characters
    for (size_t iSpecial = 0; iSpecial < found; iSpecial++) {
      if (sxCode.substr(0, 1) == _T("!") && sxCode.length() > 1) {
        bAlt = true;
        sxCode.erase(0, 1);
        continue;
      }
      if (sxCode.substr(0, 1) == _T("^") && sxCode.length() > 1) {
        bCtrl = true;
        sxCode.erase(0, 1);
        continue;
      }
      if (sxCode.substr(0, 1) == _T("+") && sxCode.length() > 1) {
        bShift = true;
        sxCode.erase(0, 1);
        continue;
      }
    }
  }

  if (!CKeySend::LookupVirtualKey(sxCode, wVK))
    return false;
  n = iEndBracket - 1;
  return true;
}

StringX PWSAuxParse::GetAutoTypeString(const StringX &sx_in_autotype,
                                       const StringX &sx_group,
                                       const StringX &sx_title,
                                       const StringX &sx_user,
                                       const StringX &sx_pwd,
                                       const StringX &sx_lastpwd,
                                       const StringX &sx_notes,
                                       const StringX &sx_url,
                                       const StringX &sx_email,
                                       std::vector<size_t> &vactionverboffsets)
{
  StringX sxtmp(_T(""));
  StringX sxNotes(sx_notes);
  TCHAR curChar;
  StringX sx_autotype(sx_in_autotype);
  StringX::size_type st_index;
  std::vector<StringX> vsxnotes_lines;

  vactionverboffsets.clear();

  // If empty, try the database default
  if (sx_autotype.empty()) {
    sx_autotype = PWSprefs::GetInstance()->
              GetPref(PWSprefs::DefaultAutotypeString);

    // If still empty, take this default
    if (sx_autotype.empty()) {
      // checking for user and password for default settings
      if (!sx_pwd.empty()){
        if (!sx_user.empty())
          sx_autotype = DEFAULT_AUTOTYPE;
        else
          sx_autotype = _T("\\p\\n");
      }
    }
  }

  // No recursive substitution (e.g. \p or \u), although '\t' will be replaced by a tab
  if (!sx_notes.empty()) {
    // Use \n and \r to tokenise this line
    StringX::size_type st_start(0), st_end(0);
    const StringX sxdelim = _T("\r\n");
    StringX sxline;
    while (st_end != StringX::npos) {
      st_end = sxNotes.find_first_of(sxdelim, st_start);
      sxline = (sxNotes.substr(st_start, (st_end == StringX::npos) ? 
                              StringX::npos : st_end - st_start));
      st_index = 0;
      for (;;) {
        st_index = sxline.find(_T("\\t"), st_index);
        if (st_index == StringX::npos)
          break;
        sxline.replace(st_index, 2, _T("\t"));
        st_index += 1;
      }
      vsxnotes_lines.push_back(sxline);
      // If we just hit a "\r\n", move past it.  Or else, it is a "\r" without
      // a following "\n" or a "\n", so just move past one single char
      if (st_end != StringX::npos) {
        st_start = st_end + (sxNotes.compare(st_end, 2, sxdelim) == 0 ? 2 : 1);
        if (st_start >= sxNotes.length())
          break;
      }
    }
    // Now change '\n' to '\r' in the complete notes field
    st_index = 0;
    for (;;) {
      st_index = sxNotes.find(sxdelim, st_index);
      if (st_index == StringX::npos)
        break;
      sxNotes.replace(st_index, 2, _T("\r"));
      st_index += 1;
    }
    st_index = 0;
    for (;;) {
      st_index = sxNotes.find(_T("\\t"), st_index);
      if (st_index == StringX::npos)
        break;
      sxNotes.replace(st_index, 2, _T("\t"));
      st_index += 1;
    }
  }

  const size_t N = sx_autotype.length();
  const StringX sxZeroes = _T("000");
  unsigned int gNumIts;

  for (size_t n = 0; n < N; n++){
    curChar = sx_autotype[n];
    if (curChar == TCHAR('\\')) {
      n++;
      if (n < N)
        curChar = sx_autotype[n];

      switch (curChar){
        case TCHAR('\\'):
          sxtmp += TCHAR('\\');
          break;
        case TCHAR('n'):
        case TCHAR('r'):
          sxtmp += TCHAR('\r');
          break;
        case TCHAR('t'):
          sxtmp += TCHAR('\t');
          break;
        case TCHAR('s'):
          sxtmp += TCHAR('\v');
          break;
        case TCHAR('g'):
          sxtmp += sx_group;
          break;
        case TCHAR('i'):
          sxtmp += sx_title;
          break;
        case TCHAR('u'):
          sxtmp += sx_user;
          break;
        case TCHAR('p'):
          sxtmp += sx_pwd;
          break;
        case TCHAR('q'):
          sxtmp += sx_lastpwd;
          break;
        case TCHAR('l'):
          sxtmp += sx_url;
          break;
        case TCHAR('m'):
          sxtmp += sx_email;
          break;

        case TCHAR('o'):
        {
          StringX sxN;
          bool bSendNotes(true);
          if (n == (N - 1)) {
            // This was the last character - send the lot!
            sxN = sxNotes;
          } else {
            size_t line_number(0);
            gNumIts = 0;
            for (n++; n < N && (gNumIts < 3); ++gNumIts, n++) {
              if (_istdigit(sx_autotype[n])) {
                line_number *= 10;
                line_number += (sx_autotype[n] - TCHAR('0'));
              } else
                break; // for loop
            }

            if (line_number == 0) {
              // Send the lot
              sxN = sxNotes;
            } else {
              if (line_number <= vsxnotes_lines.size()) {
                // Only copy if user has specified a valid Notes line number

                sxN = vsxnotes_lines[line_number - 1];
              } else {
                bSendNotes = false;
              }
            }
            // Backup the extra character that delimited the \oNNN string
            n--;
          }

          if (bSendNotes) {
            // As per help '\n' & '\r\n' replaced by '\r'
            Replace(sxN, StringX(_T("\r\n")), StringX(_T("\r")));
            Replace(sxN, _T('\n'), _T('\r'));
            sxtmp += sxN;
          }
          break; // case 'o'
        }

        // Action Verbs:
        // These are the only ones processed specially by the UI as they involve
        // actions it performs whilst doing the key sending.
        // Copy them to output string unchanged.
        case TCHAR('b'):  // backspace!
        case TCHAR('z'):  // Use older method
        case TCHAR('c'):  // select-all
        case TCHAR('j'):  // modifier emulation on
        case TCHAR('k'):  // modifier emulation off
          vactionverboffsets.push_back(sxtmp.length());
          sxtmp += _T("\\");
          sxtmp += curChar;
          break; // case 'b' & 'z'

        case TCHAR('#'):  // Use older method but allow on/off
          sxtmp += _T("\\");
          sxtmp += curChar;
          break; // case '#'

        case TCHAR('d'):  // Delay
        case TCHAR('w'):  // Wait milli-seconds
        case TCHAR('W'):  // Wait seconds
        {
          // Need to ensure that the field length is 3, even if it wasn't
          vactionverboffsets.push_back(sxtmp.length());
          sxtmp += _T("\\");
          sxtmp += curChar;

          gNumIts = 0;
          size_t i = n;
          for (i++; i < N && (gNumIts < 3); ++gNumIts, i++) {
            if (!_istdigit(sx_autotype[i]))
              break;
          }
          // Insert sufficient zeroes to ensure field is 3 characters long
          sxtmp += sxZeroes.substr(0, 3 - gNumIts);
          break; // case 'd', 'w' & 'W'
        }

        case TCHAR('{'):
        {
          // Special processing of particular commands - could be expanded later
          sxtmp += _T("\\");
          sxtmp += curChar;
          WORD wVK;
          bool bAlt, bCtrl, bShift;
          if (GetSpecialCommand(sx_autotype, n, wVK, bAlt, bCtrl, bShift)) {
            if (bAlt) sxtmp += _T('!');
            if (bCtrl) sxtmp += _T('^');
            if (bShift) sxtmp += _T('+');
            sxtmp += wVK;
          }
          break;
        }

        // Also copy explicit control characters to output string unchanged.
        case TCHAR('a'): // bell (can't hear it during testing!)
        case TCHAR('v'): // vertical tab
        case TCHAR('f'): // form feed
        case TCHAR('e'): // escape
        case TCHAR('x'): // hex digits (\xNN)
        // and any others we have forgotten!
        // '\cC', '\uXXXX', '\OOO', '\<any other character not recognised above>'
        default:
          sxtmp += L'\\';
          sxtmp += curChar;
          break;
      }
    } else
      sxtmp += curChar;
  }

  vsxnotes_lines.clear();
  return sxtmp;
}

StringX PWSAuxParse::GetAutoTypeString(const CItemData &ci,
                                       const PWScore &core,
                                       std::vector<size_t> &vactionverboffsets)
{
  const CItemData *pbci(nullptr);
  StringX sx_group, sx_title, sx_user, sx_pswd, sx_lastpswd, sx_notes, sx_url, sx_email, sx_autotype, sx_runcmd;

  if (ci.IsDependent()) {
    pbci = core.GetBaseEntry(&ci);
  }

  GetEffectiveValues(&ci, pbci, sx_group, sx_title, sx_user,
                     sx_pswd, sx_lastpswd,
                     sx_notes, sx_url, sx_email, sx_autotype, sx_runcmd);

  // If empty, try the database default
  if (sx_autotype.empty()) {
    sx_autotype = PWSprefs::GetInstance()->
              GetPref(PWSprefs::DefaultAutotypeString);

    // If still empty, take this default
    if (sx_autotype.empty()) {
      // checking for user and password for default settings
      if (!sx_pswd.empty()){
        if (!sx_user.empty())
          sx_autotype = DEFAULT_AUTOTYPE;
        else
          sx_autotype = _T("\\p\\n");
      }
    }
  }
  return PWSAuxParse::GetAutoTypeString(sx_autotype, sx_group,
                                        sx_title, sx_user, sx_pswd, sx_lastpswd,
                                        sx_notes, sx_url, sx_email,
                                        vactionverboffsets);
}

void PWSAuxParse::SendAutoTypeString(const StringX &sx_autotype,
                                     const std::vector<size_t> &vactionverboffsets)
{
  // Accepts string and vector indicating location(s) of command(s)
  // as returned by GetAutoTypeString()
  // processes the later whilst sending the former
  // Commands parsed here involve time (\d, \w, \W) or old-method override (\z,  \# & \-#)
  StringX sxtmp(_T(""));
  StringX sxautotype(sx_autotype);
  wchar_t curChar;
 
  bool bForceOldMethod(false), bForceOldMethod2(false), bCapsLock(false);
 
  StringX::size_type st_index = sxautotype.find(_T("\\z"));

  while (st_index != StringX::npos) {
    if (std::find(vactionverboffsets.begin(), vactionverboffsets.end(), st_index) !=
        vactionverboffsets.end()) {
      bForceOldMethod = true;
      break;
    }
    st_index = sxautotype.find(_T("\\z"), st_index + 1);
  }

  const size_t N = sxautotype.length();
  const unsigned defaultDelay = PWSprefs::GetInstance()->GetPref(PWSprefs::DefaultAutotypeDelay);
  CKeySend ks(bForceOldMethod, defaultDelay);

  // Turn off CAPSLOCK
  if (ks.isCapsLocked()) {
    bCapsLock = true;
    ks.SetCapsLock(false);
  }

  ks.ResetKeyboardState();

  // Stop Keyboard/Mouse Input
  pws_os::Trace(_T("PWSAuxParse::SendAutoTypeString - BlockInput set\n"));
  ks.BlockInput(true);

  // Karl Student's suggestion, to ensure focus set correctly on minimize.
  pws_os::sleep_ms(1000);

  size_t gNumIts;
  for (size_t n = 0; n < N; n++){
    curChar = sxautotype[n];
    if (curChar == _T('\\')) {
      n++;
      if (n < N)
        curChar = sxautotype[n];

      // Only need to process fields left in there by PWSAuxParse::GetAutoTypeString
      // for later processing
      switch (curChar) {
        case L'd':
        case L'w':
        case L'W':
        { 
           if (std::find(vactionverboffsets.begin(), vactionverboffsets.end(), n - 1) ==
               vactionverboffsets.end()) {
             // Not in the list of found action verbs - treat as-is
             sxtmp += L'\\';
             sxtmp += curChar;
             break;
           }

          /*
           'd' means value is in milli-seconds, max value = 0.999s
           and is the delay between sending each character

           'w' means value is in milli-seconds, max value = 0.999s
           'W' means value is in seconds, max value = 16m 39s
           and is the wait time before sending the next character.
           Use of this field does not change any current delay value.

           User needs to understand that PasswordSafe will be unresponsive
           for the whole of this wait period!
          */

          // Delay is going to change - send what we have with old delay
          ks.SendString(sxtmp);

          // start collecting new delay
          sxtmp.clear();

          unsigned int newdelay = 0;
          gNumIts = 0;
          for (n++; n < N && (gNumIts < 3); ++gNumIts, n++) {
            if (_istdigit(sxautotype[n])) {
              newdelay *= 10;
              newdelay += (sxautotype[n] - L'0');
            } else
              break; // for loop
          }

          n--;
          // Either set new character delay time or wait specified time
          if (curChar == L'd')
            ks.SetAndDelay(newdelay);
          else
            pws_os::sleep_ms(newdelay * (curChar == L'w' ? 1 : 1000));

          break; // case 'd', 'w' & 'W'
        }

        case L'z':
          if (std::find(vactionverboffsets.begin(), vactionverboffsets.end(), n - 1) ==
              vactionverboffsets.end()) {
            // Not in the list of found action verbs - treat as-is
            sxtmp += L'\\';
            sxtmp += curChar;
          }
          break;

        case L'#':
          // This toggles using the OldMethod as long as \z not specified ANYWHERE
          // in the Autotype string
          if (bForceOldMethod) {
            // User has already used '\z' - ignore this \# - treat as-is
            sxtmp += L'\\';
            sxtmp += curChar;
          } else {
            // Send what we have
            if (sxtmp.length() > 0) {
              ks.SendString(sxtmp);
              sxtmp.clear();
            }
            // Toggle
            bForceOldMethod2 = !bForceOldMethod2;
            ks.SetOldSendMethod(bForceOldMethod2);
          }
          break;

        case L'b':
          if (std::find(vactionverboffsets.begin(), vactionverboffsets.end(), n - 1) ==
              vactionverboffsets.end()) {
            // Not in the list of found action verbs - treat as-is
            sxtmp += L'\\';
            sxtmp += curChar;
          } else {
            sxtmp += L'\b';
          }
          break;

        case L'{':
        {
          // Send what we have
          if (!sxtmp.empty()) {
            ks.SendString(sxtmp);
            sxtmp.clear();
          }

          // Get this field
          StringX sxSpecial = sxautotype.substr(n + 1);
          StringX::size_type iEndBracket = sxSpecial.find(_T('}'));
          if (iEndBracket == StringX::npos) { // malformed - no '}'
            sxtmp += L'\\';
            sxtmp += curChar;
            break;
          }
          sxSpecial.erase(iEndBracket);
          StringX::size_type iModifiersLength = sxSpecial.find_last_of(_T("!^+"));

          bool bAlt(false), bCtrl(false), bShift(false);
          if (iModifiersLength != StringX::npos) {
            iModifiersLength++;
            for (size_t i = 0; i < iModifiersLength; i++) {
              if (sxSpecial[0] == _T('!')) {
                bAlt = true;
                sxSpecial.erase(0, 1);
                continue;
              }
              if (sxSpecial[0] == _T('^')) {
                bCtrl = true;
                sxSpecial.erase(0, 1);
                continue;
              }
              if (sxSpecial[0] == _T('+')) {
                bShift = true;
                sxSpecial.erase(0, 1);
                continue;
              }
            }
          } else { // no modifier
            iModifiersLength = 0;
          }

          // Get Virtual Key code
          WORD wVK = sxautotype[n + iModifiersLength + 1];
          ks.SendVirtualKey(wVK, bAlt, bCtrl, bShift);

          // Skip over modifiers, VK and closing bracket
          n += iEndBracket + 1;
          break;
        }
        default:
          sxtmp += L'\\';
          sxtmp += curChar;
          break;
      }
    } else
      sxtmp += curChar;
  }

  ks.SendString(sxtmp);

  // If we turned off CAPSLOCK, put it back
  if (bCapsLock)
    ks.SetCapsLock(true);

  pws_os::sleep_ms(100);

  // Reset Keyboard/Mouse Input
  pws_os::Trace(_T("PWSAuxParse::SendAutoTypeString - BlockInput reset\n"));
  ks.BlockInput(false);
}

//-----------------------------------------------------------------
// Internal functions
//-----------------------------------------------------------------
static UINT ParseRunCommand(const StringX &sxInputString,
                            std::vector<st_RunCommandTokens> &v_rctokens,
                            bool &bDoAutoType, StringX &sxAutoType,
                            stringT &serrmsg, StringX::size_type &st_column)
{
  // tokenize into separate elements
  std::vector<size_t> v_pos;
  StringX::iterator str_Iter;
  st_RunCommandTokens st_rctoken;
  size_t st_num_quotes(0);

  UINT uierr(0);
  int var_index(0);

  const stringT alphanum =
    _T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");

  if (sxInputString.empty()) {
  // String is empty!
    uierr = IDSC_EXS_INPUTEMPTY;
    goto exit;
  }
  for (StringX::size_type l = 0; l < sxInputString.length(); l++) {
    if (sxInputString[l] == _T('"'))
      st_num_quotes++;
  }

  if (st_num_quotes % 2 != 0) {
    st_column = sxInputString.find(_T('"'));
    // Unmatched quotes
    uierr = IDSC_EXS_UNMATCHEDQUOTES;
    goto exit;
  }

  // tokenize into separate elements using $ as the field separator
  for (StringX::size_type st_startpos = 0;
       st_startpos < sxInputString.size();
       /* st_startpos advanced in body */) {
    StringX::size_type st_next = sxInputString.find(_T('$'), st_startpos);
    if (st_next == StringX::npos)
      st_next = sxInputString.size();
    if (st_next > 0) {
      st_rctoken.sxname = sxInputString.substr(st_startpos, st_next - st_startpos);
      st_rctoken.sxindex = _T("");
      st_rctoken.index = 0;
      st_rctoken.is_variable  = st_startpos != 0;
      st_rctoken.has_brackets = false;
      v_rctokens.push_back(st_rctoken);
      v_pos.push_back(st_startpos);
    }
    st_startpos = st_next + 1; // too complex for for statement
  } // tokenization for loop

  // Check if escaped - ending character of previous token == '\'
  // Make sure this '\' is not escaped itself!
  for (size_t st_idx = v_rctokens.size() - 1; st_idx > 0 ; st_idx--) {
    st_RunCommandTokens &st_rctokens = v_rctokens[st_idx - 1];
    StringX::size_type name_len = st_rctokens.sxname.length();
    if (name_len == 0 || (name_len >= 2 &&
                          st_rctokens.sxname.substr(name_len - 2, 2).compare(_T("\\\\")) == 0)) {
      st_rctokens.sxname.erase(name_len - 2, 1);
      continue;
    }

    if (st_rctokens.sxname.substr(name_len - 1, 1).compare(_T("\\")) == 0) {
      st_rctokens.sxname = st_rctokens.sxname.substr(0, name_len - 1) + 
                         _T("$") + v_rctokens[st_idx].sxname;
      v_rctokens.erase(v_rctokens.begin() + st_idx);
    }
  }

  // Check if variable enclosed in curly brackets
  for (size_t st_idx = 0; st_idx < v_rctokens.size(); st_idx++) {
    if (v_rctokens[st_idx].sxname.length() == 0)
      continue;

    str_Iter = v_rctokens[st_idx].sxname.begin();
    // Does it start with a curly bracket?
    if (*str_Iter == _T('{')) {
      v_rctokens[st_idx].has_brackets = true;
      StringX sxvar, sxnonvar, sxindex(_T(""));
      // Yes - Find end curly bracket
      StringX::size_type st_end_cb = v_rctokens[st_idx].sxname.find(_T('}'));
      if (st_end_cb == StringX::npos) {
        st_column = v_pos[st_idx] + v_rctokens[st_idx].sxname.length();
        // Missing end curly bracket
        uierr = IDSC_EXS_MISSINGCURLYBKT;
        goto exit;
      }
      // Now see if there is an Index here
      StringX::size_type st_start_sb = v_rctokens[st_idx].sxname.find(_T('['));
      if (st_start_sb != StringX::npos) {
        // Yes  - find end square bracket
        if (st_start_sb > st_end_cb) {
          // Square backet after end of variable
          sxvar = v_rctokens[st_idx].sxname.substr(1, st_end_cb - 1);
          sxnonvar = v_rctokens[st_idx].sxname.substr(st_end_cb + 1);
          v_rctokens[st_idx].sxname = sxvar;
          if (sxnonvar.length() > 0) {
            st_rctoken.sxname = sxnonvar;
            st_rctoken.sxindex = _T("");
            st_rctoken.index = 0;
            st_rctoken.is_variable = false;
            st_rctoken.has_brackets = false;
            v_rctokens.insert(v_rctokens.begin() + st_idx + 1, st_rctoken);
            v_pos.insert(v_pos.begin() + st_idx + 1, v_pos[st_idx] + st_end_cb);
          }
          continue;
        }
        StringX::size_type st_end_sb = v_rctokens[st_idx].sxname.find(_T(']'), st_start_sb);
        if (st_end_sb == StringX::npos) {
          st_column = v_pos[st_idx] + 1;
          // Missing end square bracket
          uierr = IDSC_EXS_MISSINGSQUAREBKT;
          goto exit;
        }
        // The end-curly backet must immediately follow the end-square bracket
        if (st_end_cb != st_end_sb + 1) {
          st_column = v_pos[st_idx] + st_end_sb + 1;
          // Characters between ']' and ')'
          uierr = IDSC_EXS_INVALIDBRACKETS;
          goto exit;
        }
        sxindex = v_rctokens[st_idx].sxname.substr(st_start_sb + 1, st_end_sb - st_start_sb - 1);
        v_rctokens[st_idx].sxindex = sxindex;
        // Now check index
        uierr = ProcessIndex(sxindex, var_index, st_column);
        if (uierr > 0) {
          st_column += v_pos[st_idx];
          goto exit;
        }

        v_rctokens[st_idx].index = var_index;
        sxvar = v_rctokens[st_idx].sxname.substr(1, st_start_sb - 1);
        sxnonvar = v_rctokens[st_idx].sxname.substr(st_end_cb + 1);
      } else {
        // No square bracket
        // Split current token into 'variable' and 'non-variable' parts
        sxvar = v_rctokens[st_idx].sxname.substr(1, st_end_cb - 1);
        sxnonvar = v_rctokens[st_idx].sxname.substr(st_end_cb + 1);
      }
      v_rctokens[st_idx].sxname = sxvar;
      if (sxnonvar.length() > 0) {
        st_rctoken.sxname = sxnonvar;
        st_rctoken.sxindex = _T("");
        st_rctoken.index = 0;
        st_rctoken.is_variable = false;
        st_rctoken.has_brackets = false;
        v_rctokens.insert(v_rctokens.begin() + st_idx + 1, st_rctoken);
        v_pos.insert(v_pos.begin() + st_idx + 1, v_pos[st_idx] + st_end_cb);
      }
    }
  }

  // Now use rules of variables to get the real variable
  for (size_t st_idx = 0; st_idx < v_rctokens.size(); st_idx++) {
    if (!v_rctokens[st_idx].is_variable)
      continue;

    if (v_rctokens[st_idx].sxname.length() == 0) {
      st_column = v_pos[st_idx];
      // Variable name is empty
      uierr = IDSC_EXS_VARNAMEEMPTY;
      goto exit;
    }

    str_Iter = v_rctokens[st_idx].sxname.begin();
    if (!isalpha(*str_Iter)) {
      st_column = v_pos[st_idx];
      // First character of variable is not alphabetic
      uierr = IDSC_EXS_FIRSTNOTALPHA;
      goto exit;
    }
    StringX::size_type st_next = v_rctokens[st_idx].sxname.find_first_not_of(alphanum.c_str());
    if (st_next != StringX::npos) {
      // Split current token into 'variable' and 'non-variable' parts
      StringX sxvar = v_rctokens[st_idx].sxname.substr(0, st_next);
      StringX sxnonvar = v_rctokens[st_idx].sxname.substr(st_next);
      v_rctokens[st_idx].sxname = sxvar;
      // Before saving non-variable part - check if it is an Index e.g. var[i]
      if (sxnonvar.c_str()[0] == _T('[')) {
        // Find ending square bracket
        StringX::size_type st_end_sb = sxnonvar.find(_T(']'));
        if (st_end_sb == StringX::npos) {
          st_column = v_pos[st_idx] + sxvar.length() + 2;
          // Missing end square bracket
          uierr = IDSC_EXS_MISSINGSQUAREBKT;
          goto exit;
        }
        StringX sxindex = sxnonvar.substr(1, st_end_sb - 1);
        v_rctokens[st_idx].sxindex = sxindex;
        // Now check index
        uierr = ProcessIndex(sxindex, var_index, st_column);
        if (uierr > 0) {
          st_column += v_pos[st_idx] + sxvar.length();
          goto exit;
        }

        v_rctokens[st_idx].index = var_index;
        sxnonvar = sxnonvar.substr(st_end_sb + 1);
      } else {
        // Not a square bracket
        if (v_rctokens[st_idx].has_brackets) {
          st_column = v_pos[st_idx] + st_next + 1;
          // Variable must be alphanumeric
          uierr = IDSC_EXS_VARNAMEINVALID;
          goto exit;
        }
      }
      if (!sxnonvar.empty()) {
        st_rctoken.sxname = sxnonvar;
        st_rctoken.sxindex = _T("");
        st_rctoken.index = 0;
        st_rctoken.is_variable = false;
        st_rctoken.has_brackets = false;
        v_rctokens.insert(v_rctokens.begin() + st_idx + 1, st_rctoken);
        v_pos.insert(v_pos.begin() + st_idx + 1, v_pos[st_idx] + st_next);
      }
    }
  }

  // Special Autotype processing
  bDoAutoType = false;
  sxAutoType = _T("");
  for (size_t st_idx = 0; st_idx < v_rctokens.size(); st_idx++) {
    if (!v_rctokens[st_idx].is_variable)
      continue;

    // Is it a autotype variable?
    if (v_rctokens[st_idx].sxname == _T("a") ||
        v_rctokens[st_idx].sxname == _T("autotype")) {
      bDoAutoType = true;
      // Is the next token text and starts with '('?
      if (st_idx + 1 < v_rctokens.size() &&
          !v_rctokens[st_idx + 1].is_variable &&
          v_rctokens[st_idx + 1].sxname.c_str()[0] == _T('(')) {
        // Find ending round bracket
        StringX sx_autotype = v_rctokens[st_idx + 1].sxname;
        StringX::size_type st_end_rb = sx_autotype.find(_T(')'));
        if (st_end_rb == StringX::npos) {
          st_column = v_pos[st_idx + 1] + sx_autotype.length() + 2;
          // Missing end round bracket
          uierr = IDSC_EXS_MISSINGROUNDBKT;
          goto exit;
        }
        sxAutoType = sx_autotype.substr(1, st_end_rb - 1);
        v_rctokens[st_idx + 1].sxname = sx_autotype.substr(st_end_rb + 1);
        // Check if anything left in this text - none -> delete
        if (v_rctokens[st_idx + 1].sxname.length() == 0)
          v_rctokens.erase(v_rctokens.begin() + st_idx + 1);
        // Now delete Autotype variable
        v_rctokens.erase(v_rctokens.begin() + st_idx);
        break;
      }
    }
  }

exit:
  if (uierr != 0)
    LoadAString(serrmsg, uierr);
  else
    serrmsg = _T("");

  if (uierr > 0) {
    v_rctokens.clear();
  }
  v_pos.clear();
  return uierr;
}

static UINT ProcessIndex(const StringX &sx_Index, int &var_index, 
                         StringX::size_type &st_column)
{
  const stringT num = _T("0123456789");

  StringX sxindex(sx_Index);
  UINT uierr(0);
  bool negative_vindex(false);
  StringX::size_type st_nondigit(0);

  st_column = 0;

  if (sxindex.length() == 0) {
    st_column = 2;
    // Index is invalid or missing
    uierr = IDSC_EXS_INVALIDINDEX;
    goto exit;
  }

  if (sxindex[0] == _T('-') || sxindex[0] == _T('+')) {
    if (sxindex.length() == 1) {
      st_column = 2;
      // Index is invalid or missing
      uierr = IDSC_EXS_INVALIDINDEX;
      goto exit;
    }
    negative_vindex = (sxindex[0] == _T('-'));
    sxindex = sxindex.substr(1);
  }

  st_nondigit = sxindex.find_first_not_of(num.c_str());
  if (st_nondigit != StringX::npos) {
    st_column = st_nondigit + 1;
    // Index is not numeric
    uierr = IDSC_EXS_INDEXNOTNUMERIC;
    goto exit;
  }

  var_index = _tstoi(sxindex.c_str());
  if (negative_vindex && var_index == 0) { // i.e. [-0]
    st_column = 2;
    // Index is invalid or missing
    uierr = IDSC_EXS_INVALIDINDEX;
    goto exit;
  }

  if (negative_vindex)
    var_index *= (-1);

exit:
  return uierr;
}
