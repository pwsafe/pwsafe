/**
* \file Mac-specific implementation of media.h
*/

#include <CoreServices/CoreServices.h>
#include <CoreFoundation/CoreFoundation.h>

#include "../media.h"
#include "../utf8conv.h"

using namespace std;

static const stringT unknown_type = L"unknown";

stringT pws_os::GetMediaType(const stringT & sfilename)
{
    // Convert stringT to CFStringRef
    CFStringRef filename = CFStringCreateWithCString(kCFAllocatorDefault, pws_os::tomb(sfilename).c_str(), kCFStringEncodingUTF8);
    if (filename == NULL)
        return unknown_type;

    // Extract the file extension
    CFRange range = CFStringFind(filename, CFSTR("."), kCFCompareBackwards);
    if (range.location == kCFNotFound)
        return unknown_type;
    CFStringRef fileExtension = CFStringCreateWithSubstring(kCFAllocatorDefault, filename, CFRangeMake(range.location + 1, CFStringGetLength(filename) - range.location - 1));
    CFRelease(filename);
    if (fileExtension == NULL)
        return unknown_type;

    // Determine uniform type identifier
    CFStringRef uti = UTTypeCreatePreferredIdentifierForTag(kUTTagClassFilenameExtension, fileExtension, NULL);
    CFRelease(fileExtension);
    if (uti == NULL)
        return unknown_type;

    // Get mime type from UTI
    CFStringRef mime_type = UTTypeCopyPreferredTagWithClass(uti, kUTTagClassMIMEType);
    CFRelease(uti);
    if (mime_type == NULL)
        return unknown_type;

    // Convert mime_type to stringT
    stringT retval = pws_os::towc(CFStringGetCStringPtr(mime_type, kCFStringEncodingUTF8));
    CFRelease(mime_type);
    return retval;
}
