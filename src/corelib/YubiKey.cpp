/*
* Copyright (c) 2003-2008 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
// YubiKey.cpp
//-----------------------------------------------------------------------------

#include <string> // for msg verification
#include "StringXStream.h"
#include "UTF8Conv.h"
#include "Util.h"
#include "sha1.h"
#include "hmac.h"
#include "os/http.h"

#include "YubiKey.h"

using namespace std;

// Following should be https, currently unavailable :-(
static const char *AuthServer = "http://api.yubico.com/wsapi/verify?";

YubiKeyAuthenticator::YubiKeyAuthenticator(unsigned int apiID,
                                           const YubiApiKey_t &apiKey)
  : m_apiID(apiID)
{
  memcpy(m_apiKey, apiKey, sizeof(YubiApiKey_t));
}


stringT YubiKeyAuthenticator::SignReq(const string &msg)
{
  HMAC<SHA1> hmac(m_apiKey, sizeof(m_apiKey));
  hmac.Update(reinterpret_cast<const unsigned char *>(msg.c_str()),
              msg.length());
  unsigned char hcal[HMAC<SHA1>::HASHLEN];
  hmac.Final(hcal);
  return PWSUtil::Base64Encode(hcal, HMAC<SHA1>::HASHLEN);
}

bool YubiKeyAuthenticator::VerifySig(const string &msg, const string &h)
{
  HMAC<SHA1> hmac(m_apiKey, sizeof(m_apiKey));
  hmac.Update(reinterpret_cast<const unsigned char *>(msg.c_str()),
              msg.length());
  unsigned char hcal[HMAC<SHA1>::HASHLEN];
  hmac.Final(hcal);

  BYTE *hv = new BYTE[2*h.length()]; // a bit too much, who cares?
  size_t hlen = 0;
  CString H(h.c_str()); // kludge to workaround char/wchar_t pain
  PWSUtil::Base64Decode(LPCTSTR(H), hv, hlen);
  if (hlen != HMAC<SHA1>::HASHLEN) {
    TRACE(_T("YubiKeyAuthenticator::VerifySig: bad hlen(%d)\n"), hlen);
    delete[] hv;
    return false;
  }
  bool retval = (std::memcmp(hcal, hv, HMAC<SHA1>::HASHLEN) == 0);
  delete[] hv;
  return retval;
}

bool YubiKeyAuthenticator::VerifyOTP(const stringT &otp)
{

  if (otp.empty()) {
    m_error = _T("Empty OTP");
    return false;
  }

  m_error = _T("Sending authentication request");
  pws_os::HttpClient client;
  CUTF8Conv conv;
  const unsigned char *otp_str;
  int otp_strlen;
  conv.ToUTF8(otp.c_str(), otp_str, otp_strlen);

  bool retval = false;

  ostringstream os;
  os << "id=" << m_apiID;
  os << "&otp=" << otp_str;
  stringT req_sig = SignReq(os.str());

  StringX Req;
  conv.FromUTF8(reinterpret_cast<const unsigned char *>(os.str().c_str()),
                os.str().size(), Req);
#if 0 // still can't get this to work. Needs to be enabled on server?
  Req += _T("&h=");
  Req += req_sig.c_str();
#endif

  CString urlStr(AuthServer);
  urlStr += Req.c_str();

  if (client.Navigate(LPCTSTR(urlStr))) {
    if (client.GetStatus() == 200) { // 200 = successful HTTP transaction
      string text = client.GetBody();

      /**
       * A typical response is of the form:
       *  0         1         2         
       *  012345678901234567890123456789
       * "h=fiuyV7/F7ql48I0Qt2wt5mZbrPA=
       *  t=2008-10-29T14:57:42
       *  status=OK
       *  Each line ends with \n and all but the last are fixed length
       */
      istringstream is(text);
      string line, rsp_hash, rsp_timestamp, rsp_status, rsp_info;
      while (getline(is, line)) {
        if (line.find("h=") == 0)
          rsp_hash = line.substr(2, line.length()-3);
        else if (line.find("t=") == 0)
          rsp_timestamp = line.substr(0, line.length()-1);
        else if (line.find("status=") == 0)
          rsp_status = line.substr(0, line.length()-1);
        else if (line.find("info=") == 0)
          rsp_info = line.substr(0, line.length()-1);
        else if (line.empty() || line == "\n" || line == "\r")
          continue;
        else {
          TRACE(_T("Unexpected value in response: %s\n"), line.c_str());
          m_error = _T("Malformed response");
          goto done;
        }
      } // parse response

      string msg;
      if (!rsp_info.empty()) {
        msg = rsp_info; msg += "&";
      }
      msg += rsp_status; msg += "&"; msg += rsp_timestamp;

      if (!VerifySig(msg, rsp_hash)) {
        m_error = _T("Signature verification failed!");
        goto done;
      }

      m_error = _T(""); // rsp_status
      retval = (rsp_status.find("OK") != string::npos);
      if (!retval) {
        m_error = _T("Authentication failure: ");
        StringX statX; // pesky conversions...
        conv.FromUTF8(reinterpret_cast<const unsigned char *>(rsp_status.c_str()),
                rsp_status.size(), statX);
        m_error += statX.c_str();
      }
    } else { // successful transaction
      ostringstreamT os;
      os << _T("Request failed: ") << client.GetStatus();
      m_error = os.str();
    }
  } else { // !m_client.Navigate()
    ostringstreamT os;
    os << _T("request failed - status code: ") << client.GetStatus();
    m_error = os.str();
  }
 done:
  return retval;
}
