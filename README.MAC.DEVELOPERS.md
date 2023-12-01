## Introduction
This document explains how to build PasswordSafe on macOS.
It is organized in the following sections:

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
* [Known Issues](#known-issues-with-macos-install)


## Terminology

* x86\_64 - Intel based Mac. Apple tools use this designation.
* arm64 - arm based Mac (Apple Silicon, M1, etc.). Apple tools use this designation.
* aarch64 or arch64 - alternative reference to an arm64 based Mac.
* universal binary - a binary executable that contains multiple builds. A universal binary for pwsafe would contain x86\_64 and arm64 builds. It could run natively on Intel or Apple Silicon based Macs.

## Requirements
In general you need the following:

* Xcode 6+
* wxWidgets
* Perl
* gettext and create-dmg (For building the installation package, can be fetched from Homebrew)
* Yubikey libraries: libyubikey, libykpers-1

If you are building on Apple Silicon or M1, you need the following in addition to the above:

* Apple Silicon equipped Mac
* macOS 11.0 (Big Sur) or later
* Xcode 12+

### Xcode
Xcode is free and originally was included with the OS X installation CD. Today, you can download Xcode from the Apple Store. If you don't want the full Xcode system, you will need to install the "Command-Line Tools for Xcode". The command line tools can be installed from a terminal.

```
xcode-select --install
```

The full Xcode app includes the command-line tools. However, after you install it from the App Store, you may need to execute the following to set-up the paths:

```
sudo xcode-select --switch /Applications/Xcode.app
```

The Xcode directory in PasswordSafe sources contains the Xcode project file for building it.

pwsafe code now uses C++14 features, and therefore requires a modern-enough compiler (Xcode 6 or later).
For the x86\_64 architecture, the minimum target is macOS 10.14. For the arm64 architecture, the minimum target is macOS 11.0. 

### wxWidgets
wxWidgets is the cross-platform UI toolkit that pwsafe uses for user-interface. To get the latest version of wxWidgets, you may need to download the latest sources from wxWidgets.org and build it (instructions below).  This is the most time-consuming part of building pwsafe.

### Perl
pwsafe uses Perl for some small build tasks. macOS already ships with Perl, which should suffice.

### Yubikey
The last official packages published by Yubico, on their web site and via Homebrew, are known not to work on current macOS systems.  (As of this writing, macOS 14.1.)  The source must be retrieved from Github and compiled locally.  The process used by the PasswordSafe release build is in the pwsafe source tree in the file pwsafe/.github/workflows/macos-latest.yml.  If you don't want Yubikey support, the Xcode project file for PasswordSafe has a *Debug-no-yubi* configuration that is the same as Debug, but without reference to the Yubikey libraries.  Note, however, that the .dmg release package can only be made from a *Release* configuration build that does include Yubikey support.

## Universal Binaries

The current procedure will build a Universal Binary but this requires macOS 11+, Xcode 12+, and universal
versions of all the dependencies (i.e. wxWidgets as compiled per the procedure below.)  The result should be compatible
with macOS 10.14 and, possibly, older but this has not been throughly tested.


## Get PasswordSafe Sources
You need to get the Passwordsafe source code (obviously).
Either download from the website:

[https://github.com/pwsafe/pwsafe/archive/master.zip](https://github.com/pwsafe/pwsafe/archive/master.zip)

or clone the git repository:

    git clone https://github.com/pwsafe/pwsafe.git


## wxWidgets
wxWidgets is the UI toolkit used by pwsafe for user-interface. There are two ways to acquire wxWidgets.

### Installing wxWidgets via HomeBrew

```
brew install wxwidgets
```

The problem with using HomeBrew to install wxWidgets is that the version installed by HomeBrew might
not be the most up-to-date version. You can find the version that HomeBrew will install by running the command:

```
brew info wxwidgets
```

**Important Notes**
- The version of wxWidgets available through HomeBrew may be different for x86\_64
systems and Apple Silicon systems and it will only install the version for your Mac architecture, so you won't be able to produce a universal binary. 
- Also, because of the addition of the "Hardened Runtime", before you build pwsafe:
 
```
Open Xcode, select the pwsafe target, "Signing & Capabilities" tab, "All" subtab.
Check the box for "Diable Library Validation".
Repeat these steps for the pwsafe-cli target.
```

(This is not necessary if you are building wxWidgets from source.)

- For more information, see: 
[https://developer.apple.com/documentation/security/hardened_runtime](https://developer.apple.com/documentation/security/hardened_runtime)

### Downloading the wxWidgets Sources
The second way to get wxWidgets is to download and build wxWidgets from source.
I recommend you download the tarball from wxWidgets download site

[https://www.wxwidgets.org/downloads/](https://www.wxwidgets.org/downloads/)

Click the link named "Source for Linux, OS X, etc".  It should get you a .tar.bz2
or a .tar.gz file. DO NOT get the .7z version. That has sources with DOS CRLF 
and won't build on OS X. Also, I have found some (maybe irrelevant) inconsistencies
between the sources checked out from their repository at a particular branch/tag, and
the tarball. My recommendation is to use the tarball. That's what I always do on macOS.


### Which Version of wxWidgets?
Use wxWidgets 3.2.2.1 or newer. pwsafe code is no longer compatible with versions of wxWidgets older than 3.2.1.

There are a number of issues with version 3.0.5. For example, see
[https://trac.wxwidgets.org/ticket/19005](https://trac.wxwidgets.org/ticket/19005).

There is also a Mac specific bug in 3.2.1.

**Therefore, it is best to use wxWidgets 3.2.2.1 or newer.**

### International users
When changing the language from English to another language you might encounter problems with onStateImgage (mark indicating the selected menu item) or chevon ">>" extending the tool bar in case space is not sufficient. This is a problem in Apples SVG library, see [https://trac.wxwidgets.org/ticket/19023](https://trac.wxwidgets.org/ticket/19023). setlocale(LC_NUMERIC, ...) must be left as "C" or one of the languages using a dot as decimal point. 

### Building wxWidgets for pwsafe
**This procdure works for both x86\_64 and arm64 (Apple Silicon). 
It builds wxWidgets for a Universal Binary**

**If you are trying to build pwsafe for an older I386 or x86\_32 machine, this procedure may work, but 
it has not been tested.  The current macOS and Xcode no longer support 32-bit executables. You may need to adapt these procedures for your older platform. Or it might be better to start with older versions of wxWidgets and pwsafe that were supported on that platform.**

To build pwsafe, you 
need to build wxWidgets first, in a way that is compatible with pwsafe's project settings.
The Misc directory in pwsafe sources has a script called "osx-build-wx" which does exactly 
that.  It basically runs the wxWidgets "configure" script such that wxWidgets will be built 
with settings that are compatible with pwsafe's project settings, while retaining 
the ability to run on older versions of OS X as far back as possible.  It is possible that 
pwsafe built with such a build of wxWidgets will run on OS X 10.7, but this has not been verified.

You can pass it the "-n" option to show what parameters it will pass to the configure script.

osx-build-wx has to be run from your wxWidgets build directory.  
**NOTE: Do not copy osx-build-wx to a different directory, just use the full path to the pwsafe/Misc/osx-build-wx location.**  The osx-build-wx script retrieves some build parameters from the GitHub workflow script (in pwsafe/.github/workflows/macos-latest.yml) to ensure this build will be done the same way as the release build.  For example, if you have wxWidgets sources in "wx3", then do the following:

```
wx3 $ mkdir static-debug
wx3 $ cd static-debug
wx3/static-debug $ <path-to-pwsafe's osx-build-wx> -d
wx3/static-debug $ make
OR
wx3/static-debug $ make -j `sysctl -n hw.ncpu`
```
Note that osx-build-wx doesn't actually run make: you need to run it yourself.

The "make -j..." version will use all availble CPUs on the system.  On modern systems this is *much* faster.  
But on older systems, especially those with a single, spinning hard drive, it might be too much.  At least "-j 2" should help.
On an older system, these builds would take some time, so take a coffee break or something :-)

This process would build the Debug configuration of wxWidgets in wx3/static-debug.  It builds static libraries and
puts them in a ./lib sub-directory. To build the Release configuration, rename the directory to "static-release" and omit the -d to osx-build-wx.

**Also, you DON'T need to run "make install".**  In fact, even wxWidgets recommends against that.
If you do want to install it, edit the value for WX_PRIFIX in the osx-build-wx script before running it.
See this:

http://wiki.wxwidgets.org/Compiling_wxWidgets_using_the_command-line_(Terminal)#Why_shouldn.27t_I_run_it.3F

### Building wxWidgets for a single architecture
If you Need to build wxWidgets for a single architecture, for instance on an older platform that can only build for x86_64, 
determin your architecture using the following command:
```
uname -m
```
On Intel 64-bit versions of macOS this command will report "x86\_64". On Apple Silicon versions
of macOS (11.0 and later) this command will report "arm64".  Add the architecture reported by uname to the osx-build-wx script
with the -a option, like this:
```
wx3/static-debug $ <path-to-pwsafe's osx-build-wx> -d -a x86_64
Or, as a single command:
wx3/static-debug $ <path-to-pwsafe's osx-build-wx> -d -a `uname -m`
```

If wxWidgets builds fine but you get weird compilation errors while building pwsafe, try
re-building wxWidgets with the exact same SDK that you are building pwsafe with.  You can
get a list of all installed SDKs by running

    xcodebuild -showsdks

Then get the path to your exact sdk by running

    xcodebuild -version -sdk macosx10.9 Path

Pass that to the build script with the -k option

    wx3/static-debug $ <path-to-pwsafe's osx-build-wx> -d -k <path from above>

Or, just pass the command's output directly

    wx3/static-debug $ <path-to-pwsafe's osx-build-wx> -d -k `xcodebuild -version -sdk macosx10.9 Path`

### Point Xcode to your wxWidgets Build

For users using Xcode to build the application you must link to the wxWidgets libraries and include files. You need to build **pwsafe-release.xcconfig** and/or **pwsafe-debug.xcconfig** files in the **pwsafe/Xcode** directory, derived from **wx-config** file located in **static-release** and/or **static-debug** folder of the native generated wxWidgets environment (see above).

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
file (pwsafe/Xcode/pwsafe-xcode6.xcodeproj), and hit 'Cmd-B' (or from the menu Product => Build).
This will build the currently selected scheme.

Or, from the command line:
```
xcodebuild -project pwsafe-xcode6.xcodeproj -scheme pwsafe -configuration <Debug|Release>
```

### Debug and Release Configs
You need to decide whether to build the Debug or Release configuration of
pwsafe. Apple has changed the way we (or at least I) used to view Debug and
Release configurations. Select "pwsafe" or "pwsafe-debug" from Product
Menu => Scheme to select the Release or Debug configuration respectively. And, if you are
building pwsafe for just yourself, see that the architecture in Product Menu => Destination
matches your Mac's architecture. By default, the Project file is set-up so that "pwsafe-debug" will build for 
your Mac's architecture and "pwsafe" will build a universal binary. (Provided wxWidgets was built that way. See above.)
To build a single architecture release, go to the project level build settings and change "**Build active architecture only**"
to "yes".

At this point, just hitting Cmd-B or selecting Product => Build from the menu will build the chosen pwsafe configuration.

### Where is pwsafe.app?

```
From the Xcode Menu:
Product => Show Build Folder in Finder

From the command line:
xcodebuild -project pwsafe-xcode6.xcodeproj -configuration <Debug|Release> -showBuildSettings | grep TARGET_BUILD_DIR
```

## Build installation package
This procedure will build a .dmg file for the Universal Binary or the current machine's architecture, 
depending on how pwsafe was built, above.

Build pwsafe for release. The installation package tools only create a release .dmg file.

For this part of the procedure, you need to install **gettext** and **create-dmg**. For example, using Homebrew:
```
brew install gettext create-dmg
```

After building the pwsafe.app (in Xcode), you can create a .dmg file for installing pwsafe into the Applications folder.

To begin this process you might need to make some edits to the pwsafe/install/macosx/Makefile. If you have followed this procedure, the defaults should work.

* Set the RELDIR variable to the location of pwsafe.app
* Set the WXDIR variable to the location of the "locale" folder
* Set the MACHINEARCH variable if you are building an architecture specific version

For example:

```
RELDIR=../../../Xcode/build/Products/Release/
WXDIR=../../../wxWidgets-3.1.5/locale
MACHINEARCH=$(shell uname -m)
```

To create the .dmg file

```
cd pwsafe/install/macosx
make
```

The first time you run it you might be prompted to allow Finder automation. You need to allow it for the build to complete correctly. The file will appear in the pwsafe directory. It's name will be something like:

```
pwsafe-universal-1.17.0.dmg
or
pwsafe-x86_64-1.17.0.dmg
```

The file name includes arm64 or x86\_64 to indicate a single architecture build.

### Known Issues with macOS Install

On a clean, initial install, the first time pwsafe is run the language may default to German. This
appears to be an issue in wxWidgets on Apple M1 machines. If you encounter this problem, open a 
pwsafe file. The third menu item from the right is the "manage" submenu. Click on it and select
the last/bottom menu item. This will give you a list of languages. Pick the desired language (e.g. English).
