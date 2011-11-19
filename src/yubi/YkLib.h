/*************************************************************************
**                                                                      **
**      Y K L I B  -  Unified Yubikey low-level interface library       **
**                                                                      **
**      Copyright 2011 - Yubico AB                                      **
**                                                                      **
**      Date   / Sig / Rev  / History                                   **
**      110329 / J E / 0.00 / Main                                      **
**                                                                      **
*************************************************************************/

#pragma once

#pragma pack(push, 1)
#include "ykdef.h"
#pragma pack(pop)

#define WIN32_MEAN_AND_LEAN
#include <windows.h>

// waitForCompletion time constants (milliseconds)

#define	YKLIB_NO_WAIT	        0
#define YKLIB_MAX_SERIAL_WAIT   50
#define YKLIB_MAX_CHAL_WAIT     120
#define YKLIB_MAX_WRITE_WAIT    600

// Target slot constants

#define	YKLIB_FIRST_SLOT	    0
#define	YKLIB_SECOND_SLOT	    1

typedef enum { YKLIB_CHAL_OTP, YKLIB_CHAL_HMAC } YKLIB_CHAL_MODE;

typedef enum { 
	YKLIB_OK,
    YKLIB_FAILURE,
	YKLIB_NOT_OPENED,
	YKLIB_INVALID_PARAMETER,
	YKLIB_NO_DEVICE,
	YKLIB_MORE_THAN_ONE,
	YKLIB_WRITE_ERROR,
	YKLIB_INVALID_RESPONSE,
	YKLIB_NOT_COMPLETED,
	YKLIB_NOT_CONFIGURED,
    YKLIB_NOT_READY,
    YKLIB_PROCESSING,
    YKLIB_TIMER_WAIT,
	YKLIB_UNSUPPORTED_FEATURE	
} YKLIB_RC;

class CYkLib
{
private:
	HANDLE m_handle;
    void flushPortNames(void);
    struct tagPORT_LIST {
        wchar_t *name;
        struct tagPORT_LIST *next;
    } *m_portList;
    unsigned char m_lastCmd;
    unsigned char m_seq;

    YKLIB_RC rawWrite(BYTE slot, BYTE *dt = 0, size_t bcnt = 0);
    unsigned short getCrc(unsigned char *bp, size_t bcnt);

public:
    CYkLib(void);
    virtual ~CYkLib(void);

    unsigned short enumPorts(void);
    bool getPortName(unsigned short portIndex, wchar_t *dst, size_t dstSize);

    YKLIB_RC openKey(wchar_t *portName = 0);
    YKLIB_RC openKey(unsigned short portIndex);
    YKLIB_RC closeKey(void);

    YKLIB_RC readStatus(STATUS *sp);
    YKLIB_RC isSlotConfigured(unsigned char slot, STATUS *sp); 

	YKLIB_RC setKey160(CONFIG *cfg, const unsigned char *key);
	YKLIB_RC setMovingFactor(CONFIG *cfg, unsigned long seed);

    YKLIB_RC writeConfigBegin(unsigned char slot, CONFIG *cfg = 0, unsigned char *curPasswd = 0);

    YKLIB_RC readSerialBegin(void);

    YKLIB_RC writeChallengeBegin(unsigned char slot, YKLIB_CHAL_MODE mode, unsigned char *chal, size_t chalLen);

    YKLIB_RC abortPendingRequest(void);

    YKLIB_RC waitForCompletion(unsigned short maxWait = YKLIB_NO_WAIT, unsigned char *respBuffer = 0, unsigned char respLen = 0, unsigned short *timerVal = 0);
};
