/*
* Copyright (c) 2003-2010 Rony Shapiro <ronys@users.sourceforge.net>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __SHA1_H
#define __SHA1_H

class SHA1
{
public:
  enum {HASHLEN = 20};

  SHA1();
  ~SHA1();

  void Update(const unsigned char* data, unsigned int len);
  void Final(unsigned char digest[HASHLEN]);
  void Clear();

  SHA1(const SHA1 &cntxt)
  {
    for (int i = 0; i < 5; i++)
      state[i] = cntxt.state[i];

    count[0] = cntxt.count[0];
    count[1] = cntxt.count[1];

    for (int i = 0; i < 64; i++)
      buffer[i] = cntxt.buffer[i];
  }

  SHA1 &operator=(const SHA1 &cntxt)
  {
    if (this != &cntxt) {
      for (int i = 0; i < 5; i++)
        state[i] = cntxt.state[i];

      count[0] = cntxt.count[0];
      count[1] = cntxt.count[1];

      for (int i = 0; i < 64; i++)
        buffer[i] = cntxt.buffer[i];
    }
    return *this;
  }

private:
  unsigned long state[5];
  unsigned long count[2];
  unsigned char buffer[64];
};
#endif /* __SHA1_H */
