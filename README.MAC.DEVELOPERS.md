## Introduction
This document explains how to build PasswordSafe on macOS.
It is organized in the following sections.

* [Terminology](#terminology)
* [Requirements](#requirements)
* [Get PasswordSafe Sources](#get-passwordsafe-sources)
* [wxWidgets](#wxwidgets)
  * Downloading the Sources
  * Which Version of wxWidgets?
  * Building wxWidgets for pwsafe
  * Point Xcode to your wxWidgets Build
* [Set pwsafe version](#set-the-pwsafe-version)
* [Build pwsafe](#build-pwsafe)
  * Debug and Release Configs
  * Where is pwsafe.app?
* [Build installation package](#build-dmg)


## Terminology

* x86\_64 - Intel based Mac. Apple tools use this designation.
* arm64 - arm based Mac (Apple Silicon or M1). Apple tools use this designation.
* aarch64 or arch64 - alternative reference to an arm64 based Mac.
* universal binary - a binary executable that contains multiple builds. A universal binary for pwsafe would contain x86\_64 and arm64 builds. It could run natively on Intel or Apple Silicon based Macs.

## Requirements
In general you need the following.

* Xcode 6+
* wxWidgets
* Perl
* gettext (can be fetched from brew)

If you are building on Apple Silicon or M1, you need the following.

* Apple Silicon or M1 equipped Mac
* macOS 11.0 (Big Sur) or later
* Xcode 12+

### Xcode
Xcode is free and originally was included with the OS X installation CD. Today, you can download Xcode from the Apple Store. You will also need the
"Command-Line Tools for Xcode". The command line tools can be installed from a
terminal.

```
xcode-select --install
```

The Xcode directory in PasswordSafe sources contains the Xcode project file for building it.

pwsafe code now uses C++14 features, and therefore requires a modern-enough compiler (Xcode 6 or later).
For the x86\_64 architecture, the minimum target is macOS 10.7. For the new arm64 architecture, the minimum target is macOS 11.0. 


### wxWidgets
wxWidgets is the cross-platform UI toolkit pwsafe uses for user-interface. To get the latest version of wxWidgets, you need to
download the latest sources from wxWidgets.org and build it (instructions below).  This is
the most time-consuming part of building pwsafe.


### Perl
pwsafe uses perl for some small build tasks. macOS already ships with Perl, which should suffice.


## Get PasswordSafe Sources
You need to get the Passwordsafe source code (obviously).
Either download from the website:

[https://github.com/pwsafe/pwsafe/archive/master.zip](https://github.com/pwsafe/pwsafe/archive/master.zip)

or clone the git repository:

    git clone https://github.com/pwsafe/pwsafe.git


## wxWidgets
wxWidgets is the UI toolkit used by pwsafe for user-interface. On macOS, you will have
to download and build this before you can build pwsafe.


### Downloading the wxWidgets Sources
I recommend you download the tarball from wxWidgets download site

[https://www.wxwidgets.org/downloads/](https://www.wxwidgets.org/downloads/)

Click the link named "Source for Linux, OS X, etc".  It should get you a .tar.bz2
or a .tar.gz file. DO NOT get the .7z version. That has sources with DOS CRLF 
and won't build on OS X. Also, I have found some (may be irrelevant) inconsistencies
between the sources checked out from their repository at a particular branch/tag, and
the tarball. My recommendation is to use the tarball. That's what I always do on macOS.


### Which Version of wxWidgets?
Use wxWidgets 3.0.2 or newer. pwsafe code is no longer compatible with older 
versions of wxWidgets.

If you use HomeBrew, you can just install the latest version supported by it (3.0.5 as of now).
There are a number of issues with version 3.0.5. For example, see
[https://trac.wxwidgets.org/ticket/19005](https://trac.wxwidgets.org/ticket/19005).

Therefore, it is better to use wxWidgets 3.1.5 or newer.

### International users
When changing the language from English to another language you might encounter problems with onStateImgage (mark indicating the selected menu item) or chevon ">>" extending the tool bar in case space is not sufficient. There is a proposed patch that can help on that issue, but this is not accepted by the wxWidgets team so far. See [https://trac.wxwidgets.org/ticket/19023](https://trac.wxwidgets.org/ticket/19023).

```
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
```

The issue seems to be fixed in wxWidgets 3.1.5 (eliminating the special code for __WXMAC__). Last version of June 2021 is reproducing the same wrong behaviour with macOS. Following fix, that is not yet approved, will solve the issue.

```
diff --git a/src/common/wxcrt.cpp b/src/common/wxcrt.cpp
index f3585a905a..8437ce6ff6 100644
--- a/src/common/wxcrt.cpp
+++ b/src/common/wxcrt.cpp
@@ -138,28 +138,11 @@ char* wxSetlocale(int category, const char *locale)
         wxCFStringRef str(wxCFRetain((CFStringRef)CFLocaleGetValue(userLocaleRf, kCFLocaleLanguageCode)));
         wxString langFull = str.AsString()+"_";
         str.reset(wxCFRetain((CFStringRef)CFLocaleGetValue(userLocaleRef, kCFLocaleCountryCode)));
-        langFull += str.AsString()+".UTF-8";
-        if(category == LC_ALL)
-        {
-          langFull = "C/"+langFull+"/C/C/C/C";
-        }
+        langFull += str.AsString();
         rv = setlocale(category, langFull.c_str());
     }
-    else {
-        if(locale) {
-          wxString lc(locale);
-          if(strlen(locale) == 5) { // only xx_YY, we have to add .UTF-8
-            lc += ".UTF-8";
-          }
-          if(category == LC_ALL)
-          {
-            lc = "C/"+lc+"/C/C/C/C";
-          }
-          rv = setlocale(category, lc.c_str());
-        }
-        else
-          rv = setlocale(category, locale);
-    }
+    else
+        rv = setlocale(category, locale);
 #else
     char *rv = setlocale(category, locale);
 #endif
```

### Building wxWidgets for x86\_64 pwsafe
To build pwsafe, you 
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

### Building wxWidgets for arm64 pwsafe
If you are building for Apple Silicon (M1 or arm64), you have two options.

1. Use HomeBrew to install wxWidgets
2. Build wxWidgets from source targeting arm64

Either of these options requires an M1 equipped Mac, Big Sur 11.0 or later and Xcode 12.

> Note: It would be ideal if we could build a universal version of wxWidgets and pwsafe.
Unfortunately, that requires universal versions of all pwsafe dependencies (wxWidgets included) and universal versions of all dependencies are not currently available (hopefully, at some point in the future this problem will be resolved). Therefore, we have to fall back to building separate x86_64 and arm64 versions of pwsafe. Technically, we 
could build a universal version of wxWidgets, but other pwsafe dependencies are not 
available as universal binaries. 

#### Use HomeBrew to Install wxWidgets

As an alternative to building wxWidgets for the arm64 architecture, you can install wxWidgets using HomeBrew. 

```
brew install wxmac
```

As of this time, HomeBrew installs an arm64 version of wxWidgets at revision 3.0.5.1.
Unfortunately, this version has the "find" problem. The app will crash from 
Edit | Find Entry or Cmd-F. That problem is fixed in 3.1.5, but HomeBrew has not caught up.

After installing wxmac, generate Xcode configuration files using the following commands.

```
cd pwsafe/Xcode
./generate-configs -d /opt/homebrew/lib/wx/config/osx_cocoa-unicode-3.0 > pwsafe-debug.xcconfig
./generate-configs -r /opt/homebrew/lib/wx/config/osx_cocoa-unicode-3.0 > pwsafe-release.xcconfig
```

#### Build arm64 wxWidgets from Source

* [Download](https://github.com/wxWidgets/wxWidgets/releases/download/v3.1.5/wxWidgets-3.1.5.tar.bz2) the latest source (3.1.5 as of this writing)
* Unpack the source
* cd into the source directory
* Create a build directory (e.g. build-cocoa)
* cd into the build directory
* Run the wxWidgets configure script (see below)
* Run make
* cd into the pwsafe Xcode directory
* Generate Xcode configuration files (see below)


```
cd wxWidgets-3.1.5
mkdir build-cocoa
cd build-cocoa
../configure --with-macosx-version-min=11.0 --enable-universal_binary=arm64
make
```

```
cd pwsafe/Xcode
./generate-configs -d [full-path-to-build-cocoa-directory/wx-config] > pwsafe-debug.xcconfig
./generate-configs -r [full-path-to-build-cocoa-directory/wx-config] > pwsafe-release.xcconfig
```

> Note: We could configure wxWidgets to build a universal (x86\_64 and arm64) binary.
However, as previously noted, pwsafe has other dependencies that are not readily
available as universal binaries. Therefore, we don't bother building wxWidgets for x86_64 on 
an M1 equipped machine.

## Set the Pwsafe Version
The current version of macOS pwsafe is defined in the pwsafe/version.wx file. If you
need to update the version, do so before building pwsafe. The pwsafe build will pick up
the version as part of the build.

## Build pwsafe
If you have come this far, you only need to launch Xcode, load the pwsafe project
file (pwsafe/Xcode/pwsafe-xcode6.xcodeproj), and hit 'Cmd-B'.

### Debug and Release Configs
You need to decide whether to build the Debug or Release configuration of
pwsafe. Apple has changed the way we (or at least I) used to view Debug and
Release configurations of a software. Select "pwsafe" or "pwsafe-debug" from Product
Menu => Scheme to select Release or Debug configuration respectively. And, if you are
building pwsafe for just yourself, see that the architecture in Product Menu => Destination
matches your Mac's architecture.

At this point, just hitting Cmd-B or click Product Menu => Build to build pwsafe.

> As of Xcode 12, only pwsafe appears in the list of build schemes. 
If you choose Product | Build, it defaults to a release build. 
This is equivalent to Product | Build For | Running.
If you choose Product | Build For | Testing you will get a debug build.

### Where is pwsafe.app?
Look in these locations.

```
pwsafe/Xcode/build/Products/Debug/pwsafe.app
pwsafe/Xcode/build/Products/Release/pwsafe.app
```

> The default location used by Xcode for storing apps is somewhat opaque, 
so the pwsafe-xcode6 project sets the output location to something more useable.

## Build installation package (.dmg file)
You have to create or update help and translations files. Call "make" in folder
"help" and "src/ui/wxWidgets/I18N". For missing tools install "gettext" from "brew" for instance.
In I18N you might update related po file in sub-folder pos and call "make mos".

After building the pwsafe.app (in Xcode), language files and help you can create
a .dmg file for installing pwsafe into the Applications folder.

To begin this process you need to make some edits to the pwsafe/install/Makefile.

* Set the RELDIR variable to the location of pwsafe.app
* Set the WXDIR variable to the location of the "locale" folder

```
RELDIR=./../../Xcode/build/Products/Release/
WXDIR=../../../wxWidgets-3.1.5/locale
```

To create the .dmg file

```
cd pwsafe/install/macosx
make
```

The file will appear in the pwsafe directory. It's name will be something like:

```
pwsafe-arm64-1.13.0.dmg
or
pwsafe-x86_64-1.13.0.dmg
```

The file name includes arm64 or x86\_64 to indicate the target machine architecture.
