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
* [Build installation package](#build-installation-package)
* [Universal Binaries](universal-binaries)


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

If you are building on Apple Silicon or M1, you need the following in addition to the above.

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

The Xcode directory in PasswordSafe so[]()urces contains the Xcode project file for building it.

pwsafe code now uses C++14 features, and therefore requires a modern-enough compiler (Xcode 6 or later).
For the x86\_64 architecture, the minimum target is macOS 10.7. For the new arm64 architecture, the minimum target is macOS 11.0. 


### wxWidgets
wxWidgets is the cross-platform UI toolkit pwsafe uses for user-interface. To get the latest version of wxWidgets, you may need to
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
wxWidgets is the UI toolkit used by pwsafe for user-interface. There are two ways to acquire wxWidgets.
You can use HomeBrew to install the wxwidgets package.

```
brew install wxwidgets
```

The problem with using HomeBrew to install wxWidgets is that the version installed by HomeBrew might
not be the most up-to-date version. You can find the version that HomeBrew will install by running the command:

```
brew info wxwidgets
```

Note that the version of wxWidgets available through HomeBrew may be different for x86\_64
systems and Apple Silicon systems.

The second way to get wxWidgets is to download and build wxWidgets from source.

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

If you use HomeBrew, you can just install the latest version supported by it. On Apple Silicon 
with macOS 11.x it's 3.1.5 as of now, and on x86\_64 it's 3.0.5.1.
There are a number of issues with version 3.0.5. For example, see
[https://trac.wxwidgets.org/ticket/19005](https://trac.wxwidgets.org/ticket/19005).

**Therefore, it is best to use wxWidgets 3.1.5 or newer.**

### International users
When changing the language from English to another language you might encounter problems with onStateImgage (mark indicating the selected menu item) or chevon ">>" extending the tool bar in case space is not sufficient. This is a problem in Apples SVG library, see [https://trac.wxwidgets.org/ticket/19023](https://trac.wxwidgets.org/ticket/19023). setlocale(LC_NUMERIC, ...) must be left as "C" or one of the languages using a dot as decimal point. 

### Building wxWidgets for x86\_64 or arm64 pwsafe
**This procdure works for both x86\_64 and arm64 (Apple Silicon).** It builds wxWidgets for
the architecture reported by the following command:

```
uname -m
```

On Intel 64-bit versions of macOS this command will report "x86\_64". On Apple Silicon versions
of macOS (11.0 and later) this command will report "arm64".

**If you are trying to build pwsafe for an older I386 or x86\_32 machine, this procedure may work, but 
it has not been tested.**

To build pwsafe, you 
need to build wxWidgets first, in a way that is compatible with pwsafe's project settings.
The Misc directory in pwsafe sources has a script called "osx-build-wx" which does exactly 
that.  It basically runs the wxWidgets "configure" script in a way that the wxWidgets build 
will happen with settings that are compatible with pwsafe's project settings, while retaining 
the ability to run on older versions of OS X as far back as possible.  It is possible that 
pwsafe built with such a build of wxWidgets will run on OS X 10.7, but it has not been verified.

You can pass it the "-n" option to show what parameters it's passing to configure.

osx-build-wx has to be run from your wxWidgets build directory.  Say, if you have wxWidgets sources 
in "wx3", then do

```
wx3 $ mkdir static-debug
wx3 $ cd static-debug
wx3/static-debug $ <path-to-pwsafe's osx-build-wx> -d
wx3/static-debug $ make
```

That would build the Debug configuration of wxWidgets in wx3/static-debug.  It would generate
static libraries of wxWidgets. Omit the -d to osx-build-wx to build the Release configuration

Note that osx-build-wx doesn't actually run make: you need to run it yourself.

**Also, you DON'T need to run "make install".**  In fact, even wxWidgets recommends against that.
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

### Point Xcode to your wxWidgets Build

For users using Xcode to build the application you must generate a linking to wxWidgets libraries and include files. You need to build **pwsafe-release.xcconfig** and **pwsafe-debug.xcconfig** files in the **Xcode** directory, derived from **wx-config** file located in **static-release** and **static-debug** folder of the native generated wxWidgets environment (see above).

```
pwsafe $ cd Xcode
Xcode $ ./generate-configs -r <path-to-wxWidgets wx3/static-release/wx-config> > pwsafe-release.xcconfig
Xcode $ ./generate-configs -d <path-to-wxWidgets wx3/static-debug/wx-config> > pwsafe-debug.xcconfig
```


## Set the Pwsafe Version
The current version of macOS pwsafe is defined in the pwsafe/version.wx file. If you
need to update the version, do so before building pwsafe. The pwsafe build will pick up
the version as part of the build. If the build does not pick up the version change, try deleting the
pwsafe/src/ui/wxWidgets/version.h file before building.

## Build pwsafe
If you have come this far, you only need to launch Xcode, load the pwsafe project
file (pwsafe/Xcode/pwsafe-xcode6.xcodeproj), and hit 'Cmd-B' (or Product | Build).
This will build the currently selected scheme. Note that the build will be for the architecture
of the machine you are running on (either Intel or Apple Silicon). You cannot build for Apple Silicon
on an Intel machine or vice-versa.

### Debug and Release Configs
You need to decide whether to build the Debug or Release configuration of
pwsafe. Apple has changed the way we (or at least I) used to view Debug and
Release configurations of a software. Select "pwsafe" or "pwsafe-debug" from Product
Menu => Scheme to select the Release or Debug configuration respectively. And, if you are
building pwsafe for just yourself, see that the architecture in Product Menu => Destination
matches your Mac's architecture.

At this point, just hitting Cmd-B or click Product Menu => Build will build the chosen 
pwsafe configuration.

### Where is pwsafe.app?
Look in these locations.

```
pwsafe/Xcode/build/Products/Debug/pwsafe.app
pwsafe/Xcode/build/Products/Release/pwsafe.app
```

> The default location used by Xcode for storing apps is somewhat opaque, 
so the pwsafe-xcode6 project sets the output location to something more useable.

## Build installation package
This procedure will build a .dmg file for the current machine's architecture.

Build pwsafe for release. The installation package tools only create a release .dmg file.

You have to create or update help and translations files. Call "make" in folder
"help" and "src/ui/wxWidgets/I18N".

```
cd pwsafe/help
make
cd pwsafe/src/ui/wxWidgets/I18N
make
```

For missing tools install "gettext" from "brew" for instance.
In I18N you might update related po file in sub-folder pos and call "make mos".

```
cd pwsafe/src/ui/wxWidgets/I18N
make mos
```

After building the pwsafe.app (in Xcode), language files and help you can create
a .dmg file for installing pwsafe into the Applications folder.

To begin this process you need to make some edits to the pwsafe/install/macosx/Makefile.

* Set the RELDIR variable to the location of pwsafe.app
* Set the WXDIR variable to the location of the "locale" folder

For example:

```
RELDIR=./../../Xcode/build/Products/Release/
WXDIR=../../../wxWidgets-3.1.5/locale
```
Alternative use the shell scripts found in the Makefile to locate the correct location.

To create the .dmg file

```
cd pwsafe/install/macosx
make
```

The file will appear in the pwsafe directory. It's name will be something like:

```
pwsafe-arm64-1.15.0.dmg
or
pwsafe-x86_64-1.15.0.dmg
```

The file name includes arm64 or x86\_64 to indicate the target machine architecture.

### Known Issues with macOS Install

On a clean, initial install, the first time pwsafe is run the language may default to German. This
appears to be an issue in wxWidgets on Apple M1 machines. If you encounter this problem, open a 
pwsafe file. The third menu item from the right is the "manage" submenu. Click on it and select
the last/bottom menu item. This will give you a list of languages. Pick the desired language (e.g. English).

## Universal Binaries

In a perfect world we would build pwsafe and all its dependencies as universal binaries containing 
both x86\_64 and arm64 versions. However, this is not currently possible. To build universal binaries
you need a universal version of every dependency. Currently, most installed dependencies only
cover the architecture of the machine they are on. As a result, you can only build an
x86\_64 version on an x86\_64 based machine and you can only build an arm64 on an arm64 based
machine (e.g. an Apple M1 based machine).
