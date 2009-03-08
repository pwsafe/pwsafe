/*
* Copyright (c) 2003-2009 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
 * Implementation of utility functions that parse the two small
 * 'languages' used for 'autotype' and 'execute' command processing.
 */

#include <vector>

#include "PWSAuxParse.h"
#include "PWSprefs.h"
#include "corelib.h"
#include "ItemData.h"

#include "os/dir.h"
#include "os/file.h"

// Internal structures, forward declarations

struct st_ExecuteStringTokens {
  StringX sxname;        // Variable name
  StringX sxindex;       // If index present, exact string user entered
  int index;             // If index present, numerical value
  bool is_variable;      // Variable or text
  bool has_brackets;     // Variable enclosed in curly brackets
};

static UINT ParseExecuteString(const StringX &sxInputString,
                               std::vector<st_ExecuteStringTokens> &v_estokens,
                               bool &bDoAutoType, StringX &sxAutoType,
                               stringT &serrmsg, StringX::size_type &st_column);

static UINT ProcessIndex(const StringX &sxIndex, int &var_index,
                         StringX::size_type &st_column);

//-----------------------------------------------------------------
// Externally visible functions
//-----------------------------------------------------------------
StringX PWSAuxParse::GetExpandedString(const StringX &sxExecute_String,
                                       const StringX &sxCurrentDB, 
                                       CItemData *ci, bool &bAutoType,
                                       StringX &sxAutotype, stringT &serrmsg, 
                                       StringX::size_type &st_column)
{
  std::vector<st_ExecuteStringTokens> v_estokens;
  std::vector<st_ExecuteStringTokens>::iterator es_iter;
  std::vector<StringX> vsxnotes_lines;
  StringX sxnotes, sxretval(_T(""));
  stringT spath, sdrive, sdir, sfname, sextn;
  stringT sdbdir;

  ASSERT(ci != NULL);
  if (ci == NULL || sxCurrentDB.empty())
    return sxretval;

  UINT uierr = ParseExecuteString(sxExecute_String, v_estokens, 
                                  bAutoType, sxAutotype, 
                                  serrmsg, st_column);

  if (uierr > 0) {
    v_estokens.clear();
    return sxretval;
  }

  // derive current db's directory and basename:
  spath = sxCurrentDB.c_str();
  pws_os::splitpath(spath, sdrive, sdir, sfname, sextn);
  sdbdir = pws_os::makepath(sdrive, sdir, _T(""), _T(""));

  sxnotes = ci->GetNotes();
  if (!ci->IsNotesEmpty()) {
    // Use \n and \r to tokenise this line
    StringX::size_type st_start(0), st_end(0);
    const StringX sxdelim = _T("\r\n");
    StringX sxline;
    StringX::size_type st_index;
    while (st_end != StringX::npos) {
      st_end = sxnotes.find(sxdelim, st_start);
      sxline = (sxnotes.substr(st_start, 
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

  for (es_iter = v_estokens.begin(); es_iter < v_estokens.end(); es_iter++) {
    st_ExecuteStringTokens &st_estoken = *es_iter;

    if (!st_estoken.is_variable) {
      sxretval += st_estoken.sxname.c_str();
      continue;
    }

    if (st_estoken.sxname == _T("appdir")) {
      sxretval += pws_os::getexecdir().c_str();
    } else
    if (st_estoken.sxname == _T("dbdir")) {
      sxretval += sdbdir.c_str();
    } else
    if (st_estoken.sxname == _T("fulldb")) {
      sxretval += spath.c_str();
    } else
    if (st_estoken.sxname == _T("dbname")) {
      sxretval += sfname.c_str();
    } else
    if (st_estoken.sxname == _T("dbextn")) {
      sxretval += sextn.c_str();
    } else
    if (st_estoken.sxname == _T("g") || st_estoken.sxname == _T("group")) {
      sxretval += ci->GetGroup();
    } else
    if (st_estoken.sxname == _T("G") || st_estoken.sxname == _T("GROUP")) {
      StringX sxg = ci->GetGroup();
      StringX::size_type st_index;
      st_index = sxg.rfind(_T("."));
      if (st_index != StringX::npos) {
        sxg = sxg.substr(st_index + 1);
      }
      sxretval += sxg;
    } else
    if (st_estoken.sxname == _T("t") || st_estoken.sxname == _T("title")) {
      sxretval += ci->GetTitle();
    } else
    if (st_estoken.sxname == _T("u") || st_estoken.sxname == _T("user")) {
      sxretval += ci->GetUser();
    } else
    if (st_estoken.sxname == _T("p") || st_estoken.sxname == _T("password")) {
      sxretval += ci->GetPassword();
    } else
    if (st_estoken.sxname == _T("a") || st_estoken.sxname == _T("autotype")) {
      // Do nothing - autotype variable handled elsewhere
    } else
    if (st_estoken.sxname == _T("url")) {
      sxretval += ci->GetURL();
    } else
    if (st_estoken.sxname == _T("n") || st_estoken.sxname == _T("notes")) {
      if (st_estoken.index == 0) {
        sxretval += sxnotes;
      } else {
        // If line there - use it; otherwise ignore it
        if (st_estoken.index > 0 && st_estoken.index <= (int)vsxnotes_lines.size()) {
          sxretval += vsxnotes_lines[st_estoken.index - 1];
        } else
        if (st_estoken.index < 0 && abs(st_estoken.index) <= (int)vsxnotes_lines.size()) {
          sxretval += vsxnotes_lines[vsxnotes_lines.size() + st_estoken.index];
        }
      }
    } else {
      // Unknown variable name - rebuild it
      sxretval += _T("$");
      if (st_estoken.has_brackets)
        sxretval += _T("(");
      sxretval += st_estoken.sxname.c_str();
      if (st_estoken.index != 0)
        sxretval += st_estoken.sxindex.c_str();
      if (st_estoken.has_brackets)
        sxretval += _T(")");
    }
  }
  v_estokens.clear();
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
          sxtmp += TCHAR("\\d");
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
static UINT ParseExecuteString(const StringX &sxInputString,
                               std::vector<st_ExecuteStringTokens> &v_estokens,
                               bool &bDoAutoType, StringX &sxAutoType,
                               stringT &serrmsg, StringX::size_type &st_column)
{
  // tokenize into separate elements
  std::vector<st_ExecuteStringTokens>::iterator es_iter;
  std::vector<size_t> v_pos;
  StringX::iterator str_Iter;
  st_ExecuteStringTokens st_estoken;

  UINT uierr(0);
  int var_index(0);

  const stringT alphanum =
    _T("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789");

  if (sxInputString.empty()) {
  // String is empty!
    uierr = IDSC_EXS_INPUTEMPTY;
    goto exit;
  }

  size_t st_num_quotes(0);
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
      st_estoken.sxname = sxInputString.substr(st_startpos, st_next - st_startpos);
      st_estoken.sxindex = _T("");
      st_estoken.index = 0;
      st_estoken.is_variable  = st_startpos == 0 ? false : true;
      st_estoken.has_brackets = false;
      v_estokens.push_back(st_estoken);
      v_pos.push_back(st_startpos);
    }
    st_startpos = st_next + 1; // too complex for for statement
  } // tokenization for loop

  // Check if escaped - ending character of previous token == '\'
  // Make sure this '\' is not escaped itself!
  for (size_t st_idx = v_estokens.size() - 1; st_idx > 0 ; st_idx--) {
    st_ExecuteStringTokens &st_estoken = v_estokens[st_idx - 1];
    StringX::size_type name_len = st_estoken.sxname.length();
    if (name_len == 0 || (name_len >= 2 &&
            st_estoken.sxname.substr(name_len - 2, 2).compare(_T("\\\\")) == 0))
      continue;

    if (st_estoken.sxname.substr(name_len - 1, 1).compare(_T("\\")) == 0) {
      st_estoken.sxname = st_estoken.sxname.substr(0, name_len - 1) + 
                         _T("$") + v_estokens[st_idx].sxname;
      v_estokens.erase(v_estokens.begin() + st_idx);
    }
  }

  // Check if variable enclosed in curly brackets
  for (size_t st_idx = 0; st_idx < v_estokens.size(); st_idx++) {
    if (v_estokens[st_idx].sxname.length() == 0)
      continue;

    str_Iter = v_estokens[st_idx].sxname.begin();
    // Does it start with a curly bracket?
    if (*str_Iter == _T('{')) {
      v_estokens[st_idx].has_brackets = true;
      StringX sxvar, sxnonvar, sxindex(_T(""));
      // Yes - Find end curly bracket
      StringX::size_type st_end_cb = v_estokens[st_idx].sxname.find(_T('}'));
      if (st_end_cb == StringX::npos) {
        st_column = v_pos[st_idx] + v_estokens[st_idx].sxname.length();
        // Missing end curly bracket
        uierr = IDSC_EXS_MISSINGCURLYBKT;
        goto exit;
      }
      // Now see if there is an Index here
      StringX::size_type st_start_sb = v_estokens[st_idx].sxname.find(_T('['));
      if (st_start_sb != StringX::npos) {
        // Yes  - find end square bracket
        if (st_start_sb > st_end_cb) {
          // Square backet after end of variable
          sxvar = v_estokens[st_idx].sxname.substr(1, st_end_cb - 1);
          sxnonvar = v_estokens[st_idx].sxname.substr(st_end_cb + 1);
          v_estokens[st_idx].sxname = sxvar;
          if (sxnonvar.length() > 0) {
            st_estoken.sxname = sxnonvar;
            st_estoken.sxindex = _T("");
            st_estoken.index = 0;
            st_estoken.is_variable = false;
            st_estoken.has_brackets = false;
            v_estokens.insert(v_estokens.begin() + st_idx + 1, st_estoken);
            v_pos.insert(v_pos.begin() + st_idx + 1, v_pos[st_idx] + st_end_cb);
          }
          continue;
        }
        StringX::size_type st_end_sb = v_estokens[st_idx].sxname.find(_T(']'), st_start_sb);
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
        sxindex = v_estokens[st_idx].sxname.substr(st_start_sb + 1, st_end_sb - st_start_sb - 1);
        v_estokens[st_idx].sxindex = sxindex;
        // Now check index
        uierr = ProcessIndex(sxindex, var_index, st_column);
        if (uierr > 0) {
          st_column += v_pos[st_idx];
          goto exit;
        }

        v_estokens[st_idx].index = var_index;
        sxvar = v_estokens[st_idx].sxname.substr(1, st_start_sb - 1);
        sxnonvar = v_estokens[st_idx].sxname.substr(st_end_cb + 1);
      } else {
        // No square bracket
        // Split current token into 'variable' and 'non-variable' parts
        sxvar = v_estokens[st_idx].sxname.substr(1, st_end_cb - 1);
        sxnonvar = v_estokens[st_idx].sxname.substr(st_end_cb + 1);
      }
      v_estokens[st_idx].sxname = sxvar;
      if (sxnonvar.length() > 0) {
        st_estoken.sxname = sxnonvar;
        st_estoken.sxindex = _T("");
        st_estoken.index = 0;
        st_estoken.is_variable = false;
        st_estoken.has_brackets = false;
        v_estokens.insert(v_estokens.begin() + st_idx + 1, st_estoken);
        v_pos.insert(v_pos.begin() + st_idx + 1, v_pos[st_idx] + st_end_cb);
      }
    }
  }

  // Now use rules of variables to get the real variable
  for (size_t st_idx = 0; st_idx < v_estokens.size(); st_idx++) {
    if (!v_estokens[st_idx].is_variable)
      continue;

    if (v_estokens[st_idx].sxname.length() == 0) {
      st_column = v_pos[st_idx];
      // Variable name is empty
      uierr = IDSC_EXS_VARNAMEEMPTY;
      goto exit;
    }

    str_Iter = v_estokens[st_idx].sxname.begin();
    if (!isalpha(*str_Iter)) {
      st_column = v_pos[st_idx];
      // First character of variable is not alphabetic
      uierr = IDSC_EXS_FIRSTNOTALPHA;
      goto exit;
    }
    StringX::size_type st_next = v_estokens[st_idx].sxname.find_first_not_of(alphanum.c_str());
    if (st_next != StringX::npos) {
      // Split current token into 'variable' and 'non-variable' parts
      StringX sxvar = v_estokens[st_idx].sxname.substr(0, st_next);
      StringX sxnonvar = v_estokens[st_idx].sxname.substr(st_next);
      v_estokens[st_idx].sxname = sxvar;
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
        v_estokens[st_idx].sxindex = sxindex;
        // Now check index
        uierr = ProcessIndex(sxindex, var_index, st_column);
        if (uierr > 0) {
          st_column += v_pos[st_idx] + sxvar.length();
          goto exit;
        }

        v_estokens[st_idx].index = var_index;
        sxnonvar = sxnonvar.substr(st_end_sb + 1);
      } else {
        // Not a square bracket
        if (v_estokens[st_idx].has_brackets) {
          st_column = v_pos[st_idx] + st_next + 1;
          // Variable must be alphanumeric
          uierr = IDSC_EXS_VARNAMEINVALID;
          goto exit;
        }
      }
      if (!sxnonvar.empty()) {
        st_estoken.sxname = sxnonvar;
        st_estoken.sxindex = _T("");
        st_estoken.index = 0;
        st_estoken.is_variable = false;
        st_estoken.has_brackets = false;
        v_estokens.insert(v_estokens.begin() + st_idx + 1, st_estoken);
        v_pos.insert(v_pos.begin() + st_idx + 1, v_pos[st_idx] + st_next);
      }
    }
  }

  // Special Autotype processing
  bDoAutoType = false;
  sxAutoType.clear();
  for (size_t st_idx = 0; st_idx < v_estokens.size(); st_idx++) {
    if (!v_estokens[st_idx].is_variable)
      continue;

    // Is it a autotype variable?
    if (v_estokens[st_idx].sxname == _T("a") ||
        v_estokens[st_idx].sxname == _T("autotype")) {
      bDoAutoType = true;
      // Is the next token text and starts with '('?
      if (st_idx + 1 < v_estokens.size() &&
          !v_estokens[st_idx + 1].is_variable &&
          v_estokens[st_idx + 1].sxname.c_str()[0] == _T('(')) {
        // Find ending round bracket
        StringX sx_autotype = v_estokens[st_idx + 1].sxname;
        StringX::size_type st_end_rb = sx_autotype.find(_T(')'));
        if (st_end_rb == StringX::npos) {
          st_column = v_pos[st_idx + 1] + sx_autotype.length() + 2;
          // Missing end round bracket
          uierr = IDSC_EXS_MISSINGROUNDBKT;
          goto exit;
        }
        sxAutoType = sx_autotype.substr(1, st_end_rb - 1);
        v_estokens[st_idx + 1].sxname = sx_autotype.substr(st_end_rb + 1);
        // Check if anythnig left in this text - none -> delete
        if (v_estokens[st_idx + 1].sxname.length() == 0)
          v_estokens.erase(v_estokens.begin() + st_idx + 1);
        // Now delete Autotype variable
        v_estokens.erase(v_estokens.begin() + st_idx);
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
    v_estokens.clear();
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

  StringX::size_type st_nondigit = sxindex.find_first_not_of(num.c_str());
  if (st_nondigit != StringX::npos) {
    st_column = st_nondigit + 1;
    // Index is not numeric
    uierr = IDSC_EXS_INDEXNOTNUMERIC;
    goto exit;
  }

  var_index = _wtoi(sxindex.c_str());
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
