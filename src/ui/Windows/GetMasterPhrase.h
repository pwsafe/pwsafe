/*
* Copyright (c) 2014 David Kelvin <c-273@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

#pragma once

#include "../../core/StringX.h"

struct GetMasterPhrase {
  StringX sPhrase, sNewPhrase;
  bool bPhraseEntered, bNewPhraseEntered;

  GetMasterPhrase()
    : bPhraseEntered(false), bNewPhraseEntered(false) {}

  void clear()
  {
    bPhraseEntered = bNewPhraseEntered = false;
    sPhrase.clear();
    sNewPhrase.clear();
  }
};
