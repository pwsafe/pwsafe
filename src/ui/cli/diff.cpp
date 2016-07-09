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
                         ua.subset.valid(),
                         treatWhitespacesAsEmpty,
                         ua.subset.value,
                         ua.subset.field,
                         ua.subset.rule,
                         current,
                         comparison,
                         conflicts,
                         identical);
                         
  }
  return PWScore::SUCCESS;
}
