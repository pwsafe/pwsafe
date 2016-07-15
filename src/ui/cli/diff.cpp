#include "./diff.h"
#include "./argutils.h"
#include "./safeutils.h"

#include "../../os/file.h"

#include <iostream>
#include <algorithm>
#include <iomanip>

using namespace std;

#include "../../core/PWScore.h"

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

inline StringX modtime(const StringX &file) {
  time_t ctime, mtime, atime;
  if (pws_os::GetFileTimes(stringx2std(file), ctime, mtime, atime))
    return PWSUtil::ConvertToDateTimeString(mtime, PWSUtil::TMC_EXPORT_IMPORT);
  return StringX{};
}

void print_safe_file(const wchar_t *tag, const PWScore &core)
{
  wcout << tag << L' ' << core.GetCurFile() << L" " << modtime(core.GetCurFile()) << endl;
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

void unified_print_hdr(wchar_t tag, const CompareData &cd)
{
  for_each(cd.cbegin(), cd.cend(), [tag](const st_CompareData &d) {
    wcout << tag << st_GroupTitleUser{d.group, d.title, d.user} << endl;
  });
}

void context_print_items(wchar_t tag, const CompareData &cd, const PWScore &core)
{
  for_each(cd.cbegin(), cd.cend(), [tag, &core](const st_CompareData &d) {
    wcout << L"***************" << endl
          << L"*** " << st_GroupTitleUser{d.group, d.title, d.user} << L" ***" << endl;

    const CItemData &item = core.Find(d.indatabase == CURRENT? d.uuid0: d.uuid1)->second;
    for_each(begin(diff_fields), end(diff_fields), [&item, tag]( CItemData::FieldType ft) {
      if ( !item.GetFieldValue(ft).empty() ) {
        wcout << tag << L' ' << item.FieldName(ft) << L": " << item.GetFieldValue(ft) << endl;
      }
    });
  });
}


void unified_print_item(wchar_t tag, const CItemData &item, const CItemData::FieldBits &fields)
{
  wcout << tag;
  for_each( begin(diff_fields), end(diff_fields), [&fields, &item](CItemData::FieldType ft) {
    if (fields.test(ft)) {
      // make sure we print at least one char, for table'izing with "column" tool
      const StringX val{item.GetFieldValue(ft)};
      wcout << (val.empty()? L"----": val) << L'\t';
    }
  });
  wcout << endl;
}

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
void context_print_differences(const CItemData &item, const CItemData &otherItem,
                                  const CItemData::FieldBits &fields)
{
  for_each( begin(diff_fields), end(diff_fields),
              [&fields, &item, &otherItem](CItemData::FieldType ft) {
    const wchar_t tag = context_tag(ft, fields, item, otherItem);
    if (tag != L'-') {
      wcout << tag << L' ' << item.FieldName(ft) << L": "
            << (tag == L' '? item.GetFieldValue(ft) : otherItem.GetFieldValue(ft)) << endl;
    }
  });
  wcout << endl;
}

static void print_field_labels(const CItemData::FieldBits fields)
{
  for_each( begin(diff_fields), end(diff_fields), [&fields](CItemData::FieldType ft) {
    if (fields.test(ft)) {
      wcout << CItemData::FieldName(ft) << L'\t';
    }
  });
  wcout << endl;
}

inline wostream & print_rmtime(wchar_t tag, wostream &os, const CItemData &i)
{
  if (i.IsRecordModificationTimeSet())
    os << L' ' << tag << i.GetRMTimeExp();
  return os;
}

static void unified_diff(const PWScore &core, const PWScore &otherCore,
                         const CompareData &current, const CompareData &comparison,
                         const CompareData &conflicts, const CompareData &/*identical*/)
{
  print_safe_file(L"---", core);
  print_safe_file(L"+++", otherCore);

  unified_print_hdr(L'-', current);

  for_each(conflicts.cbegin(), conflicts.cend(), [&core, &otherCore](const st_CompareData &d) {
    const CItemData &item = core.Find(d.uuid0)->second;
    const CItemData &otherItem = otherCore.Find(d.uuid1)->second;


    wcout << L"@@ " << st_GroupTitleUser{d.group, d.title, d.user};
    print_rmtime('-', wcout, item);
    print_rmtime('+', wcout, otherItem);
    wcout << L" @@" << endl;

    print_field_labels(d.bsDiffs);
    unified_print_item('-', item, d.bsDiffs);
    unified_print_item('+', otherItem, d.bsDiffs);
  });

  unified_print_hdr(L'+', comparison);
}

static void context_diff(const PWScore &core, const PWScore &otherCore,
                         const CompareData &current, const CompareData &comparison,
                         const CompareData &conflicts, const CompareData &identical)
{
  print_safe_file(L"***", core);
  print_safe_file(L"---", otherCore);

  context_print_items('!', current, core);

  for_each(conflicts.cbegin(), conflicts.cend(), [&core, &otherCore](const st_CompareData &d) {
    const CItemData &item = core.Find(d.uuid0)->second;
    const CItemData &otherItem = otherCore.Find(d.uuid1)->second;

    wcout << L"*** " << st_GroupTitleUser{d.group, d.title, d.user};
    print_rmtime(' ', wcout, item);
    wcout << L" ***" << endl;

    wcout << L"--- " << st_GroupTitleUser{d.group, d.title, d.user};
    print_rmtime(' ', wcout, otherItem);
    wcout << L" ---" << endl;

    context_print_differences(item, otherItem, d.bsDiffs);
  });

  context_print_items('+', comparison, otherCore);
}

wostream & print_sbs_hdr(const CItemData &item, unsigned int cols)
{
  wostringstream os;
  os << st_GroupTitleUser{item.GetGroup(), item.GetTitle(), item.GetUser()};
  print_rmtime( L' ', os << setw(cols) << setfill(L' ') << left, item);
  wstring line{os.str()};
  line.resize(cols, L' ');
  return wcout << setw(cols) << setfill(L' ') << left << line;
}

wstring field_to_line(const CItemData &item, const CItemData::FieldType ft, unsigned int cols)
{
  wostringstream os;
  os << L' ' << item.FieldName(ft) << L": " << item.GetFieldValue(ft);
  wstring line{os.str()};
  line.resize(cols, L' ');
  return line;
}

static void sidebyside_diff(const PWScore &core, const PWScore &otherCore,
                         const CompareData &current, const CompareData &comparison,
                         const CompareData &conflicts, const CompareData &/*identical*/,
                         const CItemData::FieldBits &comparedFields, unsigned int cols)
{
  if ( current.empty() && conflicts.empty() && comparison.empty() )
    return; // print nothing if safes are identical

  // print a header line with safe filenames and modtimes
  wostringstream os;
  os << setw(cols) << left << core.GetCurFile() << L" " << modtime(core.GetCurFile());
  wcout.write(os.str().c_str(), cols);
  wcout << L'|' << otherCore.GetCurFile() << L" " << modtime(otherCore.GetCurFile()) << endl;

  // print a separator line
  wcout << setfill(L'-') << setw(2*cols+1) << L'-' << endl;

  // These remain constant in the next print loop
  wcout << setw(cols) << setfill(L' ') << left;

  // print the orig (left or main) safe in left column
  for_each( current.cbegin(), current.cend(),
        [&core, &comparedFields, cols](const st_CompareData &cd) {
    const CItemData &item = core.Find(cd.uuid0)->second;
    print_sbs_hdr(item, cols) << L'|' << endl;;
    for_each(begin(diff_fields), end(diff_fields),
          [&comparedFields, &item, cols](CItemData::FieldType ft) {
      if (comparedFields.test(ft) && !item.GetFieldValue(ft).empty()) {
        wcout << field_to_line(item, ft, cols) << L'|' << endl;
      }
    });
  });

  // print a separator line
  if ( !current.empty() )
    wcout << setfill(L'-') << setw(2*cols+1) << L'-' << endl;

  // print the conflicting items, one field at a time in one line. Orig safe item's files go to
  // left column, the comparison safe's items to the right.
  for_each( conflicts.cbegin(), conflicts.cend(),
        [&core, &otherCore, cols](const st_CompareData &cd) {
    const CItemData &item = core.Find(cd.uuid0)->second;
    const CItemData &otherItem = otherCore.Find(cd.uuid1)->second;

    // print item headers (GTU + item modification time) in respective columns
    print_sbs_hdr(item, cols) << L'|';
    print_sbs_hdr(otherItem, cols) << endl;

    wcout << setw(cols) << setfill(L' ') << left;
    for_each(begin(diff_fields), end(diff_fields),
          [&cd, &item, &otherItem, cols](CItemData::FieldType ft) {
      // print the fields if they were actually found to be different
      if (cd.bsDiffs.test(ft)) {
        wcout << field_to_line(item, ft, cols) << L'|' << field_to_line(otherItem, ft, cols) << endl;
      }
    });
  });

  // print a separator line
  if ( !conflicts.empty() )
    wcout << setfill(L'-') << setw(2*cols+1) << L'-' << endl;

  // print the comparison safe in right column
  for_each( comparison.cbegin(), comparison.cend(),
      [&otherCore, &comparedFields, cols](const st_CompareData &cd) {

    // fill up the left column with space and end with '|'
    wcout << setw(cols+1) << setfill(L' ') << right << L'|';
    const CItemData &otherItem = otherCore.Find(cd.uuid1)->second;

    // print the header for the item in right column
    print_sbs_hdr(otherItem, cols) << endl;
    for_each(begin(diff_fields), end(diff_fields),
        [&comparedFields, &otherItem, cols](CItemData::FieldType ft) {
      // print the fields that were compared, unless empty
      if (comparedFields.test(ft) && !otherItem.GetFieldValue(ft).empty()) {
        wcout << setw(cols+1) << right << L'|' // fill up left column with space
              << left << field_to_line(otherItem, ft, cols) << endl;
      }
    });
  });
}


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
