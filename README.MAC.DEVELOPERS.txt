PasswordSafe can be built and run on Mac OS X with Xcode.  The
Xcode directory under PasswordSafe source checkout contains the 
Xcode project file for doing so.

The Mac OS X build requires wxWidgets (2.8.11 recommended).
Mac OS X ships with some version of wxWidgets (2.8.8 on Snow Leopard),
but only the debug binaries. We haven't tried building with Apple's
build of wxWidgets.  We always built wxWidges ourselves, partly because
we needed the release builds also, and partly because we wanted to link
statically so that we could distribute the binaries on as many platforms
as possible.

In order to create binaries that will run on as many versions of OS X as
possible, we set up the Xcode project file to build using the 10.4 sdk
as well as gcc 4.0, although the defaults for Snow Leopard are 
higher versions of both.  Therefore, you will need to build wxWidgets 
accordingly so that PasswordSafe can link with it.  For the same reason, 
we link statically rather than dynamically.

Please use the 'Misc/osx-build-wx' script to build wxWidgets so that it
is compatible with PasswordSafe's Xcode project.  You only need to do this:
1. Download wxMac-2.8.11.tar.gz
2. tar xzf wxMac-2.8.11.tar.gz
3. cd wxMac-2.8.11
4. mkdir static-release ; cd static-release ; ...../Misc/osx-build-wx
5. mkdir static-debug ; cd static-debug ; ....../Misc/osx-build-wx DEBUG

This will build the static (.a's) version of wxWidgets.  The debug build
would end up in static-debug and release in static-release. It is necessary
to build them in separate directories otherwise the wx-config from one will
overwrite the other.

Now, go to the Xcode subdirectory of your pwsafe source checkout, and do this

./generate-configs -d <full-path-to-static-debug/wx-config> -r <full-path-to-static-release/wx-config>

This will generate pwsafe-debug.xcconfig and pwsafe-release.xcconfig in Xcode
directory. Xcode will read the lcoation of your wxWidgets builds from these files.
This is a one-time effort, unless you rebuild wxWidgets in some other location.

Now open pwsafe.xcodeproj in Xcode and build the 'pwsafe' target.  Keep
your fingers crossed :-).  Once built, you should be able to use your 
PasswordSafe databases from Windows/Linux without any problems.

If things don't work, or you wish to improve them nonetheless, please get
in touch with developers.

Note that some of the XML related functionality is still not in place for the
OS X build.  We will get those in sometime.

