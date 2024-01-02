/*
* Copyright (c) 2013-2024 Rony Shapiro <ronys@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
* Contributed by Ashley R. Thomas, 01-Oct-2023
*/
// RFC4648_Base32Decoder.h
//-----------------------------------------------------------------------------
#ifndef __RFC4648_BASE32DECODER_H
#define __RFC4648_BASE32DECODER_H

#include <vector>

#include "../Util.h"
#include "./external/Chromium/base32.h"

class RFC4648_Base32Decoder
{
public:
  static const size_t BITS_PER_BASE32_CHAR = 5;
  static const size_t BITS_PER_BYTE = 8;
public:

  RFC4648_Base32Decoder(const char* base32_encoded_key)
  {
    decoded_bytes.resize((strlen(base32_encoded_key) * BITS_PER_BASE32_CHAR) / BITS_PER_BYTE);
    is_successful = !decoded_bytes.empty() && base32_decode(&decoded_bytes[0], static_cast<int>(decoded_bytes.size()) * BITS_PER_BYTE, base32_encoded_key, 0);
    if (!is_successful)
      clear();
  }

  RFC4648_Base32Decoder(const RFC4648_Base32Decoder& other) {
    *this = other;
  }

  RFC4648_Base32Decoder(RFC4648_Base32Decoder&& other) noexcept {
    *this = std::move(other);
  }

  RFC4648_Base32Decoder& operator=(const RFC4648_Base32Decoder& other) {
    if (&other == this)
      return *this;
    clear();
    is_successful = other.is_successful;
    decoded_bytes = other.decoded_bytes;
    return *this;
  }

  RFC4648_Base32Decoder& operator=(RFC4648_Base32Decoder&& other) noexcept {
    if (&other == this)
      return *this;
    clear();
    is_successful = other.is_successful;
    decoded_bytes = std::move(other.decoded_bytes);
    other.is_successful = false;
    return *this;
  }

  virtual ~RFC4648_Base32Decoder() {
    clear();
  }

  bool is_decoding_successful() const { return is_successful;  }

  void clear() {
    if (!decoded_bytes.empty())
      trashMemory(&decoded_bytes[0], decoded_bytes.size());
    decoded_bytes.clear();
  }

  size_t get_size() const {
    return decoded_bytes.size();
  }

  const unsigned char* get_ptr() const {
    return decoded_bytes.empty() ? nullptr : & decoded_bytes[0];
  }

  const std::vector<uint8_t>& get_bytes() const {
    return decoded_bytes;
  }

private:
  bool is_successful;
  std::vector<uint8_t> decoded_bytes;
};


#endif /* __RFC4648_BASE32DECODER_H */
//-----------------------------------------------------------------------------
// Local variables:
// mode: c++
// End:
