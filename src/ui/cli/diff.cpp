#include "./diff.h"
#include "./argutils.h"
#include "./safeutils.h"


#include "../../core/PWScore.h"

int Diff(PWScore &core, const UserArgs &ua)
{
  CompareData current, comparison, conflicts, identical;
  PWScore otherCore;
  constexpr bool treatWhitespacesAsEmpty = false;
  int status = OpenCore(otherCore, std2stringx(ua.opArg));
  if ( status == PWScore::SUCCESS ) {
    core.Compare( &otherCore,
                  ua.fields,
                         !ua.subset.empty(),
                         treatWhitespacesAsEmpty,
                         ua.subset[0].value,
                         ua.subset[0].field,
                         ua.subset[0].rule,
                         current,
                         comparison,
                         conflicts,
                         identical);
                         
  }
  return PWScore::SUCCESS;
}
