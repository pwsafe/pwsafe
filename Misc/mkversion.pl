#!/usr/bin/perl -w
#
# Copyright (c) 2003-2012 Rony Shapiro <ronys@users.sourceforge.net>.
# All rights reserved. Use of the code is allowed under the
# Artistic License 2.0 terms, as specified in the LICENSE file
# distributed with this code, or available from
# http://www.opensource.org/licenses/artistic-license-2.0.php
#
# A simple utility to generate a version.h file from a template,
# with keyword substitution.
# The named file is replaced iff the newley generated one's
# different, to avoid spurious recompilations.
#
# Usage: $0 template outfile
# (typically: $0 version.in version.h)
#

use strict;
use warnings;
use File::Copy;

sub usage {
    print "Usage: $0 template outfile\n";
    exit 1;
}

my $SVNVERSION = "/usr/bin/svnversion";
die "Couldn't find $SVNVERSION" unless (-x $SVNVERSION);

my $ROOT = "../../.."; # should really be passed as arg
&usage unless ($#ARGV == 1);
my $TEMPLATE = $ARGV[0];
my $OUTFILE = $ARGV[1];

my $SVNVERSTRING = `$SVNVERSION -n $ROOT`;

#Now that we're done with the formalities, let's get to work:
my $TMPFILE = "/tmp/v$$";

my ($MAJOR, $MINOR, $REVISION);

open(TH, "<$TEMPLATE") || die "Couldn't read $TEMPLATE\n";
open(VH, ">$TMPFILE") || die "Couldn't open $TMPFILE for writing\n";

while (<TH>) {
    if (m/^#define\s+MAJORVERSION\s+(.+)$/) {
        $MAJOR=$1;
    } elsif (m/^#define\s+MINORVERSION\s+(.+)$/) {
        $MINOR=$1;
    } elsif (m/^#define\s+REVISION\s+(.+)$/) {
        $REVISION=$1;
    }
    if (m/^\#define\s+SVN_VERSION/) {
        print VH "#define SVN_VERSION \"$SVNVERSTRING\"\n";
    } elsif (m/^\#define\s+LINUXPRODVER/) {
        print VH "#define LINUXPRODVER $MAJOR, $MINOR, $REVISION, $SVNVERSTRING\n";
    } else {
        print VH;
    }
}

close(TH);
close(VH);

# Replace $OUTFILE with $TMPFILE iff:
# 1. Former doesn't exist
# OR
# 2. The two differ

if (!-e $OUTFILE) {
    move($TMPFILE, $OUTFILE) || die "Couldn't move $TMPFILE to $OUTFILE: $!\n";
} else {
    `/usr/bin/diff -q $TMPFILE $OUTFILE > /dev/null`;
    if ($? != 0) {
        unlink $OUTFILE || die "Couldn't remove old $OUTFILE\n";
        move($TMPFILE, $OUTFILE) || die "Couldn't move $TMPFILE to $OUTFILE: $!\n";
    } else { # no changes, cleanup
        unlink $TMPFILE;
    }
}
exit 0;

