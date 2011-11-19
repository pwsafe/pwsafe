/*****************************************************************************************
**																						**
**		Y K D E F  -  Common Yubikey project header										**
**																						**
**		Date		/ Rev		/ Sign	/ Remark										**
**		06-06-03	/ 0.9.0		/ J E	/ Main											**
**		06-08-25	/ 1.0.0		/ J E	/ Rewritten for final spec						**
**		08-06-03	/ 1.3.0		/ J E	/ Added static OTP feature						**
**		09-06-02	/ 2.0.0		/ J E	/ Added version 2 flags                         **
**		09-09-23	/ 2.1.0		/ J E	/ Added version 2.1 flags (OATH-HOTP)           **
**      10-05-01    / 2.2.0     / J E   / Added support for 2.2 extensions + frame      **
**																						**
*****************************************************************************************/

#ifndef	__YKDEF_H_INCLUDED__
#define	__YKDEF_H_INCLUDED__

// Slot entries

#define	SLOT_CONFIG					1       // First (default / V1) configuration
#define SLOT_CONFIG2                3       // Second (V2) configuration

#define SLOT_DEVICE_SERIAL          0x10    // Device serial number

#define SLOT_CHAL_OTP1              0x20    // Write 6 byte challenge to slot 1, get Yubico OTP response
#define SLOT_CHAL_OTP2              0x28    // Write 6 byte challenge to slot 2, get Yubico OTP response

#define SLOT_CHAL_HMAC1             0x30    // Write 64 byte challenge to slot 1, get HMAC-SHA1 response
#define SLOT_CHAL_HMAC2             0x38    // Write 64 byte challenge to slot 2, get HMAC-SHA1 response

#define RESP_ITEM_MASK              0x07    // Mask for slice item # in responses

#define RESP_TIMEOUT_WAIT_MASK      0x1f    // Mask to get timeout value
#define RESP_TIMEOUT_WAIT_FLAG      0x20    // Waiting for timeout operation - seconds left in lower 5 bits
#define RESP_PENDING_FLAG           0x40    // Response pending flag
#define SLOT_WRITE_FLAG             0x80    // Write flag - set by app - cleared by device

#define DUMMY_REPORT_WRITE          0x8f    // Write a dummy report to force update or abort

#define SHA1_MAX_BLOCK_SIZE         64      // Max size of input SHA1 block
#define SHA1_DIGEST_SIZE            20      // Size of SHA1 digest = 160 bits

#define SERIAL_NUMBER_SIZE          4       // Size of device serial number

// Frame structure

#define	SLOT_DATA_SIZE				64

typedef struct {
    unsigned char payload[SLOT_DATA_SIZE]; // Frame payload
    unsigned char slot;                 // Slot # field
    unsigned short crc;                 // CRC field
    unsigned char filler[3];            // Filler
} YKFRAME;

// Ticket structure

#define	UID_SIZE					6	// Size of secret ID field

typedef struct {
	unsigned char uid[UID_SIZE];		// Unique (secret) ID
	unsigned short useCtr;				// Use counter (incremented by 1 at first use after power up) + usage flag in msb
	unsigned short tstpl;				// Timestamp incremented by approx 8Hz (low part)
	unsigned char tstph;				// Timestamp (high part)
	unsigned char sessionCtr;			// Number of times used within session. 0 for first use. After it wraps from 0xff to 1
	unsigned short rnd;					// Pseudo-random value
	unsigned short crc;					// CRC16 value of all fields
} TICKET;

// Activation modifier of sessionUse field (bitfields not uses as they are not portable)

#define	TICKET_ACT_HIDRPT		0x8000	// Ticket generated at activation by keyboard (scroll/num/caps)
#define	TICKET_CTR_MASK			0x7fff	// Mask for useCtr value (except HID flag)

// Configuration structure

#define	FIXED_SIZE				16		// Max size of fixed field
#define	KEY_SIZE				16		// Size of AES key
#define KEY_SIZE_OATH           20      // Size of OATH-HOTP key (key field + first 4 of UID field)
#define	ACC_CODE_SIZE			6		// Size of access code to re-program device

typedef struct {
	unsigned char fixed[FIXED_SIZE];	// Fixed data in binary format
	unsigned char uid[UID_SIZE];		// Fixed UID part of ticket
	unsigned char key[KEY_SIZE];		// AES key
	unsigned char accCode[ACC_CODE_SIZE]; // Access code to re-program device
	unsigned char fixedSize;			// Number of bytes in fixed field (0 if not used)
	unsigned char extFlags;				// Extended flags
	unsigned char tktFlags;				// Ticket configuration flags
	unsigned char cfgFlags;				// General configuration flags
	unsigned char rfu[2];				// Reserved for future use
	unsigned short crc;					// CRC16 value of all fields
} CONFIG;

// Ticket flags

#define	TKTFLAG_TAB_FIRST			0x01		// Send TAB before first part
#define	TKTFLAG_APPEND_TAB1			0x02		// Send TAB after first part
#define	TKTFLAG_APPEND_TAB2			0x04		// Send TAB after second part
#define	TKTFLAG_APPEND_DELAY1		0x08		// Add 0.5s delay after first part
#define	TKTFLAG_APPEND_DELAY2		0x10		// Add 0.5s delay after second part
#define	TKTFLAG_APPEND_CR			0x20		// Append CR as final character
#define	TKTFLAG_PROTECT_CFG2		0x80		// Block update of config 2 unless config 2 is configured and has this bit set

// Configuration flags

#define CFGFLAG_SEND_REF			0x01		// Send reference string (0..F) before data
#define CFGFLAG_PACING_10MS			0x04		// Add 10ms intra-key pacing
#define CFGFLAG_PACING_20MS			0x08		// Add 20ms intra-key pacing
#define CFGFLAG_STATIC_TICKET		0x20		// Static ticket generation

// V1 flags only

#define	CFGFLAG_TICKET_FIRST		0x02		// Send ticket first (default is fixed part)
#define CFGFLAG_ALLOW_HIDTRIG		0x10		// Allow trigger through HID/keyboard

// V2 flags only

#define	CFGFLAG_SHORT_TICKET		0x02		// Send truncated ticket (half length)
#define CFGFLAG_STRONG_PW1          0x10        // Strong password policy flag #1 (mixed case)
#define CFGFLAG_STRONG_PW2          0x40        // Strong password policy flag #2 (subtitute 0..7 to digits)
#define CFGFLAG_MAN_UPDATE          0x80        // Allow manual (local) update of static OTP

// V2.1 flags only

#define	TKTFLAG_OATH_HOTP			0x40		// OATH HOTP mode
#define CFGFLAG_OATH_HOTP8          0x02	    // Generate 8 digits HOTP rather than 6 digits
#define CFGFLAG_OATH_FIXED_MODHEX1  0x10        // First byte in fixed part sent as modhex
#define CFGFLAG_OATH_FIXED_MODHEX2  0x40        // First two bytes in fixed part sent as modhex
#define CFGFLAG_OATH_FIXED_MODHEX   0x50        // Fixed part sent as modhex
#define CFGFLAG_OATH_FIXED_MASK     0x50        // Mask to get out fixed flags

// V2.2 flags only

#define TKTFLAG_CHAL_RESP           0x40        // Challenge-response enabled (both must be set)
#define CFGFLAG_CHAL_YUBICO         0x20        // Challenge-response enabled - Yubico OTP mode
#define CFGFLAG_CHAL_HMAC           0x22        // Challenge-response enabled - HMAC-SHA1
#define CFGFLAG_HMAC_LT64           0x04        // Set when HMAC message is less than 64 bytes
#define CFGFLAG_CHAL_BTN_TRIG       0x08        // Challenge-respoonse operation requires button press

#define EXTFLAG_SERIAL_BTN_VISIBLE  0x01        // Serial number visible at startup (button press)
#define EXTFLAG_SERIAL_USB_VISIBLE  0x02        // Serial number visible in USB iSerial field
#define EXTFLAG_SERIAL_API_VISIBLE  0x04        // Serial number visible via API call

// Status block

typedef struct {
	unsigned char versionMajor;				    // Firmware version information
	unsigned char versionMinor;
	unsigned char versionBuild;
	unsigned char pgmSeq;					    // Programming sequence number. 0 if no valid configuration
	unsigned short touchLevel;				    // Level from touch detector
} STATUS;

#define CONFIG1_VALID               0x01        // Bit in touchLevel indicating that configuration 1 is valid (from firmware 2.1)
#define CONFIG2_VALID               0x02        // Bit in touchLevel indicating that configuration 1 is valid (from firmware 2.1)

// Modified hex string mapping

#define	MODHEX_MAP					"cbdefghijklnrtuv"

#endif		// __YKDEF_H_INCLUDED__