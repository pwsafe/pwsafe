/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
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

#include "PWSAuxParse.h"
#include "PWSprefs.h"
#include "corelib.h"
#include "ItemData.h"

#include "os/dir.h"
#include "os/file.h"
#include "os/pws_tchar.h"

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
StringX PWSAuxParse::GetExpandedString(const StringX &sxRun_Command,
                                       const StringX &sxCurrentDB, 
                                       CItemData *ci, bool &bAutoType,
                                       StringX &sxAutotype, stringT &serrmsg, 
                                       StringX::size_type &st_column)
{
  std::vector<st_RunCommandTokens> v_rctokens;
  std::vector<st_RunCommandTokens>::iterator rc_iter;
  StringX sxretval(_T(""));
  stringT spath, sdrive, sdir, sfname, sextn;
  stringT sdbdir;

  UINT uierr = ParseRunCommand(sxRun_Command, v_rctokens, 
                               bAutoType, sxAutotype, 
                               serrmsg, st_column);

  // if called with NULL ci, then we just parse to validate
  if (uierr > 0 || ci == NULL || sxCurrentDB.empty()) {
    v_rctokens.clear();
    return sxretval;
  }

  // derive current db's directory and basename:
  spath = sxCurrentDB.c_str();
  pws_os::splitpath(spath, sdrive, sdir, sfname, sextn);
  sdbdir = pws_os::makepath(sdrive, sdir, _T(""), _T(""));

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
      sxretval += ci->GetGroup();
    } else
    if (st_rctoken.sxname == _T("G") || st_rctoken.sxname == _T("GROUP")) {
      StringX sxg = ci->GetGroup();
      StringX::size_type st_index;
      st_index = sxg.rfind(_T("."));
      if (st_index != StringX::npos) {
        sxg = sxg.substr(st_index + 1);
      }
      sxretval += sxg;
    } else
    if (st_rctoken.sxname == _T("t") || st_rctoken.sxname == _T("title")) {
      sxretval += ci->GetTitle();
    } else
    if (st_rctoken.sxname == _T("u") || st_rctoken.sxname == _T("user")) {
      sxretval += ci->GetUser();
    } else
    if (st_rctoken.sxname == _T("p") || st_rctoken.sxname == _T("password")) {
      sxretval += ci->GetPassword();
    } else
    if (st_rctoken.sxname == _T("a") || st_rctoken.sxname == _T("autotype")) {
      // Do nothing - autotype variable handled elsewhere
    } else
    if (st_rctoken.sxname == _T("url")) {
      sxretval += ci->GetURL();
    } else
    if (st_rctoken.sxname == _T("n") || st_rctoken.sxname == _T("notes")) {
      StringX sxnotes = ci->GetNotes();

      if (st_rctoken.index == 0) {
        sxretval += sxnotes;
      } else {
        std::vector<StringX> vsxnotes_lines;
        ParseNotes(sxnotes, vsxnotes_lines);
        // If line there - use it; otherwise ignore it
        if (st_rctoken.index > 0 && st_rctoken.index <= (int)vsxnotes_lines.size()) {
          sxretval += vsxnotes_lines[st_rctoken.index - 1];
        } else
        if (st_rctoken.index < 0 &&
            abs(st_rctoken.index) <= (int)vsxnotes_lines.size()) {
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

StringX PWSAuxParse::GetAutoTypeString(const StringX &sxInAutoCmd,
                                       const StringX &sxgroup,
                                       const StringX &sxtitle,
                                       const StringX &sxuser,
                                       const StringX &sxpwd,
                                       const StringX &sxnotes)
{
  // If empty, try the database default
  StringX sxAutoCmd(sxInAutoCmd);
  if (sxAutoCmd.empty()) {
    sxAutoCmd = PWSprefs::GetInstance()->
              GetPref(PWSprefs::DefaultAutotypeString);

    // If still empty, take this default
    if (sxAutoCmd.empty()) {
      // checking for user and password for default settings
      if (!sxpwd.empty()){
        if (!sxuser.empty())
          sxAutoCmd = DEFAULT_AUTOTYPE;
        else
          sxAutoCmd = _T("\\p\\n");
      }
    }
  }

  StringX sxNotes(sxnotes);
  std::vector<StringX> vsxnotes_lines;
  StringX::size_type st_index;

    // No recursive substitution (e.g. \p or \u), although '\t' will be replaced by a tab
  if (!sxnotes.empty()) {
    // Use \n and \r to tokenise this line
    StringX::size_type st_start(0), st_end(0);
    const StringX sxdelim = _T("\r\n");
    StringX sxline;
    while (st_end != StringX::npos) {
      st_end = sxNotes.find(sxdelim, st_start);
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
      st_start = ((st_end > (StringX::npos - sxdelim.size())) ?
                         StringX::npos : st_end + sxdelim.size());
    }
    // Now change '\n' to '\r' in the complete notes field
    st_index = 0;
    for (;;) {
      st_index = sxNotes.find(_T("\r\n"), st_index);
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

  StringX sxtmp(_T(""));
  TCHAR curChar;
  const int N = sxAutoCmd.length();

  for (int n = 0; n < N; n++){
    curChar = sxAutoCmd[n];
    if (curChar == TCHAR('\\')) {
      n++;
      if (n < N)
        curChar = sxAutoCmd[n];

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
        case TCHAR('g'):
          sxtmp += sxgroup;
          break;
        case TCHAR('i'):
          sxtmp += sxtitle;
          break;
        case TCHAR('u'):
          sxtmp += sxuser;
          break;
        case TCHAR('p'):
          sxtmp += sxpwd;
          break;
        case TCHAR('o'):
        {
          if (n == (N - 1)) {
            // This was the last character - send the lot!
            sxtmp += sxNotes;
            break;
          }
          int line_number(0);
          int gNumIts(0);
          for (n++; n < N && (gNumIts < 3); ++gNumIts, n++) {
            if (_istdigit(sxAutoCmd[n])) {
              line_number *= 10;
              line_number += (sxAutoCmd[n] - TCHAR('0'));
            } else
              break; // for loop
          }
          if (line_number == 0) {
            // Send the lot
            sxtmp += sxnotes;
          } else if (line_number <= (int)vsxnotes_lines.size()) {
            // User specifies a too big a line number - ignore the lot
            sxtmp += vsxnotes_lines[line_number - 1];
          }

          // Backup the extra character that delimited the \oNNN string
          n--;
          break; // case 'o'
        }
        case TCHAR('d'):
          // Ignore delay - treat it as just a string!
          sxtmp += _T("\\d");
          break; // case 'd'
        case TCHAR('b'): // backspace!
          sxtmp += TCHAR('\b');
          break;
        // Ignore explicit control characters
        case TCHAR('a'): // bell (can't hear it during testing!)
        case TCHAR('v'): // vertical tab
        case TCHAR('f'): // form feed
        case TCHAR('e'): // escape
        case TCHAR('x'): // hex digits (\xNN)
          break;
        // Ignore any others!
        // '\cC', '\uXXXX', '\OOO', '\<any other charatcer not recognised above>'
        default:
          break;
      }
    } else
      sxtmp += curChar;
  }

  vsxnotes_lines.clear();
  return sxtmp;
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
  std::vector<st_RunCommandTokens>::iterator rc_iter;
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
      st_rctoken.is_variable  = st_startpos == 0 ? false : true;
      st_rctoken.has_brackets = false;
      v_rctokens.push_back(st_rctoken);
      v_pos.push_back(st_startpos);
    }
    st_startpos = st_next + 1; // too complex for for statement
  } // tokenization for loop

  // Check if escaped - ending character of previous token == '\'
  // Make sure this '\' is not escaped itself!
  for (size_t st_idx = v_rctokens.size() - 1; st_idx > 0 ; st_idx--) {
    st_RunCommandTokens &st_rctoken = v_rctokens[st_idx - 1];
    StringX::size_type name_len = st_rctoken.sxname.length();
    if (name_len == 0 || (name_len >= 2 &&
            st_rctoken.sxname.substr(name_len - 2, 2).compare(_T("\\\\")) == 0))
      continue;

    if (st_rctoken.sxname.substr(name_len - 1, 1).compare(_T("\\")) == 0) {
      st_rctoken.sxname = st_rctoken.sxname.substr(0, name_len - 1) + 
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
  sxAutoType.clear();
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
        // Check if anythnig left in this text - none -> delete
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
