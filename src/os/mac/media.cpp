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
    CFIndex maxSize = CFStringGetMaximumSizeForEncoding(CFStringGetLength(mime_type), kCFStringEncodingUTF8) + 1;
    auto utf8str = new char[maxSize];
    auto success = CFStringGetCString(mime_type, utf8str, maxSize, kCFStringEncodingUTF8);
    if (success == false)
        return unknown_type;

    stringT retval = pws_os::towc(utf8str);
    CFRelease(mime_type);
    delete[](utf8str);
    return retval;
}
