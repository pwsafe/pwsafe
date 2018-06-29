/*
 * Created by Saurav Ghosh
 * Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License 2.0 terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license-2.0.php
 */

#include "./diff.h"
#include "./argutils.h"
#include "./safeutils.h"

#include "../../os/file.h"
#include "../../core/core.h"
#include "../../core/PWHistory.h"

#include "../../core/StringXStream.h"
#include <algorithm>
#include <iomanip>
#include <functional>

using namespace std;

#include "../../core/PWScore.h"

// We only work with these fields for diff'ing & printing
const CItem::FieldType diff_fields[] = {
    CItem::GROUP,
    CItem::TITLE,
    CItem::USER,
    CItem::PASSWORD,
    CItem::EMAIL,
    CItem::NOTES,
    CItem::URL,
    CItem::AUTOTYPE,
    CItem::CTIME,
    CItem::PMTIME,
    CItem::ATIME,
    CItem::XTIME,
    CItem::XTIME_INT,
    CItem::PWHIST,
    CItem::POLICY,
    CItem::POLICYNAME,
    CItem::SYMBOLS,
    CItem::RUNCMD,
    CItem::DCA,
    CItem::SHIFTDCA,
    CItem::KBSHORTCUT,
    CItem::PROTECTED
};

//////////////////////////////////////////////////////////
// Common functions
///////////////
inline StringX modtime(const StringX &file) {
  time_t ctime, mtime, atime;
  if (pws_os::GetFileTimes(stringx2std(file), ctime, mtime, atime))
    return PWSUtil::ConvertToDateTimeString(mtime, PWSUtil::TMC_EXPORT_IMPORT);
  return StringX{};
}

inline StringX safe_file_hdr(const wchar_t *tag, const PWScore &core)
{
  StringXStream os;
  os << tag << L' ' << core.GetCurFile() << L" " << modtime(core.GetCurFile());
  return os.str();
}

uint32_t dca2str(uint16 dca) {
  const std::map<int16_t, uint32_t> dca_id_str = {
    {PWSprefs::DoubleClickAutoType,             IDSC_DCAAUTOTYPE},
    {PWSprefs::DoubleClickBrowse,               IDSC_DCABROWSE},
    {PWSprefs::DoubleClickBrowsePlus,           IDSC_DCABROWSEPLUS},
    {PWSprefs::DoubleClickCopyNotes,            IDSC_DCACOPYNOTES},
    {PWSprefs::DoubleClickCopyUsername,         IDSC_DCACOPYUSERNAME},
    {PWSprefs::DoubleClickCopyPassword,         IDSC_DCACOPYPASSWORD},
    {PWSprefs::DoubleClickCopyPasswordMinimize, IDSC_DCACOPYPASSWORDMIN},
    {PWSprefs::DoubleClickRun,                  IDSC_DCARUN},
    {PWSprefs::DoubleClickSendEmail,            IDSC_DCASENDEMAIL},
    {PWSprefs::DoubleClickViewEdit,             IDSC_DCAVIEWEDIT}
  };

  const auto loc = dca_id_str.find(dca);
  return loc != dca_id_str.end() ? loc->second: IDSC_INVALID;
}

using line_t = StringX;
using lines_vec = std::vector<line_t>;

lines_vec stream2vec(StringXStream &wss) {
    lines_vec vlines;
    do {
        StringX line;
        std::getline(wss, line);
        if ( !line.empty() ) vlines.push_back(line);
    }
    while( !wss.eof() );
    return vlines;
}

inline wostream& print_field_value(wostream &os, wchar_t tag,
                                    const CItemData &item, CItemData::FieldType ft)
{
  StringX fieldValue;
  switch (ft) {
    case CItemData::DCA:
    case CItemData::SHIFTDCA:
    {
      int16 dca = -1;
      if (item.GetDCA(dca) != -1) {
        LoadAString(fieldValue, dca2str(dca));
      }
      break;
    }
    case CItemData::PWHIST:
    {
      const StringX pwh_str = item.GetPWHistory();
      if (!pwh_str.empty()) {
        StringXStream value_stream;
        size_t ignored;
        PWHistList pwhl;
        const bool save_pwhistory = CreatePWHistoryList(pwh_str, ignored, ignored, pwhl, PWSUtil::TMC_LOCALE);
        value_stream << L"Save: " << (save_pwhistory? L"Yes" : L"No");
        if ( !pwhl.empty() ) value_stream << endl;
        for( const auto &pwh: pwhl) value_stream << pwh.changedate << L": " << pwh.password << endl;
        fieldValue = value_stream.str();
      }
      break;
    }
    case CItemData::POLICY:
    {
        PWPolicy policy;
        item.GetPWPolicy(policy);
        fieldValue = policy.GetDisplayString();
        break;
    }
    default:
      fieldValue = item.GetFieldValue(ft);
      break;
  }
  const StringX sep1{L' '}, sep2{L": "};
  StringXStream tmpStream;
  tmpStream << tag << L' ' << item.FieldName(ft) << L": " << fieldValue;
  const auto offset = 1 /*tag*/ + sep1.size() + sep2.size() + item.FieldName(ft).size();
  lines_vec lines{ stream2vec(tmpStream)};
  if ( lines.size() > 1) {
    std::for_each( lines.begin()+1, lines.end(), [offset](StringX &line) { line.insert(0, offset, L' '); });
  }
  for( const auto &line: lines ) os << line << endl;
  return os;
}

inline StringX rmtime(wchar_t tag, const CItemData &i)
{
  StringXStream os;
  if (i.IsRecordModificationTimeSet())
    os << L' ' << tag << i.GetRMTimeExp();
  return os.str();
}

using unique_hdr_func_t = function<void(const st_CompareData &cd, wchar_t tag)>;

void print_unique_items(wchar_t tag, const CompareData &cd, const PWScore &core,
                            unique_hdr_func_t hdr_fn)
{
  for(const auto &d: cd) {
    hdr_fn(d, tag);
    const CItemData &item = core.Find(d.indatabase == CURRENT? d.uuid0: d.uuid1)->second;
    for( auto ft : diff_fields ) {
      switch(ft) {
        case CItem::GROUP:
        case CItem::TITLE:
        case CItem::USER:
          break;
        default:
          if ( d.bsDiffs.test(ft) && !item.GetFieldValue(ft).empty() ) {
            print_field_value(wcout, tag, item, ft);
          }
      }
    }
  }
}

using item_diff_func_t = function<void(const CItemData &item,
                                       const CItemData &otherItem,
                                       const CItemData::FieldBits &fields,
                                       CItemData::FieldType ft)>;

inline bool have_empty_policies(const CItemData &item, const CItemData &otherItem) {
    return item.GetPWPolicy().empty() && otherItem.GetPWPolicy().empty();
}

void print_conflicting_item(const CItemData &item, const CItemData &otherItem,
                            const CItemData::FieldBits &fields, item_diff_func_t diff_fn)
{
  for( auto ft: diff_fields ) {
    switch(ft) {
      case CItem::GROUP:
      case CItem::TITLE:
      case CItem::USER:
        break;
      default:
        if (fields.test(ft)) {
            switch (ft) {
                case CItemData::POLICY:
                {
                    // Policy comparison compares default policies for the safes if the
                    // item's policy is empty. We just consider them to be same if empty.
                    if ( have_empty_policies(item, otherItem) ) {
                        continue;
                    }
                    break;
                }
                default:
                    break;
            }
            diff_fn(item, otherItem, fields, ft);
        }
        break;
    }
  }
}

using conflict_hdr_func_t = function<void(const st_CompareData &cd,
                                          const CItemData &item,
                                          const CItemData &otherItem)>;

void print_conflicts(const CompareData &conflicts, const PWScore &core,
                            const PWScore &otherCore, conflict_hdr_func_t hdr_fn,
                            item_diff_func_t diff_fn)
{
  for( const auto &cd: conflicts ) {
    const CItemData &item = core.Find(cd.uuid0)->second;
    const CItemData &otherItem = otherCore.Find(cd.uuid1)->second;
    if (cd.bsDiffs.count() == 1 && cd.bsDiffs.test(CItemData::POLICY) && have_empty_policies(item, otherItem))
        continue;
    hdr_fn(cd, item, otherItem);
    print_conflicting_item(item, otherItem, cd.bsDiffs, diff_fn);
  }
}

//////////////////////////////////////////////////////////////////
// Unified diff
//////////
void unified_print_unique_items(wchar_t tag, const CompareData &cd, const PWScore &core)
{
  print_unique_items(tag, cd, core, [](const st_CompareData &cd, wchar_t tag) {
    wcout << L"***************" << endl
          << tag << st_GroupTitleUser{cd.group, cd.title, cd.user} << endl;
  });
}

static void unified_diff(const PWScore &core, const PWScore &otherCore,
                         const CompareData &current, const CompareData &comparison,
                         const CompareData &conflicts, const CompareData &/*identical*/)
{
  wcout << safe_file_hdr(L"---", core) << endl;
  wcout << safe_file_hdr(L"+++", otherCore) << endl;

  unified_print_unique_items(L'-', current, core);

  auto hdr_fn = [](const st_CompareData &cd,
                   const CItemData &item,
                   const CItemData &otherItem) {
    wcout << L"***************" << endl
          << L"@@ " << st_GroupTitleUser{cd.group, cd.title, cd.user}
          << rmtime(L'-', item) << rmtime(L'+', otherItem) << L" @@" << endl;
  };

  auto item_fn = []( const CItemData &item,
                      const CItemData &otherItem,
                      const CItemData::FieldBits &fields,
                      CItemData::FieldType ft ) {
    if (fields.test(ft)) {
      print_field_value(wcout, L'-', item, ft);
      print_field_value(wcout, L'+', otherItem, ft);
    }
  };

  print_conflicts(conflicts, core, otherCore, hdr_fn, item_fn);

  unified_print_unique_items(L'+', comparison, otherCore);
}


/////////////////////////////////////////////////////////////////
// Context diff
////////
inline wchar_t context_tag(CItem::FieldType ft, const CItemData::FieldBits &fields,
                const CItemData &item, const CItemData &otherItem)
{
  // The two items were compared & found to be differing on this field
  // only show this tag for fields there were compared
  if (fields.test(ft))
    return '!';

  const StringX val{item.GetFieldValue(ft)};

  // This field was not compared, it could be different. Print it only if
  // it is the same in both items
  if (val == otherItem.GetFieldValue(ft))
    return L' ';

  if (val.empty())
    return L'+';

  // Don't print it
  return L'-';
}

void context_print_unique_items(wchar_t tag, const CompareData &cd, const PWScore &core)
{
  print_unique_items(tag, cd, core,  [](const st_CompareData &cd, wchar_t /*tag*/) {
    wcout << L"***************" << endl
          << L"*** " << st_GroupTitleUser{cd.group, cd.title, cd.user} << L" ***" << endl;
  });
}

static void context_diff(const PWScore &core, const PWScore &otherCore,
                         const CompareData &current, const CompareData &comparison,
                         const CompareData &conflicts, const CompareData & /* identical */)
{
  wcout << safe_file_hdr(L"***", core) << endl;
  wcout << safe_file_hdr(L"---", otherCore) << endl;

  context_print_unique_items('!', current, core);

  auto hdr_fn = [](const st_CompareData &cd,
                   const CItemData &item,
                   const CItemData &otherItem) {
    wcout << L"*** " << st_GroupTitleUser{cd.group, cd.title, cd.user}
          << rmtime(' ', item) << L" ***" << endl
          << L"--- " << st_GroupTitleUser{cd.group, cd.title, cd.user}
          << rmtime(' ', otherItem) << L" ---" << endl;
  };

  auto item_fn = []( const CItemData &item,
                     const CItemData &otherItem,
                     const CItemData::FieldBits &fields,
                     CItemData::FieldType ft ) {
    const wchar_t tag = context_tag(ft, fields, item, otherItem);
    if (tag != L'-') {
      print_field_value(wcout, tag, tag == L' '? item: otherItem, ft);
    }
  };

  print_conflicts(conflicts, core, otherCore, hdr_fn, item_fn);

  context_print_unique_items('+', comparison, otherCore);
}


//////////////////////////////////////////////
// Side-by-side diff
//////////

template <class StringType>
StringType resize(StringType s, typename StringType::size_type len) {
  if (s.size() < len) s.resize(len, ' ');
  return s;
}

lines_vec resize_lines(lines_vec lines, line_t::size_type cols ) {
  for( auto &line: lines ) line = resize(line, cols);
  return lines;
}

// TODO: convert to lambda when using C++14
template <class left_line_t, class right_line_t>
void sbs_print(const PWScore &core,
               const PWScore &otherCore,
               const CompareData &matches,
               const CItemData::FieldBits &comparedFields,
               unsigned int cols, bool print_fields)
{
  for( const auto &cd: matches ) {
    const CItemData &item = core.Find(cd.uuid0)->second;
    const CItemData &otherItem = otherCore.Find(cd.uuid1)->second;
    if (cd.bsDiffs.count() == 1 && cd.bsDiffs.test(CItemData::POLICY) && have_empty_policies(item, otherItem))
        continue;
    const CItemData::FieldBits &df = cd.bsDiffs.any()? cd.bsDiffs: comparedFields;
    left_line_t left_line{core, cd.uuid0, cols};
    right_line_t right_line{otherCore, cd.uuid1, cols};
    wcout << left_line() << L'|' << right_line() << endl;
    if ( print_fields ) {
      for( auto ft: diff_fields ) {
        // print the fields if they were actually found to be different
        if (df.test(ft) && !have_empty_policies(item, otherItem)) {
          StringXStream wssl, wssr;
          wssl << left_line(ft) << flush;
          wssr << right_line(ft) << flush;
          lines_vec left_lines{resize_lines(stream2vec(wssl), cols)},
                  right_lines{resize_lines(stream2vec(wssr), cols)};
          const int ndiff = left_lines.size() - right_lines.size();
          if (ndiff < 0)
              left_lines.insert(left_lines.end(), -ndiff, StringX(cols, L' '));
          else if (ndiff > 0)
              right_lines.insert(right_lines.end(), ndiff, StringX(cols, L' '));
          for (lines_vec::size_type idx = 0; idx < left_lines.size(); ++idx)
              wcout << left_lines[idx] << L'|' << right_lines[idx] << endl;
        }
      }
    }
    wcout << resize(wstring(cols/5, left_line.sep_char), cols) << L'|'
          << resize(wstring(cols/5, right_line.sep_char), cols) << endl;
  }
};

struct field_to_line
{
  const wchar_t sep_char = L'-';
  const CItemData &item;
  unsigned int columns;
  field_to_line(const PWScore &core, const pws_os::CUUID& uuid, unsigned int cols)
  : item{core.Find(uuid)->second}, columns{cols}
  {}
  StringX operator()() const {
    oStringXStream os;
    os << st_GroupTitleUser{item.GetGroup(), item.GetTitle(), item.GetUser()}
       << rmtime( L' ', item);
    StringX hdr{os.str()};
    hdr.resize(columns, L' ');
    return hdr;
  }
  StringX operator()(CItemData::FieldType ft) const {
    oStringXStream os;
    print_field_value(os, L' ', item, ft);
    StringX line{os.str()};
    if (line.find(L'\n') == wstring::npos ) line.resize(columns, L' ');
    return line;
  }
};

struct blank
{
  const wchar_t sep_char = L' ';
  wstring line;
  blank(const PWScore &, const pws_os::CUUID &, unsigned int cols)
  : line(static_cast<size_t>(cols), L' ')
  {}
  // header
  wstring operator()() const { return line; }
  // fields
  wstring operator()(CItemData::FieldType ) const { return line; }
};


static void sidebyside_diff(const PWScore &core, const PWScore &otherCore,
                         const CompareData &current, const CompareData &comparison,
                         const CompareData &conflicts, const CompareData &/*identical*/,
                         const CItemData::FieldBits &comparedFields, unsigned int cols)
{
  if ( current.empty() && conflicts.empty() && comparison.empty() )
    return; // print nothing if safes are identical

  // print a header line with safe filenames and modtimes
  StringX hdr_left{safe_file_hdr(L"", core)}, hdr_right{safe_file_hdr(L"", otherCore)};
  hdr_left.resize(cols, L' ');
  hdr_right.resize(cols, L' ');
  wcout << hdr_left << L'|' << hdr_right << endl;

  // print a separator line
  wcout << setfill(L'-') << setw(2*cols+1) << L'-' << endl;

  // These remain constant in the next print loop
  wcout << setw(cols) << setfill(L' ') << left;

  // print the orig (left or main) safe in left column
  sbs_print<field_to_line, blank>(core, otherCore, current, comparedFields, cols, false);

  // print the conflicting items, one field at a time in one line. Orig safe item's files go to
  // left column, the comparison safe's items to the right.
  sbs_print<field_to_line, field_to_line>(core, otherCore, conflicts, comparedFields, cols, true);

  // print the comparison safe in right column
  sbs_print<blank, field_to_line>(core, otherCore, comparison, comparedFields, cols, false);
}

///////////////////////////////////
// dispatcher. Called from main()
/////////
int Diff(PWScore &core, const UserArgs &ua)
{
  CompareData current, comparison, conflicts, identical;
  PWScore otherCore;
  const StringX otherSafe{std2stringx(ua.opArg)};

  CItemData::FieldBits safeFields{ua.fields};
  for( auto ft: diff_fields ) {
    if (ua.fields.test(ft) && CItemData::IsTextField(ft)) {
      safeFields.set(ft);
    }
  }
  safeFields.reset(CItem::RMTIME);

  int status = OpenCore(otherCore, otherSafe);
  if ( status == PWScore::SUCCESS ) {
    constexpr bool treatWhitespacesAsEmpty = false;
    core.Compare( &otherCore,
                  safeFields,
                         ua.subset.valid(),
                         treatWhitespacesAsEmpty,
                         ua.subset.value,
                         ua.subset.field,
                         ua.subset.rule,
                         current,
                         comparison,
                         conflicts,
                         identical);

    switch (ua.dfmt) {
      case UserArgs::DiffFmt::Unified:
        unified_diff(core, otherCore, current, comparison, conflicts, identical);
        break;
      case UserArgs::DiffFmt::Context:
        context_diff(core, otherCore, current, comparison, conflicts, identical);
        break;
      case UserArgs::DiffFmt::SideBySide:
        sidebyside_diff(core, otherCore, current, comparison, conflicts, identical, safeFields, ua.colwidth);
        break;
      default:
        assert(false);
        break;
    }
    otherCore.UnlockFile(otherSafe.c_str());
  }
  return status;
}
