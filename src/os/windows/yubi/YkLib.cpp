/*************************************************************************
**                                                                      **
**      Y K L I B  -  Unified Yubikey low-level interface library       **
**                                                                      **
**      Copyright 2011 - Yubico AB                                      **
**                                                                      **
**      Date   / Sig / Rev  / History                                   **
**      110329 / J E / 0.00 / Main                                      **
**      111011 / J E / 0.00 / Fixed memory leak flushPortNames			**
**		111111 / J E / 0.90 / Added HOTP helper functions + release		**
**		111121 / J E / 0.91 / Fixed bug with helper functions           **
**		130205 / J E / 0.92 / Added support for NEO                     **
**                                                                      **
*************************************************************************/

#include "StdAfx.h"
#include "YkLib.h"
#include <string.h>
#include <stdlib.h>

extern "C" {
#include <initguid.h>
#include <setupapi.h>
#include <hidsdi.h>
#include <ntddkbd.h>
};

#define	FEATURE_RPT_SIZE	    9
#define FEATURE_PAYLOAD_SIZE    7

#define CRC_OK_RESIDUE          0xf0b8

CYkLib::CYkLib(void)
{
	m_handle = INVALID_HANDLE_VALUE;
    m_portList = 0;
    m_lastCmd = 0;
}

CYkLib::~CYkLib(void)
{
    closeKey();
    flushPortNames();
}

/*************************************************************************
**  private function flushPortNames                                     **
**  Flushes internal list of ports                                      **
**                                                                      **
**  void flushPortNames(void)                                           **
**                                                                      **
*************************************************************************/

void CYkLib::flushPortNames(void)
{
    struct tagPORT_LIST *tmp;

    while (m_portList) {
        tmp = m_portList->next;
        delete [] m_portList->name;
		delete m_portList;
        m_portList = tmp;
    }
}

/*************************************************************************
**  private function rawWrite                                           **
**  Send a raw packet to device without waiting for completion          **
**                                                                      **
**  YKLIB_RC rawWrite(BYTE cmd, BYTE *dt, size_t bcnt)                  **
**                                                                      **
**  Where:                                                              **
**  "cmd" is command byte                                               **
**  "dt" is pointer to command data                                     **
**  "bcnt" is number of bytes in command                                **
**                                                                      **
**  Returns: Status return code                                         **
**                                                                      **
*************************************************************************/

YKLIB_RC CYkLib::rawWrite(BYTE cmd, BYTE *dt, size_t bcnt)
{
	if (m_handle == INVALID_HANDLE_VALUE) return YKLIB_NOT_OPENED;
    if (bcnt > SLOT_DATA_SIZE) return YKLIB_INVALID_PARAMETER;        

	BYTE rptBuf[FEATURE_RPT_SIZE];
    YKFRAME frame;
	int i, j;
    YKLIB_RC rc;
    STATUS status;

    // Read status at start

    if ((rc = readStatus(&status)) != YKLIB_OK) return rc;

    // Check that command is supported by current firmware

    if (cmd > SLOT_CONFIG2)
        if (status.versionMajor < 2 || (status.versionMajor == 2 && status.versionMinor < 2)) return YKLIB_UNSUPPORTED_FEATURE;

    // Keep last sent command and sequence number 

    m_lastCmd = cmd;
    m_seq = status.pgmSeq;

	// Ensure non used bytes are cleared

	memset(&frame, 0, sizeof(frame));

	// Copy in data to be written

	if (bcnt && dt) memcpy(frame.payload, dt, bcnt);

	// Insert slot/cmd #

	frame.slot = cmd;

	// Insert CRC

    frame.crc = getCrc(frame.payload, sizeof(frame.payload));

	// Write the reports

    for (i = 0; ; i++) {

		// Copy appropriate slice of the frame

		memcpy(rptBuf + 1, ((BYTE *) &frame) + i * FEATURE_PAYLOAD_SIZE, FEATURE_PAYLOAD_SIZE);

		// Empty slices in the middle of the frame can be discarded

        if (i && (i < ((sizeof(frame) / FEATURE_PAYLOAD_SIZE) - 1))) {
			for (j = 0; j < FEATURE_PAYLOAD_SIZE; j++) if (rptBuf[1 + j]) break;
			if (j >= FEATURE_PAYLOAD_SIZE) continue;
		}

		// Insert Win32 report #

		rptBuf[0] = 0;
		rptBuf[8] = (BYTE) i | 0x80;

		// Write report

        Sleep(2);
		if (!HidD_SetFeature(m_handle, rptBuf, FEATURE_RPT_SIZE)) return YKLIB_FAILURE;

        // Return if last report written without checking completion status

        if (i >= ((sizeof(frame) / FEATURE_PAYLOAD_SIZE) - 1)) return YKLIB_OK;

        // Check that slice has been stored before sending next. This operation shall be instant

        memset(rptBuf, 0, sizeof(rptBuf));
        if (!HidD_GetFeature(m_handle, rptBuf, sizeof(rptBuf))) return YKLIB_FAILURE;
	}
}

/*************************************************************************
**  function getCrc                                                     **
**  Calculate ISO13239 checksum of buffer                               **
**                                                                      **
**  unsigned short getCrc(unsigned char *bp, size_t bcnt)               **
**                                                                      **
**  Where:                                                              **
**  "bp" is pointer to data                                             **
**  "bcnt" is number of bytes in buffer                                 **
**                                                                      **
**  Returns: CRC                                                        **
**                                                                      **
*************************************************************************/

unsigned short CYkLib::getCrc(unsigned char *bp, size_t bcnt)
{
    unsigned short crc = 0xffff;
	int i, j;

    while (bcnt--) {
	    crc ^= *bp++;
	    for (i = 0; i < 8; i++) {
            j = crc & 1;
            crc >>= 1;
            if (j) crc ^= 0x8408;
	    }
    }

    return crc;
}

/*************************************************************************
**  function enumPorts                                                  **
**  Scans all present ports for attached Yubikeys and keeps the list    **
**                                                                      **
**  unsigned short enumPorts(void)                                      **
**                                                                      **
**  Returns: Number of ports with Yubikeys found                        **
**                                                                      **
*************************************************************************/

unsigned short CYkLib::enumPorts(void)
{
    flushPortNames();

    // Find all currently available Yubikeys

    HDEVINFO hi;
    SP_DEVICE_INTERFACE_DATA di;
    PSP_DEVICE_INTERFACE_DETAIL_DATA_W pi;
    HIDD_ATTRIBUTES devInfo;
    int i;
    DWORD len, rc;
    struct tagPORT_LIST *tmp;
    unsigned short cnt = 0;

    flushPortNames();

	// Get the collection of present Yubikey devices

	hi = SetupDiGetClassDevs(&GUID_DEVINTERFACE_KEYBOARD, 0, 0, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
	if (hi == INVALID_HANDLE_VALUE) return (unsigned short)-1;

	di.cbSize = sizeof (SP_DEVICE_INTERFACE_DATA);

	// Traverse the list (put a cap if something for some reason would go wrong - just feels good to do that...)

	for (i = 0; i < 1000; i++) {
         if (!SetupDiEnumDeviceInterfaces(hi, 0, &GUID_DEVINTERFACE_KEYBOARD, i, &di)) break;

		// Get size required (the call shall fail as there is no buffer - but the len member shall be filled in)

		SetupDiGetDeviceInterfaceDetail(hi, &di, 0, 0, &len, 0);

		if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) break;

		// Allocate a buffer of appropriate size

		pi = (PSP_DEVICE_INTERFACE_DETAIL_DATA_W) new BYTE[len];
		pi->cbSize = sizeof (SP_DEVICE_INTERFACE_DETAIL_DATA);

		// Make the "real" call and get the path to the device

		rc = SetupDiGetDeviceInterfaceDetailW(hi, &di, pi, len, &len, 0);

		// Success ?

		if (rc) {

			// Try to open device

			m_handle = CreateFileW(pi->DevicePath, GENERIC_WRITE,
                  FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);	

			// Success

			if (m_handle != INVALID_HANDLE_VALUE) {

				// Get HID attributes

				if (HidD_GetAttributes(m_handle, &devInfo)) {
					if (devInfo.VendorID == YUBICO_VID) {
            // Assume every Yubico device is good for us...
                        // Keep full path of device found

                        tmp = new tagPORT_LIST;

                        tmp->name = new wchar_t[lstrlenW(pi->DevicePath) + 1];
                        lstrcpyW(tmp->name, pi->DevicePath);
                        tmp->next = m_portList;
                        m_portList = tmp;

                        cnt++;
					}
				}

				CloseHandle(m_handle);
				m_handle = INVALID_HANDLE_VALUE;
			}
		} 

		// Free allocated buffer

		delete pi;

		// If failed, bail out

		if (!rc) break;
	}

	// Free list

	SetupDiDestroyDeviceInfoList(hi);

    return cnt;
}

/*************************************************************************
**  function getPortName                                                **
**  Returns port name from a prior enumPorts call                       **
**                                                                      **
**  wchar_t *getPortName(unsigned short index)                          **
**                                                                      **
**  Where:                                                              **
**  "portIndex" is index in list 0..                                    **
**                                                                      **
**  Returns: Pointer to port name if found, NULL otherwise              **
**                                                                      **
*************************************************************************/

wchar_t *CYkLib::getPortName(unsigned short portIndex)
{
    struct tagPORT_LIST *tmp = m_portList;

    while (tmp) {
        if (!portIndex) return tmp->name;
        tmp = tmp->next;
        portIndex--;
    }
 
    return 0;
}

/*************************************************************************
**  function getPortName                                                **
**  Returns port name from a prior enumPorts call                       **
**                                                                      **
**  bool getPortName(unsigned short index, wchar_t *dst,                **
**                           size_t dstSize)                            **
**                                                                      **
**  Where:                                                              **
**  "portIndex" is index in list 0..                                    **
**  "dst" is pointer to destination buffer                              **
**  "dstSize" is size of destination buffer                             **
**                                                                      **
**  Returns: TRUE if valid port name returned, FALSE otherwise          **
**                                                                      **
*************************************************************************/

bool CYkLib::getPortName(unsigned short portIndex, wchar_t *dst, size_t dstSize)
{
    wchar_t *tmp = getPortName(portIndex);

    if (tmp) {
        lstrcpynW(dst, tmp, (int) dstSize);
        return true;
    }

    return false;
}

/*************************************************************************
**  function openKey                                                    **
**  Opens a present Yubikey                                             **
**                                                                      **
**  YKLIB_RC openKey(wchar_ t *portName)                                **
**                                                                      **
**  Where:                                                              **
**  "portName" is name of specific port. NULL for all ports              **
**                                                                      **
**  Returns: Status return code                                         **
**                                                                      **
*************************************************************************/

YKLIB_RC CYkLib::openKey(wchar_t *portName)
{
    int i;

    // Make sure previous is closed first

    closeKey();

    // If no port name specified, search all ports

    if (!portName) {
        i = enumPorts();
        if (!i) return YKLIB_NO_DEVICE;
        if (i != 1) return YKLIB_MORE_THAN_ONE;
        portName = m_portList->name;
    }

    // Open port

	m_handle = CreateFileW(portName, GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, 0, OPEN_EXISTING, 0, 0);	

    return (m_handle == INVALID_HANDLE_VALUE) ? YKLIB_FAILURE : YKLIB_OK;
}

/*************************************************************************
**  function openKey                                                    **
**  Opens a present Yubikey                                             **
**                                                                      **
**  YKLIB_RC openKey(unsigned short portIndex)                          **
**                                                                      **
**  Where:                                                              **
**  "portIndex" is index from a previous enumPorts call                 **
**                                                                      **
**  Returns: Status return code                                         **
**                                                                      **
*************************************************************************/

YKLIB_RC CYkLib::openKey(unsigned short portIndex)
{
    wchar_t buf[_MAX_PATH];

    if (!getPortName(portIndex, buf, sizeof(buf) / sizeof(buf[0]))) return YKLIB_INVALID_PARAMETER;

    closeKey();

    return openKey(buf);
}

/*************************************************************************
**  function closeKey                                                   **
**  Closes an instance of a previously opened key                       **
**                                                                      **
**  YKLIB_RC closeKey(void)                                             **
**                                                                      **
**  Returns: Status return code                                         **
**                                                                      **
*************************************************************************/

YKLIB_RC CYkLib::closeKey(void)
{
    CloseHandle(m_handle);

    m_handle = INVALID_HANDLE_VALUE;
    m_lastCmd = 0;

    return YKLIB_OK;
}

/*************************************************************************
**  function readStatus                                                 **
**  Reads status from a present opened Yubikey                          **
**                                                                      **
**  YKLIB_RC readStatus(STATUS *status)                                 **
**                                                                      **
**  Where:                                                              **
**  "status" is pointer to receiving STATUS structure                   **
**                                                                      **
**  Returns: Status return code                                         **
**                                                                      **
*************************************************************************/

YKLIB_RC CYkLib::readStatus(STATUS *status)
{
    if (m_handle == INVALID_HANDLE_VALUE) return YKLIB_NOT_OPENED;

    // Read status record

	BYTE buf[FEATURE_RPT_SIZE];

	memset(buf, 0, sizeof(buf));

	if (!HidD_GetFeature(m_handle, buf, FEATURE_RPT_SIZE)) return YKLIB_FAILURE;

	status->versionMajor = buf[2];
	status->versionMinor = buf[3];
	status->versionBuild = buf[4];
	status->pgmSeq = buf[5];
	status->touchLevel = buf[6] + ((WORD) buf[7] << 8U);

    return (buf[FEATURE_RPT_SIZE - 1]) ? YKLIB_NOT_READY : YKLIB_OK;
}

/*************************************************************************
**  function isSlotConfigured                                           **
**  Checks if a specified slot is configured or not                     **
**                                                                      **
**  YKLIB_RC isSlotConfigured(unsigned char slot, STATUS *status)       **
**                                                                      **
**  Where:                                                              **
**  "slot" is slot number 0..                                           **
**  "status" is pointer to receiving STATUS structure                   **
**                                                                      **
**  Returns: Status return code                                         **
**                                                                      **
*************************************************************************/

YKLIB_RC CYkLib::isSlotConfigured(unsigned char slot, STATUS *status)
{
    if (m_handle == INVALID_HANDLE_VALUE) return YKLIB_NOT_OPENED;

    // Feature available from version 2.1

    if (status->versionMajor < 2 || (status->versionMajor == 2 && status->versionMinor < 1)) return YKLIB_UNSUPPORTED_FEATURE;

    return (status->touchLevel & ((slot == YKLIB_FIRST_SLOT) ? 1 : 2)) ? YKLIB_OK : YKLIB_NOT_CONFIGURED;
}

/*************************************************************************
**  function writeConfigBegin                                           **
**  Start a configuration slot write                                    **
**                                                                      **
**  YKLIB_RC writeConfigBegin(unsigned char slot, CONFIG *cfg,          **
**                            bool kill, unsigned char *curPasswd)      **
**                                                                      **
**  Where:                                                              **
**  "slot" is slot number 0..                                           **
**  "cfg" is pointer Yubikey CONFIG structure. NULL kills config        **
**  "curAccCode" is pointer to current access password (if any)         **
**                                                                      **
**  Returns: Status return code                                         **
**                                                                      **
*************************************************************************/

YKLIB_RC CYkLib::writeConfigBegin(unsigned char slot, CONFIG *cfg, unsigned char *curAccCode)
{
    YKLIB_RC rc;
    #pragma pack(push, 1)
    struct {
        CONFIG cfg;
        BYTE accCode[ACC_CODE_SIZE];
    } blk;
    #pragma pack(pop)

    // Setup configuration block + CRC if valid. Zap otherwise

    memset(&blk, 0, sizeof(blk));

    if (cfg) {
        memcpy(&blk.cfg, cfg, sizeof(blk.cfg));
        blk.cfg.crc = ~getCrc((BYTE *) &blk.cfg, sizeof(blk.cfg) - sizeof(blk.cfg.crc));
    }

    // Insert access code if present

    if (curAccCode) memcpy(blk.accCode, curAccCode, sizeof(blk.accCode));

    // Start write request

    rc = rawWrite(slot ? SLOT_CONFIG2 : SLOT_CONFIG, (BYTE *) &blk, sizeof(blk));

    // Setup expected sequence # for completion

    m_seq = cfg ? (m_seq + 1) : 0;

    return rc;
}

/*************************************************************************
**  function readSerialBegin                                            **
**  Start a serial number read                                          **
**                                                                      **
**  YKLIB_RC readSerialBegin(void)                                      **
**                                                                      **
**  Returns: Status return code                                         **
**                                                                      **
*************************************************************************/

YKLIB_RC CYkLib::readSerialBegin(void)
{
    return rawWrite(SLOT_DEVICE_SERIAL);
}

/*************************************************************************
**  function writeChallengeBegin                                        **
**  Start a serial number read                                          **
**                                                                      **
**  YKLIB_RC writeChallengeBegin(unsigned char slot,                    **
**                      YKLIB_CHAL_MODE mode,                           **
**                      unsigned char *chal, size_t chalLen)            **
**                                                                      **
**  Where:                                                              **
**  "slot" is slot number 0..                                           **
**  "mode" is challenge mode YKLIB_OTP or YKLIB_HMAC                    **
**  "chal" is pointer to challenge                                      **
**  "chalLen" is number of bytes in challenge                           **
**                                                                      **
**  Returns: Status return code                                         **
**                                                                      **
*************************************************************************/

YKLIB_RC CYkLib::writeChallengeBegin(unsigned char slot, YKLIB_CHAL_MODE mode, unsigned char *chal, size_t chalLen)
{
    if (mode == YKLIB_CHAL_OTP)
        slot = (slot == YKLIB_FIRST_SLOT) ? SLOT_CHAL_OTP1 : SLOT_CHAL_OTP2;
    else if (mode == YKLIB_CHAL_HMAC)
        slot = (slot == YKLIB_FIRST_SLOT) ? SLOT_CHAL_HMAC1 : SLOT_CHAL_HMAC2;
    else 
        return YKLIB_INVALID_PARAMETER;

    // Send challenge. 

    return rawWrite(slot, chal, chalLen);
}

/*************************************************************************
**  function abortPendingRequest                                        **
**  Abort a pending request (if any)                                    **
**                                                                      **
**  YKLIB_RC abortPendingRequest(void)                                  **
**                                                                      **
**  Returns: Status return code                                         **
**                                                                      **
*************************************************************************/

YKLIB_RC CYkLib::abortPendingRequest(void)
{
    STATUS status;
    YKLIB_RC rc;

    // Check status first

    if ((rc = readStatus(&status)) != YKLIB_NOT_READY) return rc;

    // Send abort request

	BYTE rptBuf[FEATURE_RPT_SIZE];
    
    memset(rptBuf, 0, sizeof(rptBuf));
    rptBuf[FEATURE_RPT_SIZE - 1] = DUMMY_REPORT_WRITE;

    if (!HidD_SetFeature(m_handle, rptBuf, FEATURE_RPT_SIZE)) return YKLIB_FAILURE;

    // Check status again

    return readStatus(&status);
}

/*************************************************************************
**  function waitForCompletion                                          **
**  Wait for an asynchronous call to complete                           **
**                                                                      **
**  YKLIB_RC waitForCompletion(unsigned short maxWait,                  **
**                  unsigned char *respBuffer, unsigned char respLen,   **
**                  unsigned short *timerVal)                           **
**                                                                      **
**  Where:                                                              **
**  "maxWait" is max number of milliseconds to wait. Zero if none       **
**  "respBuffer" is pointer to response buffer (if applicable)          **
**  "respLen" is exact number of bytes expected                         **
**  "timerVal" is a pointer to field receiving timer val (if applicable)**
**                                                                      **
**  Returns: Status return code                                         **
**                                                                      **
*************************************************************************/

YKLIB_RC CYkLib::waitForCompletion(unsigned short maxWait, unsigned char *respBuffer, unsigned char respLen, unsigned short *timerVal)
{
	BYTE rptBuf[FEATURE_RPT_SIZE];
    DWORD tend = GetTickCount() + maxWait;

    for (;;) {

        // Query status

        memset(rptBuf, 0, sizeof(rptBuf));
        if (!HidD_GetFeature(m_handle, rptBuf, sizeof(rptBuf))) return YKLIB_FAILURE;

        // Return with ok if completed

        if (!rptBuf[FEATURE_RPT_SIZE - 1]) {

            // No response found, return with error if response requested

            if (respLen) return YKLIB_INVALID_RESPONSE;

            // If a write is pending, verify that the sequence number is correct

            if (m_lastCmd == SLOT_CONFIG || m_lastCmd == SLOT_CONFIG2)
            	if (m_seq != rptBuf[5]) return YKLIB_WRITE_ERROR;

            return YKLIB_OK;
        }

        // If first response pending, read in all response bytes

        if (rptBuf[FEATURE_RPT_SIZE - 1] & RESP_PENDING_FLAG) {

            // Continue to read until response flag is reset

            BYTE tempBuf[RESP_ITEM_MASK * FEATURE_PAYLOAD_SIZE + sizeof(WORD)], slices = 0, bail = 0;

            for (;;) {

                // Safeguard to prevent hang (shall never happen)

                if (++bail > (2 * RESP_ITEM_MASK)) {
                    memset(rptBuf, 0, sizeof(rptBuf));
                    rptBuf[FEATURE_RPT_SIZE - 1] = DUMMY_REPORT_WRITE;
                    if (!HidD_SetFeature(m_handle, rptBuf, FEATURE_RPT_SIZE)) return YKLIB_FAILURE;

                    return YKLIB_INVALID_RESPONSE;
                }

                // Store at the right place

                memcpy(tempBuf + (rptBuf[FEATURE_RPT_SIZE - 1] & RESP_ITEM_MASK) * FEATURE_PAYLOAD_SIZE, rptBuf + 1, FEATURE_PAYLOAD_SIZE);

                // Indicate that a particular slice has been stored

                slices |= (1 << (rptBuf[FEATURE_RPT_SIZE - 1] & RESP_ITEM_MASK));
                   
                // Read again/next

                memset(rptBuf, 0, sizeof(rptBuf));
                if (!HidD_GetFeature(m_handle, rptBuf, sizeof(rptBuf))) return YKLIB_FAILURE;

                // Check if completed

                if (!rptBuf[FEATURE_RPT_SIZE - 1]) break;

                // Return with error if invalid response

                if (!(rptBuf[FEATURE_RPT_SIZE - 1] & RESP_PENDING_FLAG)) return YKLIB_INVALID_RESPONSE;
            } 

            // Check that parameters are ok

            if (!respLen || (respLen > sizeof(tempBuf))) return YKLIB_INVALID_RESPONSE;
            if (slices != ((1 << (respLen + sizeof(WORD) + FEATURE_PAYLOAD_SIZE - 1) / FEATURE_PAYLOAD_SIZE) - 1)) return YKLIB_INVALID_RESPONSE;

            // Verify checksum

            if (getCrc(tempBuf, respLen + sizeof(unsigned short)) != CRC_OK_RESIDUE) return YKLIB_INVALID_RESPONSE;
                
            memcpy(respBuffer, tempBuf, respLen);

            return YKLIB_OK;
        }

        // Timer/timeout pending ?

        if (rptBuf[FEATURE_RPT_SIZE - 1] & RESP_TIMEOUT_WAIT_FLAG) {
            if (timerVal) *timerVal = rptBuf[FEATURE_RPT_SIZE - 1] & RESP_TIMEOUT_WAIT_MASK;
            return YKLIB_TIMER_WAIT;
        }

        // Still processing, or... ?

        if (!(rptBuf[FEATURE_RPT_SIZE - 1] & SLOT_WRITE_FLAG)) return YKLIB_FAILURE;

        // Timeout ?

        if (GetTickCount() >= tend) return YKLIB_PROCESSING;

        // Short wait

        Sleep(10);
    }
} 

/*************************************************************************
**  function setKey160													**
**  Assigns a 160-bit HOTP/HMAC-SHA1 key into a configuration structure	**
**                                                                      **
**  YKLIB_RC setKey160(CONFIG *cfg, const unsigned char *key)           **
**                                                                      **
**  Where:                                                              **
**  "cfg" is pointer to destination configuration structure				**
**  "key" is pointer to 160-bit (20 bytes) key							**
**                                                                      **
**  Returns: Status return code                                         **
**                                                                      **
*************************************************************************/

YKLIB_RC CYkLib::setKey160(CONFIG *cfg, const unsigned char *key)
{
    memcpy(cfg->key, key, sizeof(cfg->key));
    memcpy(cfg->uid, key + sizeof(cfg->key), KEY_SIZE_OATH - KEY_SIZE);

	return YKLIB_OK;
}

/*************************************************************************
**  function setMovingFactor											**
**  Assigns a HOTP moving factor into a configuration structure         **
**                                                                      **
**  YKLIB_RC setMovingFactor(CONFIG *cfg, unsigned long seed)           **
**                                                                      **
**  Where:                                                              **
**  "cfg" is pointer to destination configuration structure				**
**  "seed" is moving factor. Must be even divisible by 16               **
**                                                                      **
**  Returns: Status return code                                         **
**                                                                      **
*************************************************************************/

YKLIB_RC CYkLib::setMovingFactor(CONFIG *cfg, unsigned long seed)
{
    if (seed & 0xf) return YKLIB_INVALID_PARAMETER;

    cfg->uid[4] = (BYTE) (seed >> 12);
    cfg->uid[5] = (BYTE) (seed >> 4);

    return YKLIB_OK;
}
