PasswordSafe V4 requires the gtest suite in the DebugM build – specifically the DebugM build of the
coretest project.

To generate the required library, open gtest.sln in the "msvc" sub-directory in the gtest directory.

You only need to build the Debug version of gtest library (gtestd.lib) in the gtest project of the
solution "gtest.sln". PasswordSafe does not use the "gtest-md.sln" solution nor following projects
in gtest.sln:
  a. gtest_main
  b. gtest_prod_test
  c. gtest_unittest

However, you can also make the similar changes to the following in these project if you wish to
run the gtest Test programs.

The following changes should be made to the converted project file:
1. Configuration Properties: General: (NN is 12 for VS2013 & 14 for VS2015)
  a. All configurations:
    Output directory:       ..\build-vcNN/$(Configuration)\
    Intermediate Directory: $(Configuration)/$(ProjectName)\

  b. Debug configuration:
    Target Name:            $(ProjectName)d

  c. Release configuration (not used by PasswordSafe):
    Target Name:            $(ProjectName)

2. Configuration Properties: C/C++: Output Files
  a. Debug configuration:
    Program Database File Name: $(OutDir)\$(ProjectName)d.pdb

  b. Release configuration(not used by PasswordSafe):
    Program Database File Name: $(OutDir)\$(ProjectName).pdb

Please remember to re-run the configure-12.vbs or configure-14.vbs script to set the necessary gtest
include and library directories.  These would normally be the subdirectories "include" and either
"build-vc12" or "build-vc14" of the "gtest-1.7.0" directory.
