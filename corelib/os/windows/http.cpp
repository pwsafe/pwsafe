/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/**
 * \file Windows-specific implementation of http.h
 */
#include <afx.h>
#include <Windows.h>
#include <atlhttp.h>
#include <string>
#include "../http.h"

/**
 * Simple http client wrapper implementation
 */

// XXX TBD - support possible proxy definitions
// ?? How to get system defaults ??
// m_client->SetProxy( m_proxy, m_proxy_port );
namespace pws_os {
  class HttpClientImpl
  {
  public:
    bool Navigate(const stringT &urlStr)
    { return client.Navigate(urlStr.c_str(), &navData); }

    CAtlHttpClient client;
    CAtlNavigateData navData;
  };

  HttpClient::HttpClient()
  {
    impl = new HttpClientImpl;
  }

  HttpClient::~HttpClient()
  {
    delete impl;
  }

  bool HttpClient::Navigate(const stringT &urlStr)
  {
    return impl->Navigate(urlStr);
  }
  
  std::string HttpClient::GetBody()
  {
    int bodyLen = impl->client.GetBodyLength();
    char *body = new char[bodyLen+1];
    memcpy( body, impl->client.GetBody(), bodyLen );
    body[bodyLen] = 0;

    std::string text(body);
    delete[] body;
    return text;
  }

  int HttpClient::GetStatus()
  {
    return impl->client.GetStatus();
  }
};
