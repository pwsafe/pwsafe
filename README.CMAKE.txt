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


Step-by-step for getting started with Cmake using Visual Studio 2015
--------------------------------------------------------------------

1. After installing cmake, create a _build directory under pwsafe's
toplevel directory. To keep MFC & wxWidgets files separate, suggest
_build_mfc & _build_wx.

2. Start cmake-gui. Set "Where is the source code" to the top of the
git repository source tree, and "Where to build the binaries" to the
_build directory you created in the previous step depending on whether
you wish to build the MFC or wxWidgets version.  To select the
wxWidgets version, check the box WX_WINDOWS, otherwise you will build the
MFC version.

3. Click on Configure. Select the "Visual Studio 14 2015" generator. 
If you want to build x64, choose "Visual Studio 14 2015 Win64" generator. 
Cmake will think a bit and then report errors: "CMake Error at [...] (message):
  Could NOT find GTest (missing: GTEST_LIBRARY GTEST_INCLUDE_DIR
  GTEST_MAIN_LIBRARY)
To fix this, click on the "Advanced" check box to see all of the variables.
Find GTEST_ROOT in the list and set it to the directory where
gtest is installed on your machine - for example: C:/local/src/gtest-svn/.
This should enable cmake to at least find the include directory.
Note: if you do not use the default locations for the built libraries
(e.g. different directories for the VS2012 & VS2015 builds), cmake will
not find the libraries but you can specify the path relative to the
GTEST_ROOT, e.g.,

You need to define all of these values.

CMake Variable           Your Configuration
--------------           ------------------
GTEST_ROOT               C:/local/src/gtest-svn
GTEST_LIBRARY            .\build-vc14\Release
GTEST_LIBRARY_DEBUG      .\build-vc14\Debug
GTEST_MAIN_LIBRARY       .\build-vc14\Release
GTEST_MAIN_LIBRARY_DEBUG .\build-vc14\Debug

If you are building x64, you need to build gtest in x64 mode. The VS
solutions that come with gtest do not include an x64 build, so you 
will need to create an x64 build configuration. Only the Debug/x64
configuration is needed. You should name the output directory in 
a way that that identifies it clearly (e.g. Debug64). Otherwise,
it is easy to end up with x86/x64 collisions. 

4. XML support: For the MFC build, you can choose either: (a)
XML_MSXML, which will use Microsoft's implementation for XML
validation, (b) XML_XERCESC, which uses the XercesC library, or (c)
neither, in which case the compiled program will not be able to import
XML data.
The wx build doesn't support XML_MSXML.
For Xerces XML processing, check the "Advanced" checkbox, and set the
values of XercesC_INCLUDE_DIR, XercesC_LIBRARY_DEBUG and
XercesC_LIBRARY_RELEASE to the correct values.
Note: Currently only 32-bit compilations are supported.
For example:
XercesC_INCLUDE_DIR     C:/local/xerces-c-3.1.3-x86_64-windows-vc-14.0/include
XercesC_LIBRARY_DEBUG   C:/local/xerces-c-3.1.3-x86-windows-vc-14.0/lib
XercesC_LIBRARY_RELEASE C:/local/xerces-c-3.1.3-x86-windows-vc-14.0/lib

Again, if you are building x64, you need to build Xerces in x64 mode.

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
_build directory.

6. Open passwordsafe.sln and right-click on the pwsafe project, select
"Set as StartUp Project" and then build the solution.

Linux
-----

For Linux, similarly, only to get started, do:
$ mkdir build
$ cd build
$ cmake-gui .. # configure stuff as needed
$ cmake ..
$ make
