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

void print_unified_single(wchar_t tag, const CompareData &cd)
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


void print_different_fields(wchar_t tag, const CItemData &item, const CItemData::FieldBits &fields)
{
  wcout << tag;
  for_each( begin(diff_fields), end(diff_fields), [&fields, &item](CItemData::FieldType ft) {
    if (fields.test(ft)) {
      wcout << item.GetFieldValue(ft) << '\t';
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
      wcout << CItemData::FieldName(ft) << '\t';
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

  print_unified_single(L'-', current);

  for_each(conflicts.cbegin(), conflicts.cend(), [&core, &otherCore](const st_CompareData &d) {
    const CItemData &item = core.Find(d.uuid0)->second;
    const CItemData &otherItem = otherCore.Find(d.uuid1)->second;


    wcout << L"@@ " << st_GroupTitleUser{d.group, d.title, d.user};
    print_rmtime('-', wcout, item);
    print_rmtime('+', wcout, otherItem);
    wcout << L" @@" << endl;

    print_field_labels(d.bsDiffs);
    print_different_fields('-', item, d.bsDiffs);
    print_different_fields('+', otherItem, d.bsDiffs);
  });

  print_unified_single(L'+', comparison);
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

const size_t colwidth = 80;

wostream & print_in_column(const CItemData &item)
{
  wostringstream os;
  os << st_GroupTitleUser{item.GetGroup(), item.GetTitle(), item.GetUser()};
  print_rmtime( L' ', os << setw(colwidth) << setfill(L' ') << left, item);
  wstring line{os.str()};
  line.resize(colwidth, L' ');
  return wcout << setw(colwidth) << setfill(L' ') << left << line;
}

wstring field_to_line(const CItemData &item, const CItemData::FieldType ft)
{
  wostringstream os;
  os << L' ' << item.FieldName(ft) << L": " << item.GetFieldValue(ft);
  wstring line{os.str()};
  line.resize(colwidth, L' ');
  return line;
}

static void sidebyside_diff(const PWScore &core, const PWScore &otherCore,
                         const CompareData &current, const CompareData &comparison,
                         const CompareData &conflicts, const CompareData &/*identical*/,
                         const CItemData::FieldBits &safeFields)
{
  wostringstream os;
  os << setw(colwidth) << left << core.GetCurFile() << L" " << modtime(core.GetCurFile());
  wcout.write(os.str().c_str(), colwidth);
  wcout << L'|' << otherCore.GetCurFile() << L" " << modtime(otherCore.GetCurFile()) << endl;
  wcout << setfill(L'-') << setw(2*colwidth+1) << L'-' << endl;

  for_each( current.cbegin(), current.cend(), [&core, &safeFields](const st_CompareData &cd) {
    const CItemData &item = core.Find(cd.uuid0)->second;
    print_in_column(item) << L'|' << endl;;
    for_each(begin(diff_fields), end(diff_fields), [&safeFields, &item](CItemData::FieldType ft) {
      if (safeFields.test(ft) && !item.GetFieldValue(ft).empty()) {
        wcout << setw(colwidth) << setfill(L' ') << left << field_to_line(item, ft) << L'|' << endl;
      }
    });
  });

  wcout << setfill(L'-') << setw(2*colwidth+1) << L'-' << endl;

  for_each( conflicts.cbegin(), conflicts.cend(), [&core, &otherCore](const st_CompareData &cd) {
    const CItemData &item = core.Find(cd.uuid0)->second;
    const CItemData &otherItem = otherCore.Find(cd.uuid1)->second;
    print_in_column(item) << L'|';
    print_in_column(otherItem) << endl;
    for_each(begin(diff_fields), end(diff_fields), [&cd, &item, &otherItem](CItemData::FieldType ft) {
      if (cd.bsDiffs.test(ft)) {
        wcout << setw(colwidth) << setfill(L' ') << left << field_to_line(item, ft) << L'|'
              << setw(colwidth) << setfill(L' ') << left << field_to_line(otherItem, ft) << endl;
      }
    });
  });

  wcout << setfill(L'-') << setw(2*colwidth+1) << L'-' << endl;

  for_each( comparison.cbegin(), comparison.cend(), [&otherCore, &safeFields](const st_CompareData &cd) {
    wcout << setw(colwidth+1) << setfill(L' ') << right << L'|';
    const CItemData &otherItem = otherCore.Find(cd.uuid1)->second;
    print_in_column(otherItem) << endl;
    for_each(begin(diff_fields), end(diff_fields), [&safeFields, &otherItem](CItemData::FieldType ft) {
      if (safeFields.test(ft) && !otherItem.GetFieldValue(ft).empty()) {
        wcout << setw(colwidth+1) << setfill(L' ') << right << L'|'
              << setw(colwidth) << setfill(L' ') << left << field_to_line(otherItem, ft) << endl;
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
        sidebyside_diff(core, otherCore, current, comparison, conflicts, identical, safeFields);
        break;
      default:
        assert(false);
        break;
    }
    otherCore.UnlockFile(otherSafe.c_str());
  }
  return status;
}
