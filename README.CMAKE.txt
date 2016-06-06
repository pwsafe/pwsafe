Why Cmake?

- Cmake provides a portable way of configuring the build environment
  for passwordsafe.
- Source tree is untouched, all build-related files are in specified
  "build" subdirectory (by convention under the main project
  directory, "build" on unix/mac, "_build" on
  Windows. passwordsafe.sln/Makefile is created under build directory.
  (I'm using this to keep _build-mfc and _build-wx off the same source tree).
- Supports all visual studio versions, saving the need to update
  manually when adding/removing files.


Step-by-step for getting started with Cmake using Visual Studio 2015.

1. After installing cmake, create a _build directory under pwsafe's
toplevel directory.
2. Start cmake-gui. Set "Where is the source code" to the top of the
git repository source tree, and "Where to build the binaries" to the
_build directory you created in the previous step.
3. Click on Configure. Select the Visual Studio platform. Cmake will
think a bit and the report errors: "CMake Error at [...] (message):
  Could NOT find GTest (missing: GTEST_LIBRARY GTEST_INCLUDE_DIR
  GTEST_MAIN_LIBRARY)
To fix this, click on GTEST_ROOT and set it to the directory where
gtest is installed on your machine.
4. Check the "Advanced" checkbox, and set the values of
XercesC_INCLUDE_DIR, XercesC_LIBRARY_DEBUG and XercesC_LIBRARY_RELEASE
to the correct values.
5. Click on Generate. This will create the passwordsafe.sln file in
_build.
6. Open passwordsafe.sln and right-click on the pwsafe project, select
"Set as StartUp Project".
7. You may need to go back and set paths to external libraries, such
as GTEST_LIBRARY, GTEST_LIBRARY_DEBUG, GTEST_MAIN_LIBRARY and
GTEST_MAIN_LIBRARY_DEBUG. Click on Cofigure and Generate after setting
these, and Visual Studio will prompt you to refresh the project
files.

For Linux, similarly, only to get started, do:
$ mkdir build
$ cd build
$ cmake-gui .. # configure stuff as needed
$ cmake ..
$ make
