/*****************************************************************************************
**																						**
**		O T P D E F  -  Common OTP settings                                             **
**																						**
**		Date		/ Rev		/ Sign	/ Remark										**
**		10-04-28	/ 1.0.0		/ J E	/ Main											**
**      11-07-07    / 1.0.1     / J E   / Changed prefix limits                         **
**																						**
*****************************************************************************************/

#ifndef	__OTPDEF_H_INCLUDED__
#define	__OTPDEF_H_INCLUDED__

#define PUBLIC_ID_STD_SIZE                  6
#define PUBLIC_ID_SIZE_OATH                 6 

#define MIN_OATH_CUSTOMER_PREFIX            1
#define MAX_OATH_CUSTOMER_PREFIX            9999

#define MIN_YUBICO_CUSTOMER_PREFIX          1
#define MAX_YUBICO_CUSTOMER_PREFIX          0xffff

#define MAX_YUBICO_SERIAL                   0x00ffffffUL
#define MAX_OATH_SERIAL                     99999UL

#define YUBICO_OTP_CUSTOMER_PREFIX_CODE     0x28
#define OATH_HOTP_CUSTOMER_PREFIX_START     190

// Reserved for upload to the Yubico server

#define YUBICO_SERVER_UPLOAD_PREFIX         0xff

// openauthentication.org assigned manufacturer prefix

#define YUBICO_OATH_MFG_PREFIX              "ub"
#define YUBICO_OMP_CODE                     0xe1        // Modhex equivalent

#endif		// __OTPDEF_H_INCLUDED__