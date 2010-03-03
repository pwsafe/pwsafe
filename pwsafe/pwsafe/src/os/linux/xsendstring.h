
/*
 * xsendstring.h - interface for sending a random string as keyboard input
 * to the X application in focus
 */

#ifndef __XSENDSTRING_H__
#define __XSENDSTRING_H__

/* Set the method to AUTO if you're not sure what it should be */
typedef enum { ATMETHOD_AUTO, ATMETHOD_XTEST, ATMETHOD_XSENDKEYS } AutotypeMethod;

void SendString(const char* str, AutotypeMethod method, unsigned delayMS);

#endif

