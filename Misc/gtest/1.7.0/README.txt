PasswordSafe V4 requires the gtest suite in the DebugM build – specifically the DebugM build of the
coretest project.

To generate the required library, copy the relevant directory (msvc12 if using VS2013 or msvc14
if using VS2015) into the root directory of "gtest-1.7.0"" (there should already be a msvc directory here).

You only need to build the Debug version of gtest library (gtestd.lib) in the gtest project of the
solution "gtest.sln".

Please remember to re-run the configure-12.vbs or configure14.vbs script to set the necessary gtest
include and library directories.  These would normally be the subdirectories "include" and either
"build-vc12" or "build-vc14" of the "gtest-1.7.0" directory.
