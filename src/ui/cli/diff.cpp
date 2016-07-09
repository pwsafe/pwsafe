#include "./diff.h"
#include "./argutils.h"
#include "./safeutils.h"

#include "../../os/file.h"

#include <iostream>

using namespace std;

#include "../../core/PWScore.h"

inline time_t modtime(const StringX &file) {
  time_t ctime, mtime, atime;
  if (pws_os::GetFileTimes(stringx2std(file), ctime, mtime, atime))
    return mtime;
  return 0;
}

void print_unified_single(wchar_t tag, const CompareData &cd)
{
  for_each(cd.cbegin(), cd.cend(), [tag](const st_CompareData &d) {
    wcout << tag << d.group << L">>" << d.title << L'[' << d.user << ']' << endl;
  });
}

void print_different_fields(wchar_t tag, const CItemData &item, const CItemData::FieldBits &fields)
{
  for( size_t bit = 0; bit < CItem::LAST_DATA; bit++) {
    if (fields.test(bit)) {
          wcout << item.GetFieldValue(static_cast<CItem::FieldType>(bit)) << '\t';
    }
  }
}

static void print_field_labels(const CItemData::FieldBits fields)
{
  for( unsigned char bit = 0; bit < CItem::LAST_DATA; bit++) {
    if (fields.test(bit)) {
      wcout << CItemData::FieldName(static_cast<CItem::FieldType>(bit)) << '\t';
    }
  }
}

static void unified_diff(const PWScore &core, const PWScore &otherCore,
                         const CompareData &current, const CompareData &comparison,
                         const CompareData &conflicts, const CompareData &/*identical*/)
{
  wcout << L"--- " << core.GetCurFile() << L" " << modtime(core.GetCurFile()) << endl;
  wcout << L"+++ " << otherCore.GetCurFile() << L" " << modtime(otherCore.GetCurFile()) << endl;

  print_unified_single(L'-', current);

  for_each(conflicts.cbegin(), conflicts.cend(), [&core, &otherCore](const st_CompareData &d) {
    const CItemData &item = core.Find(d.uuid0)->second;
    const CItemData &otherItem = otherCore.Find(d.uuid1)->second;


    wcout << L"@@ " << d.group << L">>" << d.title << L'[' << d.user << ']';
    if (item.IsRecordModificationTimeSet())
      wcout << L" -" << item.GetRMTimeL();
    if (otherItem.IsRecordModificationTimeSet())
      wcout << L" +" << otherItem.GetRMTimeL();
    wcout << L" @@" << endl;

    print_field_labels(d.bsDiffs);
    wcout << endl;
    print_different_fields('-', item, d.bsDiffs);
    wcout << endl;
    print_different_fields('+', otherItem, d.bsDiffs);
    wcout << endl;
  });

  print_unified_single(L'+', comparison);
}

static void context_diff(const PWScore &core, const PWScore &otherCore,
                         const CompareData &current, const CompareData &comparison,
                         const CompareData &conflicts, const CompareData &identical)
{

}

static void sidebyside_diff(const PWScore &core, const PWScore &otherCore,
                         const CompareData &current, const CompareData &comparison,
                         const CompareData &conflicts, const CompareData &identical)
{

}


int Diff(PWScore &core, const UserArgs &ua)
{
  CompareData current, comparison, conflicts, identical;
  PWScore otherCore;
  constexpr bool treatWhitespacesAsEmpty = false;
  const StringX otherSafe{std2stringx(ua.opArg)};

  CItemData::FieldBits safeFields{ua.fields};
  safeFields.reset(CItem::POLICY);
  for( unsigned char bit = 0; bit < CItem::LAST_DATA; bit++) {
    if (ua.fields.test(bit) && CItemData::IsTextField(bit)) {
      safeFields.set(bit);
    }
  }
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
        sidebyside_diff(core, otherCore, current, comparison, conflicts, identical);
        break;
      default:
        assert(false);
        break;
    }
    otherCore.UnlockFile(otherSafe.c_str());
  }
  return status;
}
