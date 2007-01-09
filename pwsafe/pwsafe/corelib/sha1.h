/*
 * Copyright (c) 2003-2007 Rony Shapiro <ronys@users.sourceforge.net>.
 * All rights reserved. Use of the code is allowed under the
 * Artistic License terms, as specified in the LICENSE file
 * distributed with this code, or available from
 * http://www.opensource.org/licenses/artistic-license.php
 */
#ifndef _SHA1_H_
#define _SHA1_H_

class SHA1
{
 public:
  enum {HASHLEN = 20};
  SHA1();
  ~SHA1();
  void Update(const unsigned char* data, unsigned int len);
  void Final(unsigned char digest[HASHLEN]);
 private:
  unsigned long state[5];
  unsigned long count[2];
  unsigned char buffer[64];
};
#endif
