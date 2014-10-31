This document explains how to build PasswordSafe on Mac OS X.  It
is organized in these sections

1. Requirements
2. Get PasswordSafe Sources
3. wxWidgets
   3.1 Downloading wxWidgets sources
   3.2 Which version of wxWidgets?
   3.3 Building wxWidgets for pwsafe
   3.4 Point Xcode to your wxWidgets build
4. Build PassswordSafe
   4.1 Xcode project files
   4.2 Debug and Release configs
   4.3 Re-distribution
   4.4 Where is pwsafe.app?
   4.5 Checking binary types
5. Building on Old Platforms (10.6-)


1. REQUIREMENTS
===============

1.1 Xcode
1.2 wxWidgets
1.3 Perl

1.1 Xcode
---------
Xcode is free and is included with the OS X installation CD.  If you don't have the CD,
you can probably download Xcode from Apple's website.  You will also need the
"Command-Line Tools for Xcode".

The Xcode directory in PasswordSafe sources contains the Xcode project file for building it.

1.2 wxWidgets
-------------
wxWidgets is the cross-platform UI toolkit pwsafe uses for user-interface.  You'd need to
download the latest sources from wxWidgets.org and build it (instructions below).  This is
the most time-consuming part of building pwsafe.


1.3 Perl
--------
pwsafe uses perl for some small build tasks.  OS X already ships with Perl, which should 
suffice.


2. GET PASSWORDSAFE SOURCES
===========================

You need to get the Passwordsafe Sources (obviously).  Either download from the website:

http://sourceforge.net/projects/passwordsafe/files/passwordsafe/3.34/pwsafe-3.34.1-src.zip
(this may not be the latest version)

or clone the git repository:

git clone git://git.code.sf.net/p/passwordsafe/git-code pwsafe

3. wxWidgets
============

wxWidgets is the UI toolkit used by pwsafe for user-interface.  On OS X, you will have
to download and build this before you can build pwsafe.


3.1 Downloading the sources
---------------------------

I recommend you download the tarball from wxWidgets download site

https://www.wxwidgets.org/downloads/

Click the link named "Source for Linux, OS X, etc".  It should get you a .tar.bz2
or a .tar.gz file.  DO NOT get the .7z version.  That has sources with DOS CRLF 
and won't build on OS X.  Also, I have found some (may be irrelevant) inconsistencies
between the sources checked out from their repository at a particular branch/tag, and
the tarball.  My recommendation is to use the tarball.  That's what I always do on OS X.


3.2 Which version of wxWidgets?
-------------------------------

At the time of this writing, wxWidgets has two mainline distributions, 2.8.X and 3.0.X.
The 2.8.X is now in a maintenance mode and won't see any changes short of critical bug
fixes.  Also, with 2.8.X, you can only generate i386 and ppc targets.  Unless you need
to do that (for OS X 10.4 users may be?), my advice is to use wx3.0.X on OS X.

Also, don't use wxWidgets 3.0.0.  It requires a patch that is not in the mainline 
repository.

Also note that some Mac OS X ships with some version of wxWidgets (2.8.8 on Snow 
Leopard), but only the debug binaries. We haven't tried building with Apple's
build of wxWidgets.  We always built wxWidgets ourselves, partly because we needed
the release builds also, and partly because we wanted to link statically so that 
we could distribute the binaries on as many platforms as possible.

OS X 10.9 doesn't ship with wxWidgets anymore, so you will need to build wxWidgets 
yourself on that platform, or newer.


3.3 Building wxWidgets for pwsafe
---------------------------------

These instructions should work on all platforms, but haven't been tested/applied well
enough on older platforms (OS X 10.6 with Xcode 3).  If you are on that platform and these
instructions don't work for you, try to read the section "6. Building on Old Platforms"

pwsafe uses wxWidgets as the cross-platform toolkit for its UI.  To build pwsafe, you 
need to build wxWidgets first, in a way that is compatible with pwsafe's project settings.
The Misc directory in pwsafe sources has a script called "osx-build-wx" which does exactly 
that.  It basically runs the wxWidgets "configure" script in a way that the wxWidgets build 
will happen with settings that are compatible with pwsafe's project settings, while retaining 
the ability to run on older versions of OS X as far back as possible.  It is possible that 
pwsafe built with such a build of wxWidgets will run on OS X 10.5, but it has not been verified.

You can pass it the "-n" option to show what parameters it's passing to configure.

osx-build-wx has to be run from your build directory.  Say, if you have wxWidgets sources 
in "wx3", then do

wx3 $ mkdir static-debug
wx3 $ cd static-debug
wx3/static/debug $ <path-to-pwsafe's osx-build-wx> -d
wx3/static/debug $ make

That would build the Debug configuration of wxWidgets in wx3/static-debug.  It would generate
static libraries of wxWidgets with universal binaries for i386 and x86_64 in static-debug/lib.
You can do the same thing again, but the directory name should be something like 
"static-release", and pass -r to osx-build-wx to build the Release configuration

Note that osx-build-wx doesn't actually run make: you need to run it yourself.

Also, you DON'T need to run "make install".  In fact, even wxWidgets recommends against that.
See this

http://wiki.wxwidgets.org/Compiling_wxWidgets_using_the_command-line_(Terminal)#Why_shouldn.27t_I_run_it.3F

These builds would take some time, so take a coffee break or something :-)

Note: I haven't used wx2.8 with pwsafe-xcode{5,6}.xcodeproj lately.  That can only generate i386 
targets and I don't see any value in that.


3.4 Point Xcode to your wxWidgets build
---------------------------------------

When Xcode tries to build pwsafe, it won't find the wxWidgets libraries you just built,
because Xcode has no idea where they are.  You need to generate "xcconfig" files to tell
Xcode to look into your "static-debug" and "static-release" directories for wxWidgets'
headers/libs.

Go to the Xcode directory in pwsafe sources, and do these

./generate-configs -d [full-path-to-your-static-debug/wx-config] > pwsafe-debug.xcconfig
./generate-configs -r [full-path-to-your-static-release/wx-config] > pwsafe-release.xcconfig



4. Building pwsafe
==================

If you have come this far, you only need to launch Xcode, load the pwsafe project
file, and hit 'Cmd-B'.  But pwsafe has 3 different project files.  Which one do
you use?


4.1 Xcode project files
-----------------------

There are 3 different versions of Xcode project files

* pwsafe.xcodeproj

This is the oldest one, and is probably not used much anymore by anybody.  It's main
use would be to create binaries for OS X as old as 10.4, running on ppc architecture.
It supports a bunch of other targets too, which is all described in detail below. But
if you are running a contemporary OS X (10.6+) and you just want to build pwsafe for
yourself, you probably don't care much about this project.

This project was created with Xcode 3 on OS X 10.6.  It was never tested with or on 
any other version of Xcode/OS X

* pwsafe-xcode5.xcodeproj

This one is to be used with Xcode5.  But Apple seems to auto-upgrade Xcode5 to Xcode6,
so it might not be useful for very long.  This one generates only intel binaries (no ppc),
but universal binaries with i386 and x86_64.  While building from the IDE, it will
only build your native architecture (which is probably x86-64), but you can build all
supported architectures from the command-line, as described later. You only need to do
that if you are building it for someone else, who's running an architecture different than
yours (i.e. non x86_64), which is unlikely.

If you want universal binaries, you need a similar build of wxWidgets, i.e. one that 
generated wxWidgets libraries as universal binaries.  There's a script to do that for you,
described later.

* pwsafe-xcode6.xcodeproj

This is very similar to the Xcode 5 version.  The only meaningful change (that I could
see) from the diffs is, it has an option to combine all hi-res artwork (images) into a 
single file, which probably doesn't apply to pwsafe.  Still, if you are using Xcode6,
use this project file.  If things change with Xcode6, this project will be updated
accordingly.  Others project files will not be updated anymore, unless there are bugs.


4.2 Debug and Release configs
-----------------------------

These instructions apply to Xcode5+ only.  How to switch between Release/Debug
configurations on older Xcode should be obvious.

Next, you need to decide whether to build the Debug or Release configuration of
pwsafe.  Apple has changed the way we (or at least I) used to view Debug and
Release configurations of a software.  Select "pwsafe" or "pwsafe-debug" from Product
Menu => Scheme to select Release or Debug configuration respectively.  And, if you are
building pwsafe for just yourself, see that the architecture in Product Menu => Destination
matches your Mac's architecture.

At this point, just hitting Cmd-B or click Product Menu => Build to build pwsafe.

4.3 Re-distribution
-------------------

From the IDE, Xcode will only build for your native architecture.  If you 
want to build for all suppported architectures, do it from the command line:

xcodebuild -project pwsafe-xcode5.xcodeproj -target pwsafe -configuration Debug ONLY_ACTIVE_ARCH=NO

Note that to generate universal binaries, your build of wxWidgets must also have 
produced universal binaries.  If you used the osx-build-wx script as above, it would be so. 


4.4 Where is pwsafe.app?
------------------------

In Xcode/build/Debug or Xcode/build/Release directories, dependening
on the configuration you built. 


4.5 Checking binary types
-------------------------

You can check for universal binaries by

lipo -info <path to a lib>

Example:

lipo -info lib/libwx_baseu-3.0.a
Architectures in the fat file: lib/libwx_baseu-3.0.a are: i386 x86_64

lipo -info build/Release/pwsafe.app/Contents/MacOS/pwsafe 
Architectures in the fat file: build/Release/pwsafe.app/Contents/MacOS/pwsafe are: x86_64 i386 

I hope it worked for you.  If not, please write to "passwordsafe-devel" mailing list or file
a ticket.


5. BUILDING ON OLD PLATFORMS (10.6-)
====================================


These are the build instructions for old systems running OS X 10.6 with Xcode 3, where osx-build-wx 
has not been tested.  If on that platform, the script might not work for you, and you might need to
build wxWidgets by hand.  In such a case, these instructions could be of some help.  You certainly
don't need to read these if you are on a latest OS X with Xcode5+ and only want to build pwsafe for
yourself.

If you are on OS X 10.6, you are probably running Xcode 3 or 4.  In which case, you'd need to use
pwasfe.xcodeproj.  This can be built in four flavours (four different Xcode targets):


                wxWidgets
 Target         required      xcconfig files               Binary types     Deployment target    Base SDK
 --------------------------------------------------------------------------------------------------------
 pwsafe           2.8     pwsafe-debug.xcconfig             i386 + ppc       10.4+                10.4
                          pwsafe-release.xcconfig           

 pwsafe-i386      2.8     pwsafe-debug.xcconfig        	    i386             10.4+                10.4
                          pwsafe-release.xcconfig      

 pwsafe64         3.x     pwsafe64-debug.xcconfig           X86_64           Compiler Default     Current Mac OS
                          pwsafe64-release.xcconfig         

 pwsafe-llvm      2.8     pwsafe-llvm-debug.xcconfig        i386             Compiler Default     Current Mac OS
                          pwsafe-llvm-release.xcconfig      
 --------------------------------------------------------------------------------------------------------


If you want to run PasswordSafe on older versions of Mac OS X (10.4+), including the ppc 
architecture, build the 'pwsafe' target. 

If you want to run it on the older versions of Mac OS X (10.4) but only on i386 hardware, select 
the 'pwsafe-i386' target.

If you need a 64-bit version of PasswordSafe, build the pwsafe64 target.  Note that you
need wxWidgets 2.9 release and also build it appropriately (see below).

The 'pwsafe-llvm' is an experimental target to build PasswordSafe with the llvm compiler.
This is only of interest if you are developing PasswordSafe on OS X and want to use
the static analysis abilities of clang (which are not available for C++ with the stock
llvm 1.7 shipped with OS X 10.6 or Xcode 3.2.x).  But the builds seem faster using llvm.


Building wxWidgets for pwsafe & pwsafe-i386 targets
===================================================

Since these targets are intended to run on 10.4 and above, we set these up to build
with 10.4 SDK.  Which in turn implies that wxWidgets 2.8 also needs to be built with 10.4 SDK.
That involves passing a ton of parameters to the "./configure" script for building wxWidgets.
There's a shell script (osx-build-wx) to aid with doing that, in the 'Misc' folder in the PasswordSafe
folder.  You only need to do the following (this assumes that the PasswordSafe folder and the wxWidgets folder
are in the same folder e.g., Documents/pwsafe and Documents/wxMac-2.8.12):

1. Download wxMac-2.8.12.tar.gz (or the latest 2.8 version)
2. tar xzf wxMac-2.8.12.tar.gz
3. cd wxMac-2.8.12
4. mkdir static-release ; cd static-release ; ../../<name of your pwsafe folder>/Misc/osx-build-wx
5. mkdir static-debug ; cd static-debug ; ../../<name of your pwsafe folder>/Misc/osx-build-wx DEBUG

This will build the static (.a's) version of wxWidgets.  The debug build
would end up in static-debug and release in static-release. It is necessary
to build them in separate directories otherwise the "wx-config" script from one will
overwrite the other.  Of course, you don't need both Debug and Release builds
of wxWidgets unless you need both Debug and Release builds of PasswordSafe.

It is possible for PasswordSafe & wxWidgets to be built with 10.6 SDK and still run on 10.4+ if the
deployment target is set appropriately in Xcode, but I have no way of trying that.
I'm also not aware of what precautions to take in the code to not add any dependencies
that cannot be satisfied on 10.4.


Building wxWidgets for pwsafe64 target
======================================

Essentially, you need a 64-bit build of wxWidgets, which is only possible with 2.9 series of wxWidgets or later.

1. Download wxWidgets-2.9.2.tar.gz (or the latest 2.9 release)
2. tar xzf wxWidgets-2.9.2.tar.gz
3. cd wxWidgets-2.9.2
4. mkdir static64-debug ; cd static64-debug
5. ../configure --prefix=`pwd` --disable-shared --enable-unicode --enable-debug --with-osx_cocoa
6. make

That last bit about "--with-osx_cocoa" is what ensures you get a 64-bit build of wxWidgets.  For
Release configuration, just change "--enable-debug" with "--disable-debug".  Of course, if you're
only going to build one of Debug or Release configurations of PasswordSafe, you only need to build 
the corresponding configuration of wxWidgets.


Building wxWidgets for pwsafe-llvm target
=========================================

I use an llvm-built version of wxWidgets when I build pwsafe-llvm, but its
probably not necessary.  The gcc and llvm libraries/binaries are compatible
with each other.  Still, if you want to build wxWidgets with llvm, do this:

1. Download wxMac-2.8.12.tar.gz (or the latest 2.8 release)
2. tar xzf wxWidgets-2.8.12.tar.gz
3. cd wxWidgets-2.8.12
4. mkdir static-llvm-debug ; cd static-llvm-debug ;
5. ../configure --prefix=`pwd` CC='llvm-gcc-4.2` CXX='llvm-g++-4.2' CFLAGS='-arch i386' CXXFLAGS='-arch i386' CPPFLAGS='-arch i386' LDFLAGS='-arch i386' OBJCFLAGS='-arch i386' OBJCXXFLAGS='-arch i386' --enable-debug --disable-shared --disable-copmat24 --enable-unicode
6. make

For the release build, do the same, except replace "--enable-debug" with "--disable-debug".
 


Generate xcconfig files
=======================

Having built wxWidgets, unless you are willing to "make install" wxWidgets and overwrite 
whatever shipped with your os, you will have to tell Xcode where to pick up your wxWidgets
headers/libs from. wxWidgets makes it easy by creating a script called 'wx-config' during its 
build process (command-line makefile based builds only) that spits out the correct location 
of headers/libs as well as compiler and liker settings compatible with that build of wxWidgets.
This is used in UNIX makefiles to compile/link with the desired build of wxWidgets where its 
trivial to read in the settings from outputs of external commands.

Since Xcode can't pick up settings from output of external commands, we use a script to 
put those settings into configuration files that Xcode can use. Xcode target configurations 
can be "based on" xcconfig files, which are essentially sets of name-value pairs.  For a 
target/configuration, Xcode will use settings from the xcconfig file, if found. Else, it 
will use the values specified in its GUI.

The "Xcode/generate-configs" script generates xcconfig data from the wx-config files.
Go to the Xcode subdirectory of your pwsafe source checkout, and do this:

./generate-configs -d full_path_to_wx-config  > xcconfig_file

For Release builds, the first parameter should be "-r" instead of "-d".

Substitute "xcconfig_file" with the correct xcconfig file name from the above table.  Each
target and each configuration has its own xcconfig file.  Make sure you have them
in the "Xcode" sub-directory of your PasswordSafe source tree.


Building PasswordSafe
=====================

You can either open pwsafe.xcodeproj in Xcode and build your preferred target.  Or
you can do so from the command line.  In the Xcode directory, do this:

xcodebuild -target <your target> -configuration <Debug or Release>

Keep your fingers crossed :-).  Once built, the app would be there in one of "build",
"build64" or "build-llvm" folders.  You should be able to use your 
PasswordSafe databases from Windows/Linux without any problems.

If things don't work, or you wish to improve them nonetheless, please get
in touch with developers.

Note that some of the XML related functionality is still not in place for the
OS X build.  We will get those in sometime.

