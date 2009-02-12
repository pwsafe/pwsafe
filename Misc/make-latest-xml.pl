#!/usr/bin/perl -w

# This script prints an XML fragment that can be parsed by
# the application to determine if there's a newer release available.
# The script parses the file passed as arg (version.h)
# to get the information it needs.
# For the XML format description, see ../src/ui/Windows/AboutDlg.cpp

my @verline;
my ($Major, $Minor, $Build, $Revision);

while (<>) {
    if (/#define\sPRODUCTVER/) {
        s/,/ /g;
        @verline = split;
        shift @verline; shift @verline;
        ($Major, $Minor, $Build, $Revision) = @verline;
        last;
    }
};

print "<VersionInfo>\n";
print " <Product name=PasswordSafe variant=PC";
print " major=$Major minor=$Minor build=$Build rev=$Revision />\n";
print " <Product name=PasswordSafe variant=PPc";
print " major=1 minor=9 build=2 rev=100 />\n";
print " <Product name=PasswordSafe variant=U3";
print " major=$Major minor=$Minor build=$Build rev=$Revision />\n";
#print " <Product name=SWTPasswordSafe variant=Java";
#print " major=0 minor=6 build=0 rev=1230 />\n";
print "</VersionInfo>\n";
exit 0;
