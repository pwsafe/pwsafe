# Just invokes the post-build.sh script with generator (DEB|RPM) and package as arguments.
#
# CMAKE_* variables aren't available at CPACK time, so we need to
# be relative to the package directory, which defaults to the build directory
set(PB_SCRIPT "${CPACK_PACKAGE_DIRECTORY}/../Misc/post-build.sh")

execute_process(COMMAND ${PB_SCRIPT} ${CPACK_GENERATOR} ${CPACK_PACKAGE_FILES})