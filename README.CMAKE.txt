Why Cmake?

- Cmake provides a portable way of configuring the build environment
  for passwordsafe.
- Source tree is untouched, all build-related files are in specified
  "build" subdirectory (by convention under the main project
  directory, "build" on unix/mac, "_build" on Windows.
  passwordsafe.sln/Makefile is created under build directory.
  (I'm using this to keep _build-mfc and _build-wx off the same source tree).
- Supports all Visual Studio versions, saving the need to update
  manually when adding/removing files.


Step-by-step for getting started with Cmake using Visual Studio 2015.

1. After installing cmake, create a _build directory under pwsafe's
toplevel directory. To keep MFC & wxWidgets files separate, suggest
_build_mfc & _build_wx.

2. Start cmake-gui. Set "Where is the source code" to the top of the
git repository source tree, and "Where to build the binaries" to the
_build directory you created in the previous step depending on whether
you wish to build the MFC or wxWidgets version.  To select the
wxWidgets version, check the box WX_WINDOWS, otherwise you will build the
MFC version.

3. Click on Configure. Select the Visual Studio platform. Cmake will
think a bit and the report errors: "CMake Error at [...] (message):
  Could NOT find GTest (missing: GTEST_LIBRARY GTEST_INCLUDE_DIR
  GTEST_MAIN_LIBRARY)
To fix this, click on GTEST_ROOT and set it to the directory where
gtest is installed on your machine - for example: C:/local/src/gtest-svn/
Note: if you do not use the default locations (e.g. different directories
for the VS2012 & VS2015 builds), Cmake will not find the libraries and you
will have to specify their absolute path e.g.
GTEST_INCLUDE_DIR        C:/local/src/gtest-svn/include
GTEST_LIBRARY            C:/local/src/gtest-svn/build-vc14/Release/gtest.lib
GTEST_LIBRARY_DEBUG      C:/local/src/gtest-svn/build-vc14/Debug/gtestd.lib
GTEST_MAIN_LIBRARY       C:/local/src/gtest-svn/build-vc14/Release/gtest_main.lib
GTEST_MAIN_LIBRARY_DEBUG C:/local/src/gtest-svn/build-vc14/Debug/gtest_maind.lib

4. If you wish to support Xerces XML processing, make sure that you have mot
checked NO_XML and check the "Advanced" checkbox, and set the values of
XercesC_INCLUDE_DIR, XercesC_LIBRARY_DEBUG and XercesC_LIBRARY_RELEASE
to the correct values.  Note: Currently only 32-bit compilations are supported.
For example:
XercesC_INCLUDE_DIR     C:/local/xerces-c-3.1.3-x86_64-windows-vc-14.0/include
XercesC_LIBRARY_DEBUG   C:/local/xerces-c-3.1.3-x86-windows-vc-14.0/lib/xerces-c_static_3D.lib
XercesC_LIBRARY_RELEASE C:/local/xerces-c-3.1.3-x86-windows-vc-14.0/lib/xerces-c_static_3.lib

Note: Microsoft XML is not yet supported via Cmake.

5. If you wish to build the wxWidgets version, check the box WX_WINDOWS and
pick a different _build directory (e.g. _build_wx) so that your MFC files are
not overwritten.  You will have to click Configure whenever you change the _build
directory.

In addition, you will need to define the following:
wxWidgets_ROOT_DIR     C:/local/wxWidgets/3.0.2/
wxWidgets_LIB          C:/local/wxWidgets/3.0.2/lib/vc140_lib

You may wish to define other variables from your environment e.g.
HHC                   C:/Program Files (x86)/HTML Help Workshop/hhc.exe
PERL_EXECUTABLE       perl.exe

Note: wxWidgets_wxrc_EXECUTABLE is not required for pwsafe wxWidgets builds.

5. Click on Generate. This will create the passwordsafe.sln file in the appropriate
_build.

6. Open passwordsafe.sln and right-click on the pwsafe project, select
"Set as StartUp Project" and then build the solution.

For Linux, similarly, only to get started, do:
$ mkdir build
$ cd build
$ cmake-gui .. # configure stuff as needed
$ cmake ..
$ make
