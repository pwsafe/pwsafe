// UUIDGen.h
// Silly class for generating UUIDs
// Each instance has its own unique value, 
// which can be accessed as an array of bytes or as a human-readable
// ASCII string.
//
// Much as I hate re-inventing wheels, I couldn't find a way to do 
// this programatically in Windows.
//
// The standard way for generating these requires the MAC address of the
// host computer, which makes sense for RPC, the original intended use.
// Since PasswordSafe will run on machines without a NIC, we fake the MAC.
//
// Code based on publicly available source from:
//  ftp://ftp.digital.com/pub/DEC/DCE/PD-DCE-RPC.tar.Z
// Original source copyright notice follows:
/*
 * 
 * (c) Copyright 1989 OPEN SOFTWARE FOUNDATION, INC.
 * (c) Copyright 1989 HEWLETT-PACKARD COMPANY
 * (c) Copyright 1989 DIGITAL EQUIPMENT CORPORATION
 * To anyone who acknowledges that this file is provided "AS IS"
 * without any express or implied warranty:
 *                 permission to use, copy, modify, and distribute this
 * file for any purpose is hereby granted without fee, provided that
 * the above copyright notices and this notice appears in all source
 * code copies, and that none of the names of Open Software
 * Foundation, Inc., Hewlett-Packard Company, or Digital Equipment
 * Corporation be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission.  Neither Open Software Foundation, Inc., Hewlett-
 * Packard Company, nor Digital Equipment Corporation makes any
 * representations about the suitability of this software for any
 * purpose.
 * 
 */

#ifndef __UUID_H
#define __UUID_H

typedef unsigned char uuid_array_t[16];
typedef unsigned char uuid_str_t[37]; //"204012e6-600f-4e01-a5eb-515267cb0d50"
struct uuid_struct;

class CUUIDGen {
 public:
  CUUIDGen(); // UUID generated at creation time
  ~CUUIDGen();
  void GetUUID(uuid_array_t &);
  void GetUUIDStr(uuid_str_t &);
 private:
  uuid_struct *uuid;
};

#endif /* __UUID_H */
