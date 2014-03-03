These files are written by Yubico and are provided here with their permission.
The latest version of the files may be downloaded from:
http://static.yubico.com/var/uploads/files/YubikeyLib.zip
Notes:
1. The the Windows driver development kit needs to be installed in order to compile this library.
2. To get this to compile under VS2012, I had to remove sal.h from the DDK as described in
http://stackoverflow.com/questions/1356653/multiple-compiling-errors-with-basic-c-application-on-vs2010-beta-1/2312570#2312570

Note: sal.h does NOT have to be removed if using VS2013 and WDK 8.1.
FYI: In WDK 8.1, sal.h is normally installed in "C:\Program Files\Windows Kits\8.1\Include\shared" [32-bit Windows] or "C:\Program Files (x86)\Windows Kits\8.1\Include\shared" [64-bit WIndows] rather than "C:\WinDDK\7600.16385.1\inc\api\" for WDK 7.1A.
