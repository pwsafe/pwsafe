/* Copyright 2017 The ChromiumOS Authors
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

 /*
 * This basis for this source file was originally obtained from the following URL:
 * https://chromium.googlesource.com/chromiumos/platform/ec/+/refs/heads/main/common/base32.c
 */

#include <ctype.h>
#include <algorithm>

 /* Base-32 encoding/decoding */
#include "base32.h"

static const unsigned char crc5_table1[] = {
	0x00, 0x0E, 0x1C, 0x12, 0x11, 0x1F, 0x0D, 0x03,
	0x0B, 0x05, 0x17, 0x19, 0x1A, 0x14, 0x06, 0x08
};
static const unsigned char crc5_table0[] = {
	0x00, 0x16, 0x05, 0x13, 0x0A, 0x1C, 0x0F, 0x19,
	0x14, 0x02, 0x11, 0x07, 0x1E, 0x08, 0x1B, 0x0D
};
uint8_t crc5_sym(uint8_t sym, uint8_t previous_crc)
{
	uint8_t tmp = sym ^ previous_crc;
	return crc5_table1[tmp & 0x0F] ^ crc5_table0[(tmp >> 4) & 0x0F];
}

/* A-Z0-9 with I,O,0,1 removed */
// From original Chromium source code:
//const char base32_map[33] = "ABCDEFGHJKLMNPQRSTUVWXYZ23456789";
// RFC4648 table:
const char base32_map[33] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

/**
 * Decode a base32 symbol.
 *
 * @param sym Input symbol
 * @return The symbol value or -1 if error.
 */
static int decode_sym(int sym)
{
	int i = 0;

	for (i = 0; i < 32; i++) {
		if (toupper(sym) == base32_map[i])
			return i;
	}

	return -1;
}

bool base32_encode(char *dest, int destlen_chars,
		  const void *srcbits, int srclen_bits,
		  int add_crc_every)
{
	const uint8_t* src = reinterpret_cast<const uint8_t*>(srcbits);
	int destlen_needed;
	int crc = 0, crc_count = 0;
	int didx = 0;
	int i;
	*dest = 0;
	/* Make sure destination is big enough */
	destlen_needed = (srclen_bits + 4) / 5;  /* Symbols before adding CRC */
	if (add_crc_every) {
		/* Must be an exact number of groups to add CRC */
		if (destlen_needed % add_crc_every)
			return false;
		destlen_needed += destlen_needed / add_crc_every;
	}
	destlen_needed++;  /* For terminating null */
	if (destlen_chars < destlen_needed)
		return false;
	for (i = 0; i < srclen_bits; i += 5) {
		int sym;
		int sidx = i / 8;
		int bit_offs = i % 8;
		int byte_bits_remaining = 8 - bit_offs;

		if (bit_offs <= 3) {
			/* Entire symbol fits in that byte */
			sym = src[sidx] >> (3 - bit_offs);
		} else {
			/* Use the bits we have left */
			sym = src[sidx] << (bit_offs - 3); /* This uses byte_bits_remaining bits. */

			/* Use the bits from the next byte, if any */
			if (i + byte_bits_remaining < srclen_bits)
				sym |= src[sidx + 1] >> (11 - bit_offs);
		}

		sym &= 0x1f;

		/* Pad incomplete symbol with 0 bits */
		if (srclen_bits - i < 5)
			sym &= 0x1f << (5 + i - srclen_bits);

		dest[didx++] = base32_map[sym];

		/* Add CRC if needed */
		if (add_crc_every) {
			crc = crc5_sym(static_cast<uint8_t>(sym), static_cast<uint8_t>(crc));
			if (++crc_count == add_crc_every) {
				dest[didx++] = base32_map[crc];
				crc_count = crc = 0;
			}
		}
	}

	/* Terminate string and return */
	dest[didx] = 0;
	return true;
}

bool base32_decode(uint8_t *dest, int destlen_bits, const char *src,
		  int crc_after_every)
{
	int crc = 0, crc_count = 0;
	int out_bits = 0;

	for (; *src; src++) {
		int sym, sbits, dbits, b;

		if (isspace(*src) || *src == '-')
			continue;

		sym = decode_sym(*src);
		if (sym < 0)
			return false;  /* Bad input symbol */

		/* Check CRC if needed */
		if (crc_after_every) {
			if (crc_count == crc_after_every) {
				if (crc != sym)
					return false;
				crc_count = crc = 0;
				continue;
			} else {
				crc = crc5_sym(static_cast<uint8_t>(sym), static_cast<uint8_t>(crc));
				crc_count++;
			}
		}

		/*
		 * Stop if we're out of space.  Have to do this after checking
		 * the CRC, or we might not check the last CRC.
		 */
		if (out_bits >= destlen_bits)
			break;

		/* See how many bits we get to use from this symbol */
		sbits = std::min(5, destlen_bits - out_bits);
		if (sbits < 5)
			sym >>= (5 - sbits);

		/* Fill up the rest of the current byte */
		dbits = 8 - (out_bits & 7);
		b = std::min(dbits, sbits);
		if (dbits == 8)
			dest[out_bits / 8] = 0; /* Starting a new byte */
		dest[out_bits / 8] |= (sym << (dbits - b)) >> (sbits - b);
		out_bits += b;
		sbits -= b;

		/* Start the next byte if there's space */
		if (sbits > 0) {
			dest[out_bits / 8] = static_cast<uint8_t>(sym << (8 - sbits));
			out_bits += sbits;
		}
	}

	/* If we have CRCs, should have a full group */
	if (crc_after_every && crc_count)
		return false;

	/* Success if reaching end of input string. */
	return *src == '\0';
}
