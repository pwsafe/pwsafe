/*
* Copyright (c) 2003-2011 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * Interface / wrapper to YubiKey API
 */
#pragma once

#import <YubiClientAPI.dll> no_namespace, named_guids

struct Yubi {
  Yubi(CCmdTarget *owner);
  void Init(); // Call in owner's OnInit
  void Destroy(); // Call in Owner's OnDestroy
  IYubiClient *m_obj;
  DWORD m_eventCookie;
  ycENCODING m_encoding;
  void putVariant(_variant_t va, UINT dataField);
  void getVariant(_variant_t *va, UINT dataField);
private:
  CCmdTarget *m_owner;
};
