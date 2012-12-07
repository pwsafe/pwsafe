https://sourceforge.net/tracker/?func=detail&atid=429582&aid=1517048&group_id=41019
Date: 2007-08-20 23:30
Sender: xenophonf
Logged In: YES 
user_id=822570
Originator: NO

Here's the source code to a MSI package for Password Safe 3.0.9; save this
as "pwsafe.wxs".   I hereby grant this file to the public domain.  You'll
also need the latest version (3.something) of the Windows Installer XML
toolkit (WIX), a version of the Artistic license in RTF format named
"LICENSE.rtf", and a copy of the files that get installed by the 3.0.9
setup program.  Copy everything into the same directory and run the
following commands to build the MSI:

path "%ProgramFiles%\Windows Installer XML v3\bin";%PATH%
candle pwsafe.wxs
light -ext WixUIExtension -cultures:en-us pwsafe.wixobj -out pwsafe.msi
