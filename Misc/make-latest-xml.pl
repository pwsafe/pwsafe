#!/usr/bin/perl -w

# This script prints an XML fragment that can be parsed by
# the application to determine if there's a newer release available.
# The script parses the file passed as arg (version.h)
# to get the information it needs.
# For the XML format description, see ../src/ui/Windows/AboutDlg.cpp

my @verline;
my ($WinMajor, $WinMinor, $WinBuild, $WinRevision);
my ($LinMajor, $LinMinor, $LinBuild, $LinRevision);

while (<>) {
    if (/#define\sPRODUCTVER/) {
        s/,/ /g;
        @verline = split;
        shift @verline; shift @verline;
        ($WinMajor, $WinMinor, $WinBuild, $WinRevision) = @verline;
    } elsif (/#define\sLINUXPRODVER/) {
        s/,/ /g;
        @verline = split;
        shift @verline; shift @verline;
        ($LinMajor, $LinMinor, $LinBuild, $LinRevision) = @verline;
    }
};

print "<VersionInfo>\n";
print " <Product name=PasswordSafe variant=PC";
print " major=$WinMajor minor=$WinMinor build=$WinBuild rev=$WinRevision />\n";
print " <Product name=PasswordSafe variant=PPc";
print " major=1 minor=9 build=2 rev=100 />\n";
print " <Product name=PasswordSafe variant=U3";
print " major=$WinMajor minor=$WinMinor build=$WinBuild rev=$WinRevision />\n";
print " <Product name=PasswordSafe variant=Linux";
print " major=$LinMajor minor=$LinMinor build=$LinBuild rev=$LinRevision />\n";
print "</VersionInfo>\n";
exit 0;
