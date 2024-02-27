[![Gitpod Ready-to-Code](https://img.shields.io/badge/Gitpod-Ready--to--Code-blue?logo=gitpod)](https://gitpod.io/#https://github.com/pwsafe/pwsafe) 
[![mac-pwsafe](https://github.com/pwsafe/pwsafe/actions/workflows/main.yml/badge.svg)](https://github.com/pwsafe/pwsafe/actions/workflows/main.yml)

Introduction
============
Password Safe is a password database utility. Like many other such
products, commercial and otherwise, it stores your passwords in an
encrypted file, allowing you to remember only one password (the "master
password"), instead of all the username/password combinations that
you use.

What makes Password Safe different? Three things:
1. Simplicity: Password Safe is designed to do one thing, and to do it
well. Start the application, enter your "master password", double-click on
the entry and the password is now on your clipboard, ready for pasting.
2. Security: The original version was [designed by Bruce Schneier.](https://www.schneier.com/academic/passsafe/)
3. Open Source: The source code for the project is available for
inspection. For more information, see https://pwsafe.org.

The current version of Password Safe currently runs on Windows 7 and
later. Older versions are still available supporting Windows 95, 98,
ME, NT, 2000, XP, Vista as well as PocketPC.

The Mac version of Password Safe runs on macOS version 10.14 Mojave
and later, and runs native on Intel and Apple Silicon. (Universal binary.)

Linux packages are available for popular distributions. See
README.LINUX.md for more details.

FreeBSD is also supported. See README.FREEBSD.

Downloading
===========
The latest & greatest version of Password Safe may be downloaded from
[SourceForge](https://sourceforge.net/projects/passwordsafe/files/latest/download)
or
[GitHub](https://github.com/pwsafe/pwsafe/releases/latest).

Internationalization (Non-English Support)
==========================================
Thanks to the help of volunteers from all over the world, Password Safe
has built-in support for the following languages:
- Arabic
- Czech
- Chinese
- Danish
- Dutch
- French
- German
- Hungarian
- Italian
- Korean
- Polish
- Portuguese (Brazilian)
- Russian
- Slovenian
- Spanish
- Swedish
- Turkish

You can update the translations or add another language via [transifex](https://www.transifex.com/passwordsafe/passwordsafe/),
or by dropping [me](https://pwsafe.org/contact.php) a note.

Installation
============
For convenience, Password Safe is packaged into an executable
installer program. This installs the program, sets up shortcuts and
allows you to uninstall the program from the Windows Control
Panel. However, if you prefer, the -bin.zip file contains all the
files you need. Just extract the files (using WinZip, for example) to
any directory, double-click on the Pwsafe.exe icon,and that's
it. You may want to create a shortcut to your desktop and/or Start
menu. Finally, a Microsoft installer (.msi) package is also
available.

License
=======
Password Safe is available under the restrictions set forth in the
standard [Artistic License 2.0](https://opensource.org/license/artistic-2-0)". For more details, see the LICENSE
file that comes with the package.

Password Safe 3.29 and later makes use of the
[pugixml project](http://www.pugixml.org), which is available under
the [MIT license](http://www.opensource.org/licenses/mit-license.html).

Non-Windows versions of Password Safe make use of wxWidgets, under the
license terms defined in [https://wxwidgets.org/about/licence](https://wxwidgets.org/about/licence),
which is basically LGPL, with an exception stating that derived works in
binary form may be distributed on the user’s own terms.

Helping Out
===========
Please report bugs via the project's bug tracking form, on SourceForge at
https://sourceforge.net/p/passwordsafe/bugs/ or on Github at
https://github.com/pwsafe/pwsafe/issues. It's worth first
the bug list, to see if the issue hasn't already been reported.

Requests for features and enhancements should be submitted to the
Feature Requests page, at
https://sourceforge.net/p/passwordsafe/feature-requests/

You can also post questions, suggestions, rants, raves, etc. to the
Help or Open Discussion forums, at
https://sourceforge.net/p/passwordsafe/discussion/

If you wish to contribute to the project by writing code and/or
documentation, please drop a note to the developer's mailing list:
http://lists.sourceforge.net/lists/listinfo/passwordsafe-devel
or via the web form at https://pwsafe.org/contact.php

New releases are announced on the passwordsafe-announce mailing list:
http://lists.sourceforge.net/lists/listinfo/passwordsafe-announce

Last but not least: Password Safe is and will always remain an open
source project, free for use and redistribution. However, donations to the
project will help me justify the time and effort I spend in
maintaining and improving this utility. In addition, donations to the
project help maintain SourceForge, the hosting site. If you wish to
donate, please click on the Donate button at the top of
https://pwsafe.org

Release Notes
=============
For information on the latest features, bugfixes and known problems,
see [ReleaseNotes.md](docs/ReleaseNotes.md) for the Windows version and 
[ReleaseNotesWX.md](docs/ReleaseNotesWX.md) for the Linux and Mac versions.

Credits
=======
- The original version of Password Safe was designed by Bruce
Schneier, written by Mark Rosen, and was freely downloadable from
Conterpane Lab's website. Thanks to Mark for writing a great little
application! Following Mark, it was maintained by "AYB", at
Counterpane. Thanks to Counterpane for deciding to open source the
project.
- Jim Russell first brought the code to SourceForge, did some major
cleaning up of the code, set up a nice project and added some minor
features in release 1.9.0
- Releases 2.x have been brought to you by: Andrew Mullican,
Edward Quackenbush, Gregg Conklin, Graham Ullrich, and Rony
Shapiro. Karel (Karlo) Van der Gucht also contributed some of the
password policy code and some GUI improvements for 1.92.
- Emilijan Mirceski did the new graphics for 2.0.
- Maxim Tikhonov translated the first online help to Russian.
- Thanks to the Password Safe developer's mailing list for help in
nailing down the 3.x file format.
- Many, many new features in 3.x have been implemented by DK.
- Finally, thanks to the folks at SourceForge and GitHub for hosting
this project.

