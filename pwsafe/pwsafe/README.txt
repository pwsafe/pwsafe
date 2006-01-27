$Id$

Introduction:
=============
Password Safe is a password database utility. Like many other such
products, commercial and otherwise, it stores your passwords in an
encrypted file, allowing you to remember only one password (the "safe
combination"), instead of all the username/password combinations that
you use.

What makes Password Safe different? Three things:
1. Simplicity: Password Safe is designed to do one thing, and to do it
well. Start the application, enter your "combination", double-click on
the right entry - presto - the password is now on your clipboard,ready
for pasting.
2. Security: The original version was designed and written by Bruce
Schneier - 'nuff said.
3. Open Source: The source code for the project is available for
inspection. For more information, see http://passwordsafe.sourceforge.net

Password Safe currently runs on Windows 95, 98, ME, NT4, 2000 and
XP, as well as Microsoft's PocketPC. Support for additional platforms
is planned for future releases. 

Downloading:
============
The latest & greatest version of Password Safe may be downloaded from
https://sourceforge.net/project/showfiles.php?group_id=41019

Non-English Versions:
=====================
People have volunteered to make PasswordSafe available in several
languages, including: Spanish, French, Italian, Finnish, Russian and
German. Please see http://passwordsafe.sourceforge.net/#NonEnglish for the
current list.

If anyone is interested in doing such work in other
languages, please let me (https://sourceforge.net/users/ronys) know.

Installation:
=============
Nothing special. No "Setup", no dependencies, no annoying wizard, no
need to sacrifice a goat and/or reboot your computer. Just extract the
files (using WinZip, for example) to any directory, double-click on
the Pwsafe.exe icon,and that's it. "Advanced" users may want to
create a shortcut to their desktop and/or Start menu.

Note: Versions 2.15 and later require newer versions of Microsoft's runtime
libraries. If PasswordSafe complains about a "Missing DLL", you can get the
DLL from http://passwordsafe.sourceforge.net/redist.zip - just unzip it and
place in in C:\Windows\system32. Alternately, use the install-kit version
of the distribution to place this dll for you.

License:
========
Password Safe is available under the restrictions set forth in the
standard "Artistic License". For more details, see the LICENSE file
that comes with the package.

Helping Out:
============
Please report bugs via the project's bug tracking form, at
https://sourceforge.net/tracker/?group_id=41019&atid=429579 (you might
want to browse the list, to see if the issue hasn't been reported
already).

Requests for features and enhancements should be submitted to the RFE
(Request For Enhancements) page, at
https://sourceforge.net/tracker/?group_id=41019&atid=429582

You can also post questions, suggestions, rants, raves, etc. to the
Help or Open Discussion forums, at
https://sourceforge.net/forum/?group_id=41019

If you wish to contribute to the project by writing code and/or
documentation, please drop a note to the developer's mailing list:

http://lists.sourceforge.net/lists/listinfo/passwordsafe-devel

New releases are announced on the passwordsafe-announce mailing list:

http://lists.sourceforge.net/lists/listinfo/passwordsafe-announce)

Last but not least: PasswordSafe is and will always remain an open
source project, free for and redistribution. However, donations to the
project will help me justify the time and effort I spend in
maintaining and improving this utility. In addtion, donations to the
project help maintain SourceForge, the hosting site. If you wish to
donate, please follow this link:
https://sourceforge.net/donate/index.php?group_id=41019

Release Notes:
==============
For information on the latest features, bugfixes and known problems,
see the ReleaseNotes.txt file that comes with the package.

Credits:
========
- The original version of Password Safe was written by Mark Rosen,
and was freely downloadable from Conterpane Lab's website. Thanks to
Mark for writing a great little application! Following Mark, it was
maintained by "AYB", at Counterpane. Thanks to Counterpane for
deciding to open source the project.
- Jim Russell first brought the code to SourceForge, did some major
cleaning up of the code, set up a nice project and added some minor
features in release 1.9.0
- The current release has been brought to you by: Andrew Mullican,
Edward Quackenbush, Gregg Conklin, Graham Ullrich, and Rony
Shapiro. Karel (Karlo) Van der Gucht also contributed some of the
password policy code and some GUI improvements for 1.92.
- Emilijan Mirceski did the new graphics for 2.0.
- Maxim Tikhonov translated the online help to Russian.
- DK has contributed many bugfixes and refinements.
- Finally, thanks to the folks at SourceForge for hosting this
project.

$Log$
Revision 1.9.2.1  2006/01/27 06:21:44  ronys
V2.16 release

Revision 1.9  2005/01/15 11:07:14  ronys
Update non-English list, add reference to donations.

Revision 1.8  2004/11/24 16:21:13  ronys
Added link to Italian help, plus unix2dos'ed

Revision 1.7  2004/09/25 08:04:31  ronys
Added pointer to French help file.

Revision 1.6  2004/03/31 04:44:31  ronys
Announce availability of German version of online help

Revision 1.5  2004/02/17 21:11:06  ronys
Updated for 2.0

Revision 1.4  2003/05/28 08:29:04  ronys
Corrected credits

Revision 1.3  2003/05/14 14:49:55  ronys
post-1.92 merge into main trunk

Revision 1.2.2.1  2003/05/13 13:40:13  ronys
1.92 release

Revision 1.2  2003/04/30 13:20:14  ronys
Listed supported platforms

Revision 1.1  2003/04/29 14:22:32  ronys
First versions of README and Release Notes under CVS for 1.91
