#include "./diff.h"
#include "./argutils.h"
#include "./safeutils.h"

#include "../../os/file.h"

#include <iostream>
#include <algorithm>
#include <iomanip>

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

inline wstring safe_file_hdr(const wchar_t *tag, const PWScore &core)
{
  wstringstream os;
  os << tag << L' ' << core.GetCurFile() << L" " << modtime(core.GetCurFile());
  return os.str();
}

inline wostream& print_field_value(wostream &os, wchar_t tag,
                                    const CItemData &item, CItemData::FieldType ft)
{
  return os << tag << L' ' << item.FieldName(ft) << L": " << item.GetFieldValue(ft);
}

wostream & operator<<( wostream &os, const st_GroupTitleUser &gtu)
{
  if ( !gtu.group.empty() )
    os << gtu.group << L" >> ";

  assert( !gtu.title.empty() );
  os << gtu.title;

  if ( !gtu.user.empty() ) {
    os << L'[' << gtu.user << L']';
  }

  return os;
}

inline wostream & print_rmtime(wchar_t tag, wostream &os, const CItemData &i)
{
  if (i.IsRecordModificationTimeSet())
    os << L' ' << tag << i.GetRMTimeExp();
  return os;
}

using unique_hdr_func_t = function<void(const st_CompareData &cd, wchar_t tag)>;

void print_unique_items(wchar_t tag, const CompareData &cd, const PWScore &core,
                            unique_hdr_func_t hdr_fn)
{
  for_each(cd.cbegin(), cd.cend(), [tag, &core, &hdr_fn](const st_CompareData &d) {
    hdr_fn(d, tag);
    const CItemData &item = core.Find(d.indatabase == CURRENT? d.uuid0: d.uuid1)->second;
    for_each( begin(diff_fields), end(diff_fields), [&item, tag](CItemData::FieldType ft) {
      switch(ft) {
        case CItem::GROUP:
        case CItem::TITLE:
        case CItem::USER:
          break;
        default:
          if ( !item.GetFieldValue(ft).empty() ) {
            print_field_value(wcout, tag, item, ft) << endl;
          }
      }
    });
  });
}

//////////////////////////////////////////////////////////////////
// Unified diff
//////////
void unified_print_unique_items(wchar_t tag, const CompareData &cd, const PWScore &core)
{
  print_unique_items(tag, cd, core, [](const st_CompareData &cd, wchar_t tag) {
    wcout << tag << st_GroupTitleUser{cd.group, cd.title, cd.user} << endl;
  });
}

void unified_print_conflicting_item(const CItemData &item, const CItemData &otherItem,
                            const CItemData::FieldBits &fields)
{
  for_each( begin(diff_fields), end(diff_fields),
              [&fields, &item, &otherItem](CItemData::FieldType ft) {
    switch(ft) {
      case CItem::GROUP:
      case CItem::TITLE:
      case CItem::USER:
        break;
      default:
        if (fields.test(ft)) {
          print_field_value(wcout, L'-', item, ft) << endl;
          print_field_value(wcout, L'+', otherItem, ft) << endl;
        }
    }
  });
}

static void unified_diff(const PWScore &core, const PWScore &otherCore,
                         const CompareData &current, const CompareData &comparison,
                         const CompareData &conflicts, const CompareData &/*identical*/)
{
  wcout << safe_file_hdr(L"---", core) << endl;
  wcout << safe_file_hdr(L"+++", otherCore) << endl;

  unified_print_unique_items(L'-', current, core);

  for_each(conflicts.cbegin(), conflicts.cend(), [&core, &otherCore](const st_CompareData &d) {
    const CItemData &item = core.Find(d.uuid0)->second;
    const CItemData &otherItem = otherCore.Find(d.uuid1)->second;


    wcout << L"@@ " << st_GroupTitleUser{d.group, d.title, d.user};
    print_rmtime('-', wcout, item);
    print_rmtime('+', wcout, otherItem);
    wcout << L" @@" << endl;

    unified_print_conflicting_item(item, otherItem, d.bsDiffs);
  });

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

void context_print_conflicting_item(const CItemData &item, const CItemData &otherItem,
                                  const CItemData::FieldBits &fields)
{
  for_each( begin(diff_fields), end(diff_fields),
              [&fields, &item, &otherItem](CItemData::FieldType ft) {
    const wchar_t tag = context_tag(ft, fields, item, otherItem);
    if (tag != L'-') {
      print_field_value(wcout, tag, tag == L' '? item: otherItem, ft) << endl;
    }
  });
  wcout << endl;
}

static void context_diff(const PWScore &core, const PWScore &otherCore,
                         const CompareData &current, const CompareData &comparison,
                         const CompareData &conflicts, const CompareData &identical)
{
  wcout << safe_file_hdr(L"***", core) << endl;
  wcout << safe_file_hdr(L"---", otherCore) << endl;

  context_print_unique_items('!', current, core);

  for_each(conflicts.cbegin(), conflicts.cend(), [&core, &otherCore](const st_CompareData &d) {
    const CItemData &item = core.Find(d.uuid0)->second;
    const CItemData &otherItem = otherCore.Find(d.uuid1)->second;

    wcout << L"*** " << st_GroupTitleUser{d.group, d.title, d.user};
    print_rmtime(' ', wcout, item);
    wcout << L" ***" << endl;

    wcout << L"--- " << st_GroupTitleUser{d.group, d.title, d.user};
    print_rmtime(' ', wcout, otherItem);
    wcout << L" ---" << endl;

    context_print_conflicting_item(item, otherItem, d.bsDiffs);
  });

  context_print_unique_items('+', comparison, otherCore);
}


//////////////////////////////////////////////
// Side-by-side diff
//////////

// TODO: convert to lambda when using C++14
template <class left_line_t, class right_line_t>
void sbs_print(const PWScore &core,
               const PWScore &otherCore,
               const CompareData &matches,
               const CItemData::FieldBits comparedFields,
               unsigned int cols)
{
  for_each( matches.cbegin(), matches.cend(), [&](const st_CompareData &cd) {
    const CItemData::FieldBits &df = cd.bsDiffs.any()? cd.bsDiffs: comparedFields;
    left_line_t left_line{core, cd.uuid0, cols};
    right_line_t right_line{otherCore, cd.uuid1, cols};
    wcout << left_line() << L'|' << right_line() << endl;
    for_each(begin(diff_fields), end(diff_fields), [&](CItemData::FieldType ft) {
      // print the fields if they were actually found to be different
      if (df.test(ft)) {
        wcout << left_line(ft) << L'|' << right_line(ft) << endl;
      }
    });
  });
};

struct field_to_line
{
  const CItemData &item;
  unsigned int columns;
  field_to_line(const PWScore &core, const pws_os::CUUID& uuid, unsigned int cols)
  : item{core.Find(uuid)->second}, columns{cols}
  {}
  wstring operator()() const {
    wostringstream os;
    os << st_GroupTitleUser{item.GetGroup(), item.GetTitle(), item.GetUser()};
    print_rmtime( L' ', os << setw(columns) << setfill(L' ') << left, item);
    wstring hdr{os.str()};
    hdr.resize(columns, L' ');
    return hdr;
  }
  wstring operator()(CItemData::FieldType ft) const {
    wostringstream os;
    print_field_value(os, L' ', item, ft);
    wstring line{os.str()};
    line.resize(columns, L' ');
    return line;
  }
};

struct blank
{
  wstring line;
  blank(const PWScore &, const pws_os::CUUID &, unsigned int cols)
  : line(static_cast<size_t>(cols), L' ')
  {}
  // header
  wstring operator()() const { return line; }
  // fields
  wstring operator()(CItemData::FieldType ft) const { return line; }
};


static void sidebyside_diff(const PWScore &core, const PWScore &otherCore,
                         const CompareData &current, const CompareData &comparison,
                         const CompareData &conflicts, const CompareData &/*identical*/,
                         const CItemData::FieldBits &comparedFields, unsigned int cols)
{
  if ( current.empty() && conflicts.empty() && comparison.empty() )
    return; // print nothing if safes are identical

  // print a header line with safe filenames and modtimes
  wstring hdr_left{safe_file_hdr(L"", core)}, hdr_right{safe_file_hdr(L"", otherCore)};
  hdr_left.resize(cols, L' ');
  hdr_right.resize(cols, L' ');
  wcout << hdr_left << L'|' << hdr_right << endl;

  // print a separator line
  wcout << setfill(L'-') << setw(2*cols+1) << L'-' << endl;

  // These remain constant in the next print loop
  wcout << setw(cols) << setfill(L' ') << left;

  // print the orig (left or main) safe in left column
  sbs_print<field_to_line, blank>(core, otherCore, current, comparedFields, cols);

  // print a separator line
  if ( !current.empty() )
    wcout << setfill(L'-') << setw(2*cols+1) << L'-' << endl;

  // print the conflicting items, one field at a time in one line. Orig safe item's files go to
  // left column, the comparison safe's items to the right.
  sbs_print<field_to_line, field_to_line>(core, otherCore, conflicts, comparedFields, cols);

  // print a separator line
  if ( !conflicts.empty() )
    wcout << setfill(L'-') << setw(2*cols+1) << L'-' << endl;

  // print the comparison safe in right column
  sbs_print<blank, field_to_line>(core, otherCore, comparison, comparedFields, cols);
}

///////////////////////////////////
// dispatcher. Called from main()
/////////
int Diff(PWScore &core, const UserArgs &ua)
{
  CompareData current, comparison, conflicts, identical;
  PWScore otherCore;
  constexpr bool treatWhitespacesAsEmpty = false;
  const StringX otherSafe{std2stringx(ua.opArg)};

  CItemData::FieldBits safeFields{ua.fields};
  safeFields.reset(CItem::POLICY);
  for_each( begin(diff_fields), end(diff_fields),
                [&ua, &safeFields](CItemData::FieldType ft) {
    if (ua.fields.test(ft) && CItemData::IsTextField(ft)) {
      safeFields.set(ft);
    }
  });
  safeFields.reset(CItem::POLICY);
  safeFields.reset(CItem::RMTIME);

  int status = OpenCore(otherCore, otherSafe);
  if ( status == PWScore::SUCCESS ) {
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
