/*
* Copyright (c) 2013-2019 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/
#ifndef __KEYWRAP_H
#define __KEYWRAP_H

/**
* KeyWrap takes a Fish-derived block cipher instance that's
* created with the wrapping key, the key to be wrapped/unwrapped,
* its length and a pointer to the output.
* Wrapping is done per RFC3394, except that we support any block
* cipher, not just AES. Also, we only use the default IV.
*/

class Fish;

class KeyWrap
{
public:
 KeyWrap(Fish *fish = 0) : m_fish(fish) {}
  ~KeyWrap() {}

  void SetFish(Fish *fish) {m_fish = fish;} // if not set in c'tor
  void Wrap(const unsigned char *in, unsigned char *out, unsigned inlen);
  bool Unwrap(const unsigned char *in, unsigned char *out, unsigned inlen);
  
 private:
  Fish *m_fish;
};

#endif /* __KEYWRAP_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
