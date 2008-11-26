/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __HTTP_H
#define __HTTP_H
#include "typedefs.h"

namespace pws_os {
  /**
   * Simple http client wrapper
   */
  class HttpClientImpl; // Handle design pattern
  
  class HttpClient {
  public:
    HttpClient();
    ~HttpClient();
    bool Navigate(const stringT &urlStr);
    std::string GetBody(); // NOT stringT
    int GetStatus();
  private:
    HttpClientImpl *impl;
    HttpClient(const HttpClient &); // don't allow copying
    HttpClient &operator=(const HttpClient &); // nor assignment
  };
};
#endif /* __HTTP_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
