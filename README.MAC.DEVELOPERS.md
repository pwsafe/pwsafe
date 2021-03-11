## Introduction
This document explains how to build PasswordSafe on Mac OS X.
It is organized in the following sections.

* [Requirements](#requirements)
* [Get PasswordSafe Sources](#get-passwordsafe-sources)
* [wxWidgets](#wxwidgets)
  * Downloading the Sources
  * Which Version of wxWidgets?
  * Building wxWidgets for pwsafe
  * Point Xcode to your wxWidgets Build
* [Build pwsafe](#build-pwsafe)
  * Debug and Release Configs
  * Where is pwsafe.app?
* [Build installation package](#build-dmg)


## Requirements

* Xcode
* wxWidgets
* Perl
* gettext (can be fetched from brew)


### Xcode
Xcode is free and is included with the OS X installation CD. If you don't have the CD,
you can probably download Xcode from Apple's website. You will also need the
"Command-Line Tools for Xcode".

The Xcode directory in PasswordSafe sources contains the Xcode project file for building it.

pwsafe code now uses C++14 features, and therefore requires a modern-enough compiler (Xcode6).
Also, the minimum deployment target is now OS X 10.7.


### wxWidgets
wxWidgets is the cross-platform UI toolkit pwsafe uses for user-interface.  You'd need to
download the latest sources from wxWidgets.org and build it (instructions below).  This is
the most time-consuming part of building pwsafe.


### Perl
pwsafe uses perl for some small build tasks. OS X already ships with Perl, which should 
suffice.


## Get PasswordSafe Sources
You need to get the Passwordsafe source code (obviously).
Either download from the website:

https://github.com/pwsafe/pwsafe/archive/master.zip

or clone the git repository:

    git clone https://github.com/pwsafe/pwsafe.git


## wxWidgets
wxWidgets is the UI toolkit used by pwsafe for user-interface. On OS X, you will have
to download and build this before you can build pwsafe.


### Downloading the Sources
I recommend you download the tarball from wxWidgets download site

https://www.wxwidgets.org/downloads/

Click the link named "Source for Linux, OS X, etc".  It should get you a .tar.bz2
or a .tar.gz file. DO NOT get the .7z version. That has sources with DOS CRLF 
and won't build on OS X. Also, I have found some (may be irrelevant) inconsistencies
between the sources checked out from their repository at a particular branch/tag, and
the tarball. My recommendation is to use the tarball. That's what I always do on OS X.


### Which Version of wxWidgets?
Use wxWidgets 3.0.2 or newer. pwsafe code is no longer compatible with older 
versions of wxWidgets.

If you use homebrew, you can just install the latest version supported by it (3.0.5 as of now).

### M1 Macs
You'd need to use the latest version, i.e. 3.1.4 as of this writing. The current
stable version (3.0.5) has several issues. You may see the app crash when you try to search (Cmd-F)

https://trac.wxwidgets.org/ticket/19005

### International users
When changing the language from English to another language you might encounter problems with onStateImgage (mark indicating the selected menu item) or chevon ">>" extending the tool bar in case space is not sufficient. There is a proposed patch that can help on that issue, but this is not accepted by the wxWidgets team so far. See https://trac.wxwidgets.org/ticket/19023

diff --git a/src/common/intl.cpp b/src/common/intl.cpp
index 44c4df22a8..97396917f3 100644
a	b	bool wxLocale::Init(int language, int flags) 
575	575	#elif defined(__WXMAC__)
576	576	    const wxString& locale = info->CanonicalName;
577	577	
578	 	    const char *retloc = wxSetlocale(LC_ALL, locale);
 	578	    const char *retloc = wxSetlocale(LC_ALL, (! m_pszOldLocale || strcmp(m_pszOldLocale, "C/UTF-8/C/C/C/C")) ? locale : "C/"+locale+".UTF-8/C/C/C/C");
579	579	
580	580	    if ( !retloc )
581	581	    {


### Building wxWidgets for pwsafe
pwsafe uses wxWidgets as the cross-platform toolkit for its UI. To build pwsafe, you 
need to build wxWidgets first, in a way that is compatible with pwsafe's project settings.
The Misc directory in pwsafe sources has a script called "osx-build-wx" which does exactly 
that.  It basically runs the wxWidgets "configure" script in a way that the wxWidgets build 
will happen with settings that are compatible with pwsafe's project settings, while retaining 
the ability to run on older versions of OS X as far back as possible.  It is possible that 
pwsafe built with such a build of wxWidgets will run on OS X 10.7, but it has not been verified.

You can pass it the "-n" option to show what parameters it's passing to configure.

osx-build-wx has to be run from your build directory.  Say, if you have wxWidgets sources 
in "wx3", then do
```
wx3 $ mkdir static-debug
wx3 $ cd static-debug
wx3/static-debug $ <path-to-pwsafe's osx-build-wx> -d
wx3/static-debug $ make
```

That would build the Debug configuration of wxWidgets in wx3/static-debug.  It would generate
static libraries of wxWidgets with universal binaries for i386 and x86_64 in static-debug/lib.
You can do the same thing again, but the directory name should be something like 
"static-release", and omit the -d to osx-build-wx to build the Release configuration

Note that osx-build-wx doesn't actually run make: you need to run it yourself.

Also, you DON'T need to run "make install".  In fact, even wxWidgets recommends against that.
See this

http://wiki.wxwidgets.org/Compiling_wxWidgets_using_the_command-line_(Terminal)#Why_shouldn.27t_I_run_it.3F

These builds would take some time, so take a coffee break or something :-)

If wxWidgets builds fine but you get weird compilation errors while building pwsafe, try
re-building wxWidgets with the exact same SDK that you are building pwsafe with.  You can
get a list of all installed SDKs by running

    xcodebuild -showsdks

Then get the path to your exact sdk by running

    xcodebuild -version -sdk macosx10.9 Path

Pass that to the build script with the -k option

    wx3/static/debug $ <path-to-pwsafe's osx-build-wx> -d -k <path from above>

Or, just pass the command's output directly

    wx3/static/debug $ <path-to-pwsafe's osx-build-wx> -d -k `xcodebuild -version -sdk macosx10.9 Path`

### M1 Macs
You'd need to build it yourself, like this

cd wxWidgets  # chdir into wxWidgets sources
mkdir arm     # make a folder for arm64 + x86_64 builds
cmake -B arm -DCMAKE_OSX_ARCHITECTURES='arm64;x86_64'

This will build wxWidgets for both architectures.


### Point Xcode to your wxWidgets Build
When Xcode tries to build pwsafe, it won't find the wxWidgets libraries you just built,
because Xcode has no idea where they are. You need to generate "xcconfig" files to tell
Xcode to look into your "static-debug" and "static-release" directories for wxWidgets'
headers/libs.

Go to the Xcode directory in pwsafe sources, and do these
```
./generate-configs -d [full-path-to-your-static-debug/wx-config] > pwsafe-debug.xcconfig
./generate-configs -r [full-path-to-your-static-release/wx-config] > pwsafe-release.xcconfig
```


## Build pwsafe
If you have come this far, you only need to launch Xcode, load the pwsafe project
file, and hit 'Cmd-B'. But pwsafe has 3 different project files. Which one do
you use?


### Debug and Release Configs
You need to decide whether to build the Debug or Release configuration of
pwsafe. Apple has changed the way we (or at least I) used to view Debug and
Release configurations of a software. Select "pwsafe" or "pwsafe-debug" from Product
Menu => Scheme to select Release or Debug configuration respectively. And, if you are
building pwsafe for just yourself, see that the architecture in Product Menu => Destination
matches your Mac's architecture.

At this point, just hitting Cmd-B or click Product Menu => Build to build pwsafe.


### Where is pwsafe.app?
In Xcode/build/Debug or Xcode/build/Release directories, depending
on the configuration you built. It might be located by default in the users folder
~/Library/Developer/Xcode/DerivedData/pwsafe-xcode6-.../Build/Products/Debug or
~/Library/Developer/Xcode/DerivedData/pwsafe-xcode6-.../Build/Products/Release,
where ... is a complex unique number for the build.


## Build installation package
You have to create or update help and translations files. Call "make" in folder
"help" and "src/ui/wxWidgets/I18N". For missing tools install "gettext" from "brew" for instance.
In I18N you might update related po file in sub-folder pos and call "make mos".

After building of pwsafe.app (in Xcode), language files and help you might call "make" in "install/macosx", after setting path to wxWidgets "locale" folder, to create the package in main folder.
