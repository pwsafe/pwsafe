// UUIDGen.cpp
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

#ifdef _WIN32
#include "PwsPlatform.h"
#else
/* currently here only for Cygwin test harness */
#include <asm/byteorder.h> /* for htonl, htons */
#endif
#include "UUIDGen.h"

#ifndef POCKET_PC
#include <time.h>
#ifndef _WIN32
#include <sys/time.h>
#endif
#endif
#include <stdlib.h> /* for rand() */
#include <string.h> /* for memcpy */
#include <stdio.h> /* for sprintf() */
#include <assert.h>

typedef struct
{
    char eaddr[6];      /* 6 bytes of ethernet hardware address */
} uuid_address_t, *uuid_address_p_t;

typedef char Byte;
typedef unsigned char  unsigned8;
typedef unsigned short  unsigned16;
typedef unsigned int unsigned32;
typedef unsigned32      boolean32;
typedef unsigned char   *unsigned_char_p_t;

typedef struct
{
    unsigned32  lo;
    unsigned32  hi;
} uuid_time_t, *uuid_time_p_t;

/*
 * Universal Unique Identifier (UUID) types.
 */
typedef struct uuid_struct
{
    unsigned32          time_low;
    unsigned16          time_mid;
    unsigned16          time_hi_and_version;
    unsigned8           clock_seq_hi_and_reserved;
    unsigned8           clock_seq_low;
    Byte                node[6];
} uuid_t, *uuid_p_t;

typedef struct
{
    unsigned32  lo;
    unsigned32  hi;
} unsigned64_t, *unsigned64_p_t;

typedef enum
{
    uuid_e_less_than, uuid_e_equal_to, uuid_e_greater_than
} uuid_compval_t;

const unsigned32 error_status_ok = 0;
const unsigned32 uuid_s_ok = error_status_ok;
const unsigned32 uuid_s_internal_error = 0x16c9a08d;
const unsigned32 uuid_s_bad_version = 0x16c9a088;
const long       uuid_c_version          = 1;
const long       uuid_c_version_highest  = 2;

static uuid_time_t      time_now;     /* utc time as of last query        */
static uuid_time_t      time_last;    /* 'saved' value of time_now        */
static unsigned16       time_adjust;  /* 'adjustment' to ensure uniqness  */
static unsigned16       clock_seq;    /* 'adjustment' for backwards clocks*/

/*
 * UADD_UW_2_UVLW - macro to add a 16-bit unsigned integer to
 *                   a 64-bit unsigned integer
 *
 * Note: see the UADD_UVLW_2_UVLW() macro
 *
 */
#define UADD_UW_2_UVLW(add1, add2, sum)                                 \
{                                                                       \
    (sum)->hi = (add2)->hi;                                             \
    if ((add2)->lo & 0x80000000UL)                                        \
    {                                                                   \
        (sum)->lo = (*add1) + (add2)->lo;                               \
        if (!((sum)->lo & 0x80000000UL))                                  \
        {                                                               \
            (sum)->hi++;                                                \
        }                                                               \
    }                                                                   \
    else                                                                \
    {                                                                   \
        (sum)->lo = (*add1) + (add2)->lo;                               \
    }                                                                   \
}

/*
 * UADD_UVLW_2_UVLW - macro to add two unsigned 64-bit long integers
 *                      (ie. add two unsigned 'very' long words)
 *
 * Important note: It is important that this macro accommodate (and it does)
 *                 invocations where one of the addends is also the sum.
 *
 * This macro was snarfed from the DTSS group and was originally:
 *
 * UTCadd - macro to add two UTC times
 *
 * add lo and high order longword separately, using sign bits of the low-order
 * longwords to determine carry.  sign bits are tested before addition in two
 * cases - where sign bits match. when the addend sign bits differ the sign of
 * the result is also tested:
 *
 *        sign            sign
 *      addend 1        addend 2        carry?
 *
 *          1               1            TRUE
 *          1               0            TRUE if sign of sum clear
 *          0               1            TRUE if sign of sum clear
 *          0               0            FALSE
 */
#define UADD_UVLW_2_UVLW(add1, add2, sum)                               \
    if (!(((add1)->lo&0x80000000UL) ^ ((add2)->lo&0x80000000UL)))           \
    {                                                                   \
        if (((add1)->lo&0x80000000UL))                                    \
        {                                                               \
            (sum)->lo = (add1)->lo + (add2)->lo ;                       \
            (sum)->hi = (add1)->hi + (add2)->hi+1 ;                     \
        }                                                               \
        else                                                            \
        {                                                               \
            (sum)->lo  = (add1)->lo + (add2)->lo ;                      \
            (sum)->hi = (add1)->hi + (add2)->hi ;                       \
        }                                                               \
    }                                                                   \
    else                                                                \
    {                                                                   \
        (sum)->lo = (add1)->lo + (add2)->lo ;                           \
        (sum)->hi = (add1)->hi + (add2)->hi ;                           \
        if (!((sum)->lo&0x80000000UL))                                    \
            (sum)->hi++ ;                                               \
    }

/*
 * Check the reserved bits to make sure the UUID is of the known structure.
 */

#define CHECK_STRUCTURE(uuid) \
( \
    (((uuid)->clock_seq_hi_and_reserved & 0x80) == 0x00) || /* var #0 */ \
    (((uuid)->clock_seq_hi_and_reserved & 0xc0) == 0x80) || /* var #1 */ \
    (((uuid)->clock_seq_hi_and_reserved & 0xe0) == 0xc0)    /* var #2 */ \
)

/*
 * The following macros invoke CHECK_STRUCTURE(), check that the return
 * value is okay and if not, they set the status variable appropriately
 * and return either a boolean FALSE, nothing (for void procedures),
 * or a value passed to the macro.  This has been done so that checking
 * can be done more simply and values are returned where appropriate
 * to keep compilers happy.
 *
 * bCHECK_STRUCTURE - returns boolean FALSE
 * vCHECK_STRUCTURE - returns nothing (void)
 * rCHECK_STRUCTURE - returns 'r' macro parameter
 */

#define bCHECK_STRUCTURE(uuid, status) \
{ \
    if (!CHECK_STRUCTURE (uuid)) \
    { \
        *(status) = uuid_s_bad_version; \
        return (FALSE); \
    } \
}

#define vCHECK_STRUCTURE(uuid, status) \
{ \
    if (!CHECK_STRUCTURE (uuid)) \
    { \
        *(status) = uuid_s_bad_version; \
        return; \
    } \
}

#define rCHECK_STRUCTURE(uuid, status, result) \
{ \
    if (!CHECK_STRUCTURE (uuid)) \
    { \
        *(status) = uuid_s_bad_version; \
        return (result); \
    } \
}

/*
 * defines for time calculations
 */
#define UUID_C_100NS_PER_SEC            10000000
#define UUID_C_100NS_PER_USEC           10

/*
 * local defines used in uuid bit-diddling
 */
#define HI_WORD(w)                  ((w) >> 16)
#define RAND_MASK                   0x3fff      /* same as CLOCK_SEQ_LAST */

#define TIME_MID_MASK               0x0000ffff
#define TIME_HIGH_MASK              0x0fff0000
#define TIME_HIGH_SHIFT_COUNT       16

#define MAX_TIME_ADJUST             0x7fff

#define CLOCK_SEQ_LOW_MASK          0xff
#define CLOCK_SEQ_HIGH_MASK         0x3f00
#define CLOCK_SEQ_HIGH_SHIFT_COUNT  8
#define CLOCK_SEQ_FIRST             1
#define CLOCK_SEQ_LAST              0x3fff      /* same as RAND_MASK */

/*
 * Note: If CLOCK_SEQ_BIT_BANG == TRUE, then we can avoid the modulo
 * operation.  This should save us a divide instruction and speed
 * things up.
 */

#ifndef CLOCK_SEQ_BIT_BANG
#define CLOCK_SEQ_BIT_BANG          1
#endif

#if CLOCK_SEQ_BIT_BANG
#define CLOCK_SEQ_BUMP(seq)         ((*seq) = ((*seq) + 1) & CLOCK_SEQ_LAST)
#else
#define CLOCK_SEQ_BUMP(seq)         ((*seq) = ((*seq) + 1) % (CLOCK_SEQ_LAST+1))
#endif

#define UUID_VERSION_BITS           (uuid_c_version << 12)
#define UUID_RESERVED_BITS          0x80

#define IS_OLD_UUID(uuid) (((uuid)->clock_seq_hi_and_reserved & 0xc0) != 0x80)

/*
**  U U I D _ _ U E M U L
**
**  Functional Description:
**        32-bit unsigned quantity * 32-bit unsigned quantity
**        producing 64-bit unsigned result. This routine assumes
**        long's contain at least 32 bits. It makes no assumptions
**        about byte orderings.
**
**  Inputs:
**
**        u, v       Are the numbers to be multiplied passed by value
**
**  Outputs:
**
**        prodPtr    is a pointer to the 64-bit result
**
**  Note:
**        This algorithm is taken from: "The Art of Computer
**        Programming", by Donald E. Knuth. Vol 2. Section 4.3.1
**        Pages: 253-255.
**--
**/

static void uuid__uemul(unsigned32 u, unsigned32 v, unsigned64_t *prodPtr)
{
  /*
   * following the notation in Knuth, Vol. 2
   */
  unsigned32      uuid1, uuid2, v1, v2, temp;

  uuid1 = u >> 16;
  uuid2 = u & 0xffff;
  v1 = v >> 16;
  v2 = v & 0xffff;

  temp = uuid2 * v2;
  prodPtr->lo = temp & 0xffff;
  temp = uuid1 * v2 + (temp >> 16);
  prodPtr->hi = temp >> 16;
  temp = uuid2 * v1 + (temp & 0xffff);
  prodPtr->lo += (temp & 0xffff) << 16;
  prodPtr->hi += uuid1 * v1 + (temp >> 16);
}

/*
** T I M E _ C M P
**
** Compares two UUID times (64-bit UTC values)
**/

static uuid_compval_t time_cmp(uuid_time_p_t time1, uuid_time_p_t time2)
{
    /*
     * first check the hi parts
     */
    if (time1->hi < time2->hi) return (uuid_e_less_than);
    if (time1->hi > time2->hi) return (uuid_e_greater_than);

    /*
     * hi parts are equal, check the lo parts
     */
    if (time1->lo < time2->lo) return (uuid_e_less_than);
    if (time1->lo > time2->lo) return (uuid_e_greater_than);

    return (uuid_e_equal_to);
}

/*
 *  Define constant designation difference in Unix and DTSS base times:
 *  DTSS UTC base time is October 15, 1582.
 *  Unix base time is January 1, 1970.
 */
#define uuid_c_os_base_time_diff_lo     0x13814000
#define uuid_c_os_base_time_diff_hi     0x01B21DD2

/*
 * U U I D _ _ G E T _ O S _ T I M E
 *
 * Get OS time - contains platform-specific code.
 */
static void uuid__get_os_time (uuid_time_t * uuid_time)
{

  struct timeval      tp;
  unsigned64_t        utc,
    usecs,
    os_basetime_diff;

  /*
   * Get current time
   */
  if (gettimeofday (&tp, (struct timezone *) 0))
    {
      return;
    }

  /*
   * Multiply the number of seconds by the number clunks 
   */
  uuid__uemul ((long) tp.tv_sec, UUID_C_100NS_PER_SEC, &utc);

  /*
   * Multiply the number of microseconds by the number clunks 
   * and add to the seconds
   */
  uuid__uemul ((long) tp.tv_usec, UUID_C_100NS_PER_USEC, &usecs);
  UADD_UVLW_2_UVLW (&usecs, &utc, &utc);

  /*
   * Offset between DTSS formatted times and Unix formatted times.
   */
  os_basetime_diff.lo = uuid_c_os_base_time_diff_lo;
  os_basetime_diff.hi = uuid_c_os_base_time_diff_hi;
  UADD_UVLW_2_UVLW (&utc, &os_basetime_diff, uuid_time);

}

/*
** N E W _ C L O C K _ S E Q
**
** Ensure *clkseq is up-to-date
**
** Note: clock_seq is architected to be 14-bits (unsigned) but
**       I've put it in here as 16-bits since there isn't a
**       14-bit unsigned integer type (yet)
**/

/*
 * Windows has the ANSI time() function, so it will use that.
 */
#ifndef POCET_PC
#define UUID_NONVOLATILE_CLOCK
#endif

static void new_clock_seq(unsigned16 *clkseq)
{
  /*
   * A clkseq value of 0 indicates that it hasn't been initialized.
   */
  if (*clkseq == 0)
    {
#ifdef UUID_NONVOLATILE_CLOCK
      *clkseq = (unsigned16)time(NULL); /* casting 32 bit value into 16 bits */
      if (*clkseq == 0)                       /* still not init'd ???   */
        {
	  *clkseq = (unsigned16)rand();      /* no clock to seed srand() with... */
        }
#else
      /*
       * with a volatile clock, we always init to a random number
       */
      *clkseq = (unsigned16)rand();
#endif
    }

  CLOCK_SEQ_BUMP (clkseq);
  if (*clkseq == 0)
    {
      *clkseq = *clkseq + 1;
    }
}

/*
 * U U I D _ _ G E T _ O S _ A D D R E S S
 *
 * Get ethernet hardware address from the OS
 */
static void uuid__get_os_address(uuid_address_p_t addr, unsigned32 *status)
{
  /*
   * Cheat big time since we won't always have a NIC
   */

  addr->eaddr[0] = 0x27;
  addr->eaddr[1] = 0x4f;
  addr->eaddr[2] = 0xfb;
  addr->eaddr[3] = 0x2f;
  addr->eaddr[4] = 0x55;
  addr->eaddr[5] = 0x4f;
  *status = error_status_ok;
}

/*
**++
**
**  ROUTINE NAME:       uuid_create
**
**  SCOPE:              PUBLIC - declared in UUID.IDL
**
**  DESCRIPTION:
**
**  Create a new UUID. Note: we only know how to create the new
**  and improved UUIDs.
**
**  INPUTS:             none
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:
**
**      uuid            A new UUID value
**
**      status          return status value
**
**          uuid_s_ok
**          uuid_s_coding_error
**
**  IMPLICIT INPUTS:    none
**
**  IMPLICIT OUTPUTS:   none
**
**  FUNCTION VALUE:     void
**
**  SIDE EFFECTS:       none
**
**--
**/

static void uuid_create(uuid_t *uuid, unsigned32 *status)
{
  uuid_address_t          eaddr;      /* our IEEE 802 hardware address */
  boolean32               got_no_time = 0;

  /*
   * get our hardware network address
   */
  uuid__get_os_address(&eaddr, status);
  if (*status != error_status_ok) {
    return;
  }

  do {
    /*
     * get the current time
     */
    uuid__get_os_time (&time_now);

    /*
     * do stuff like:
     *
     *  o check that our clock hasn't gone backwards and handle it
     *    accordingly with clock_seq
     *  o check that we're not generating uuid's faster than we
     *    can accommodate with our time_adjust fudge factor
     */
    switch (time_cmp (&time_now, &time_last))
      {
      case uuid_e_less_than:
	new_clock_seq (&clock_seq);
	time_adjust = 0;
	break;
      case uuid_e_greater_than:
	time_adjust = 0;
	break;
      case uuid_e_equal_to:
	if (time_adjust == MAX_TIME_ADJUST) {
	  /*
	   * spin your wheels while we wait for the clock to tick
	   */
	  got_no_time = 1;
	} else {
	  time_adjust++;
	}
	break;
      default:
	*status = uuid_s_internal_error;
	return;
      }
  } while (got_no_time);

  time_last.lo = time_now.lo;
  time_last.hi = time_now.hi;

  if (time_adjust != 0) {
    UADD_UW_2_UVLW (&time_adjust, &time_now, &time_now);
  }

  /*
   * now construct a uuid with the information we've gathered
   * plus a few constants
   */
  uuid->time_low = time_now.lo;
  uuid->time_mid = time_now.hi & TIME_MID_MASK;

  uuid->time_hi_and_version =
    (time_now.hi & TIME_HIGH_MASK) >> TIME_HIGH_SHIFT_COUNT;
  uuid->time_hi_and_version |= UUID_VERSION_BITS;

  uuid->clock_seq_low = clock_seq & CLOCK_SEQ_LOW_MASK;
  uuid->clock_seq_hi_and_reserved =
    (clock_seq & CLOCK_SEQ_HIGH_MASK) >> CLOCK_SEQ_HIGH_SHIFT_COUNT;

  uuid->clock_seq_hi_and_reserved |= UUID_RESERVED_BITS;

  memcpy (uuid->node, &eaddr, sizeof (uuid_address_t));

  *status = uuid_s_ok;
}


/*
**++
**
**  ROUTINE NAME:       uuid_to_string
**
**  SCOPE:              PUBLIC - declared in UUID.IDL
**
**  DESCRIPTION:
**
**  Encode a UUID into a printable string.
**
**  INPUTS:
**
**      uuid            A binary UUID to be converted to a string UUID.
**
**  INPUTS/OUTPUTS:     none
**
**  OUTPUTS:
**
**      uuid_string     The string representation of the given UUID.
**
**      status          return status value
**
**          uuid_s_ok
**          uuid_s_bad_version
**          uuid_s_coding_error
**
**  IMPLICIT INPUTS:    none
**
**  IMPLICIT OUTPUTS:   none
**
**  FUNCTION VALUE:     void
**
**  SIDE EFFECTS:       none
**
**--
**/

static void uuid_to_string(uuid_p_t uuid, unsigned_char_p_t uuid_string,
			   unsigned32 *status)
{
  /*
   * don't do anything if the output argument is NULL
   */
  if (uuid_string == NULL) {
    *status = uuid_s_ok;
    return;
  }

  vCHECK_STRUCTURE (uuid, status);

  sprintf((char *)uuid_string,
	  "%08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x",
	  uuid->time_low, uuid->time_mid, uuid->time_hi_and_version,
	  uuid->clock_seq_hi_and_reserved, uuid->clock_seq_low,
	  (unsigned8) uuid->node[0], (unsigned8) uuid->node[1],
	  (unsigned8) uuid->node[2], (unsigned8) uuid->node[3],
	  (unsigned8) uuid->node[4], (unsigned8) uuid->node[5]);

  *status = uuid_s_ok;
}


CUUIDGen::CUUIDGen()
{
  unsigned32 status;
  uuid = new uuid_struct;
  
  uuid_create(uuid, &status);
  assert(status == uuid_s_ok);
}

CUUIDGen::~CUUIDGen()
{
  delete uuid;
}

void CUUIDGen::GetUUID(uuid_array_t &uuid_array)
{
  unsigned32 *p = (unsigned32 *)uuid_array;
  *p = htonl(uuid->time_low);
  unsigned16 *p1 = (unsigned16 *)&uuid_array[4];
  *p1 = htons(uuid->time_mid);
  unsigned16 *p2 = (unsigned16 *)&uuid_array[6];
  *p2 = htons(uuid->time_hi_and_version);
  unsigned8  *p3 = (unsigned8 *)&uuid_array[8];
  *p3 = uuid->clock_seq_hi_and_reserved;
  unsigned8  *p4 = (unsigned8 *)&uuid_array[9];
  *p4 = uuid->clock_seq_low;
  Byte *p5 = (Byte *)&uuid_array[10];
  for (int i = 0; i < 6; i++)
    p5[i] = uuid->node[i];
}

void CUUIDGen::GetUUIDStr(uuid_str_t &str)
{
  unsigned32 status;
  uuid_to_string(uuid, str, &status);
  assert(status == uuid_s_ok);
}

#ifdef TEST
#include <stdio.h>
int main()
{
  uuid_str_t str;
  uuid_array_t uuid_array;

  for (int i = 0; i< 10; i++) {
    CUUIDGen uuid;
    uuid.GetUUIDStr(str);
    printf("%s\n",str);
    uuid.GetUUID(uuid_array);
    printf("%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x\n",
	   uuid_array[0], uuid_array[1], uuid_array[2], uuid_array[3], 
	   uuid_array[4], uuid_array[5], uuid_array[6], uuid_array[7], 
	   uuid_array[8], uuid_array[9], uuid_array[10], uuid_array[11], 
	   uuid_array[12], uuid_array[13], uuid_array[14], uuid_array[15]);
  }
  return 0;
}
#endif
