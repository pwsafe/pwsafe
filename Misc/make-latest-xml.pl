#!/usr/bin/env perl
#
# Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
# All rights reserved. Use of the code is allowed under the
# Artistic License 2.0 terms, as specified in the LICENSE file
# distributed with this code, or available from
# http://www.opensource.org/licenses/artistic-license-2.0.php
#
# This script prints an XML fragment that can be parsed by
# the application to determine if there's a newer release available.
# The script parses the version files passed as arg
# to get the information it needs.
#
# Note that the "rev" attribute should be the git abbrev assiciated
# with the variant, but this isn't checked, as is tricky to get right,
# so deferring for now.
#
# For the XML format description, see ../src/ui/Vardows/AboutDlg.cpp

use warnings;


sub parse_version {
    my @verline;
    my ($Variant, $VarMajor, $VarMinor, $VarBuild);
    my $Rev = 0; # Getting this right for al Variants is tricky, leaving as 0 for now

    if ($_ eq "version.mfc") {
        $Variant = "PC";
    } elsif ($_ eq "version.wx") {
        $Variant = "Linux";
    } else {
        die "Unknown version file - can't determine variant";
    }
    open(INFILE, "<$_") || die "Couldn't open $_";
    while (<INFILE>) {
        if (/^VER_MAJOR/) {
            @verline = split;
            $VarMajor = $verline[2];
        } elsif (/^VER_MINOR/) {
            @verline = split;
            $VarMinor = $verline[2];
        } elsif (/^VER_REV/) {
            @verline = split;
            $VarBuild = $verline[2];
        }
    };
    close(INFILE);
    print " <Product name=\"PasswordSafe\" variant=\"$Variant\"";
    print " major=\"$VarMajor\" minor=\"$VarMinor\" build=\"$VarBuild\" rev=\"$Rev\" />\n";
}


print "<VersionInfo>\n";
my @myargs=@ARGV;
foreach (@myargs) {
    parse_version($_);
};
print "</VersionInfo>\n";
exit 0;
