This file introduces the prebuilt libwicedmesh.a libraries and how to integrate them.

1. Files:
readme.txt - this file.
commit_version - the source code base of the GIT commit for this release.
release/libwicedmesh.a - library for iOS release building,
                         including architecture of arm7, arm64.
debug/libwicedmesh.a - library file for iOS debug building,
                       including architecture of arm7, arm64, x86_64 and i386.
libwicedmesh.a - the library file should be linked into the target framework or App.
                 For debug mode, this file should be debug/libwicedmesh.a
                 For release mode, the file should be release/libwicedmesh.a
                 By default, this file is set to debug/libwicedmesh.a

2. To let Xcode update the correct libwicedmesh.a file based on debug/release mode automatically,
Following script can be used in the "Build Phase -> Run Script" to achieve this.
Steps to setup the "Run Script":
a) Got to Xcode, select a Target, and go to "Build Phases" page.
b) Through left top "+" button on the page to add a new phase of "New Run Script Phase" in the drop list.
c) By default the new "Run Script" phase is at bottom.
d) Drag the new "Run Script" phase to the front of "Complie Sources" phase, so
   the "Run Script" can be executed before source code compiling and linking.
e) Expand the new "Run Script" phase, copy and paste following script code into the Shell.

###############################################################################
# This script is used to update libs/libwicedmesh.a automatically based on
# the build configuration of Debug or Release.
# Dudley Du <dudl@cypress.com>
###############################################################################
LIBSDIR="$SRCROOT/meshcore/libwicedmesh/libs"
if [[ ! -d "$LIBSDIR" ]]; then
echo "error: not found directory: \"$SRCROOT/$PROJECT_NAME/meshcore/libwicedmesh/libs\""
exit 1
fi

if [[ "$CONFIGURATION" == "Debug" && -f "$LIBSDIR/debug/libwicedmesh.a" ]]; then
diff "$LIBSDIR/libwicedmesh.a" "$LIBSDIR/debug/libwicedmesh.a" 2>/dev/null 1>&2
if [ $? -ne 0 ]; then
rm -f "$LIBSDIR/libwicedmesh.a"
cp -f "$LIBSDIR/debug/libwicedmesh.a" "$LIBSDIR/"
fi
fi

if [[ "$CONFIGURATION" == "Release" && -f "$LIBSDIR/release/libwicedmesh.a" ]]; then
diff "$LIBSDIR/libwicedmesh.a" "$LIBSDIR/release/libwicedmesh.a" 2>/dev/null 1>&2
if [ $? -ne 0 ]; then
rm -f "$LIBSDIR/libwicedmesh.a"
cp -f "$LIBSDIR/release/libwicedmesh.a" "$LIBSDIR/"
fi
fi

if [[ ! -f "$LIBSDIR/libwicedmesh.a" ]]; then
echo "error: fail to install $CONFIGURATION libwicedmesh.a library file, not found. Path: \"$LIBSDIR\"/libwicedmesh.a"
else
echo "success, install $CONFIGURATION libwicedmesh.a library to \"$LIBSDIR/libwicedmesh.a\""
fi
###############################################################################


3. To build the mesh core source code, following additional mesh relative macros must be defined before compiling the code.
These macros can be added through Project's "Build Settings -> Preprocessor Macros", so and to be inherited by all targets.
The macros for Debug and Release should be added as below.
Preprocessor Macros
        Debug       MESH_OVER_GATT_ONLY=1 WICED_BT_MESH_TRACE_ENABLE=1
        Release.    MESH_OVER_GATT_ONLY=1

4. Add -ObjC flag to project's "Build Settings -> Linking -> Other Linker Flags" configuration.
