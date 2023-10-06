The files in this directory were obtained on October 6, 2023 from the following
Chromium repo locations:
https://chromium.googlesource.com/chromiumos/platform/ec/+/refs/heads/main/LICENSE
https://chromium.googlesource.com/chromiumos/platform/ec/+/refs/heads/main/include/base32.h
https://chromium.googlesource.com/chromiumos/platform/ec/+/refs/heads/main/common/base32.c


The following changes were made to the files:
  * Convert to C++.
  * Convert to use RFC4648 base-32 character map.
  * Convert base32_encode/base32_decode to return...
        'true' for success
        'false' for failure
  * Apply fix for Chromium 303736599 (https://issuetracker.google.com/issues/303736599).