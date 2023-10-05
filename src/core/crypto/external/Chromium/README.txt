The files in this directory were obtained on October 1, 2023 from the following
Chromium repo locations:
https://chromium.googlesource.com/chromiumos/platform/ec/+/master/LICENSE
https://chromium.googlesource.com/chromiumos/platform/ec/+/master/include/base32.h
https://chromium.googlesource.com/chromiumos/platform/ec/+/master/common/base32.c

The following changes were made to the files:
  * Convert to C++.
  * Convert to use RFC3548 base-32 character map.
  * Convert base32_encode/base32_decode to return...
        'true' for success
        'false' for failure
