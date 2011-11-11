PasswordSafe can be built and run on Mac OS X with Xcode.  The
Xcode directory under PasswordSafe source checkout contains the 
Xcode project file for building it.

Requirements
============

1. Xcode
2. wxWidgets
3. Perl

Xcode
=====

I've only tried building with Xcode 3.2.x that ships with Snow Leopard, not Xcode 4. You can
try building with Xcode 4, but some targets require gcc/g++ 4.0 and 10.4u sdk.  If you
don't have those, try building some of the other targets (see below).

wxWidgets
========
 
Unless you want a 64-bit build of pwsafe, use the latest stable 2.8 release
of pwsafe.  64-bit builds require 2.9 release of wxWidgets which is still
under development and isn't very stable (in my experience, as compared to 2.8).

Also note that Mac OS X ships with some version of wxWidgets (2.8.8 on Snow Leopard),
but only the debug binaries. We haven't tried building with Apple's
build of wxWidgets.  We always built wxWidges ourselves, partly because
we needed the release builds also, and partly because we wanted to link
statically so that we could distribute the binaries on as many platforms
as possible.

Perl
====
Mac OS X (at least till 10.6) ships with perl, which should suffice.


Building PasswordSafe
=====================

There are three steps to building PasswordSafe on Mac OS X:

1. Building wxWidgets
2. Generate xcconfig files
3. Build PasswordSafe

The first two need to be performed only once, unless you rebuild wxWidgets for some reason.


Building wxWidgets
=================
How you build wxWidgets depends on the version of Mac OS X you are building on, and
the platform (hardware & OS) on which you want to run your build of PasswordSafe.  PasswordSafe
can be built in four flavours (four different Xcode targets):

                wxWidgets
 Target         required      xcconfig files               Binary types     Deployment target    Base SDK
 --------------------------------------------------------------------------------------------------------
 pwsafe           2.8     pwsafe-debug.xcconfig             i386 + ppc       10.4+                10.4
                          pwsafe-release.xcconfig           

 pwsafe-i386      2.8     pwsafe-debug.xcconfig        	    i386             10.4+                10.4
                          pwsafe-release.xcconfig      

 pwsafe64         2.9     pwsafe64-debug.xcconfig           X86_64           Compiler Default     Current Mac OS
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

the 'pwsafe-llvm' is an experimental target to build PasswordSafe with the llvm compiler.
This is only of interest if you are developing PasswordSafe on OS X and want to use
the static analysis abilities of clang (which are not available for C++ with the stock
llvm 1.7 shipped with OS X 10.6 or Xcode 3.2.x).  But the builds seem faster using llvm.


Building wxWidgets for pwsafe & pwsafe-i386 targets
===================================================

Since these targets are intended to run on 10.4 and above, we set these up to build
with 10.4 sdk.  Which in turn implies that wxWidgets 2.8 also needs to be built with 10.4 sdk.
That involves passing a ton of parameters to the "./configure" script for building wxWidgets.
There's a shell script (osx-build-wx) to aid with doing that, in the 'Misc' folder.  You only
need to do this

1. Download wxMac-2.8.12.tar.gz (or the latest 2.8 version)
2. tar xzf wxMac-2.8.12.tar.gz
3. cd wxMac-2.8.11
4. mkdir static-release ; cd static-release ; ...../Misc/osx-build-wx
5. mkdir static-debug ; cd static-debug ; ....../Misc/osx-build-wx DEBUG

This will build the static (.a's) version of wxWidgets.  The debug build
would end up in static-debug and release in static-release. It is necessary
to build them in separate directories otherwise the "wx-config" script from one will
overwrite the other.  Of course, you don't need both Debug and Release builds
of wxWidgets unless you need both Debug and Release builds of PasswordSafe.

It is possible that PasswordSafe & wxWidgets are be built with 10.6 sdk and still run on 10.4+ if the
deployment target is set appropriately in Xcode, but I have no way of trying that.
I'm also not aware of what precautions to take in the code to not add any dependencies
that cannot be satisfied on 10.4


Building wxWidgets for pwsafe64 target
======================================

Essentially, you need a 64-bit build of wxWidgets, which is only possible with 2.9 series of wxWidgets or later.

1. Download wxWidgets-2.9.2.tar.gz (or the latest 2.9 release)
2. tar xzf wxWidgets-2.9.2.tar.gz
3. cd wxWidgets-2.9.2
4. mkdir static64-debug ; cd static64-debug
5  ../configure --prefix=`pwd` --disable-shared --enable-unicode --enable-debug --with-osx_cocoa
5. make

That last bit about "--with-osx_cocoa" is what ensures you get a 64-bit build of wxWidgets.  For
Release configuration, just change "--enable-debug" with "--disable-debug".  Of course, if you're
only going to build one of Debug or Release configurations of PasswordSafe, you only need to build 
the corresponding configuration of wxWidgets.


Building wxWidgets for pwsafe-llvm target
========================================

I use an llvm-built version of wxWidgets when I build pwsafe-llvm, but its
probably not necessary.  The gcc and llvm libraries/binaries are compatible
with each other.  Still, if you want to build wxWidgets with llvm, do this:

1. Download wxMac-2.8.12.tar.gz (or the latest 2.8 release)
2. tar xzf wxWidgets-2.8.12.tar.gz
3. cd wxWidgets-2.8.12
4. mkdir static-llvm-debug ; cd static-llvm-debug ;
5. ../configure --prefix=`pwd` CC='llvm-gcc-4.2` CXX='llvm-g++-4.2' CFLAGS='-arch i386' CXXFLAGS='-arch i386' CPPFLAGS='-arch i386' LDFLAGS='-arch i386' OBJCFLAGS='-arch i386' OBJCXXFLAGS='-arch i386' --enable-debug --disable-shared --disable-copmat24 --enable-unicode
6. make

For the release build, do the same, except replace "--enabe-debug" with "--disable-debug".
 


Generate xcconfig files
=========================

Having built wxWidgets, unless you are willing to "make install" wxWidgets and overwrite 
whatever shipped with your os, you will have to tell Xcode where to pick up your wxWidgets
headers/libs from. wxWidgets makes it easy by creating a script called 'wx-config' during its 
build process (command-line makefile based builds only) that spits out the correct location 
of headers/libs as well as compiler and liker settings compatible with that build of wxWidgets.
This is used in UNIX makefiles to compile/link with the desired build of wxWidgets where its 
trivial to read in the settings from outputs of external commands.

Since Xcode can't pick up settings from output of external commands , we use a script to 
put those settings into configuration files that Xcode can use. Xcode target configurations 
can be "based on" xcconfig files, which are essentially sets of name-value pairs.  For a 
target/configuration, Xcode will use settings from the xcconfig file, if found. Else, it 
will use the values specified in its GUI.

The "Xcode/generate-configs" script generates xcconfig data from the wx-config files.
Go to the Xcode subdirectory of your pwsafe source checkout, and do this

./generate-configs -d full_path_to_wx-config  > xcconfig_file

For Release builds, the first parameter should be "-r" instead of "-d"

Substitue "xcconfig_file" with the correct xcconfig file name from the above table.  Each
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

