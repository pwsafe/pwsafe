#!/usr/bin/env perl
#
# Copyright (c) 2003-2018 Rony Shapiro <ronys@pwsafe.org>.
# All rights reserved. Use of the code is allowed under the
# Artistic License 2.0 terms, as specified in the LICENSE file
# distributed with this code, or available from
# http://www.opensource.org/licenses/artistic-license-2.0.php
#
# A simple utility to generate a version.h file from a template,
# with keyword substitution.
# The named file is replaced iff the newly generated one's
# different, to avoid spurious recompilations.
#
# Usage: $0 MAJOR=major_ver MINOR=minor_ver REV=rev SPECIAL=special_ver template outfile
# (e.g., $0 MAJOR=4 MINOR=3 REV=0 SPECIAL="" version.in version.h)
#

use strict;
use warnings;
use File::Copy;
use File::Spec;

sub usage {
    print "Usage: $0 MAJOR=x MINOR=y [REV=z] [SPECIAL=w] template outfile\n";
    exit 1;
}

my %git_loc  = (
	darwin => '/usr/local/git/bin/git',
	linux  => '/usr/bin/git',
);

&usage unless ($#ARGV >= 3 && $#ARGV <= 5);
my ($MAJOR, $MINOR, $REVISION, $SPECIAL) = (0, 0, 0, "");

my @myargs=@ARGV;
foreach (@myargs) {
    my $dummy;
    if (m/MAJOR=/) {
        ($dummy,$MAJOR) = split("=",$_); shift @ARGV;
    } elsif (m/MINOR=/) {
        ($dummy,$MINOR) = split("=",$_); shift @ARGV;
    } elsif (m/REV=/) {
        ($dummy,$REVISION) = split("=",$_); shift @ARGV;
    } elsif (m/SPECIAL=/) {
        ($dummy,$SPECIAL) = split("=",$_); shift @ARGV;
    }
}
my ($TEMPLATE, $OUTFILE) = ($ARGV[0], $ARGV[1]);

my $GIT = defined $git_loc{$^O}? $git_loc{$^O}: "/usr/local/bin/git";
my $VERSTRING;

if (-x $GIT && (-d ".git" || -d "../../../.git")) {
    $VERSTRING = `$GIT describe --all --always --dirty=+  --long`;
    chomp $VERSTRING;
    # If string is of the form heads/master-0-g5f69087, drop everything
    # to the left of the rightmost g. Otherwise, this is a branch/WIP, leave full
    # info
    $VERSTRING =~ s,^heads/master-0-,,;
} else {
    # No git, building from tarball, srpm, etc.
    $VERSTRING = "local";
}


#Now that we're done with the formalities, let's get to work:
my $TMPFILE = File::Spec->tmpdir()."/v$$";


open(TH, "<$TEMPLATE") || die "Couldn't read $TEMPLATE\n";
open(VH, ">$TMPFILE") || die "Couldn't open $TMPFILE for writing\n";

while (<TH>) {
    s/\@pwsafe_VERSION_MAJOR\@/$MAJOR/;
    s/\@pwsafe_VERSION_MINOR\@/$MINOR/;
    s/\@pwsafe_REVISION\@/$REVISION/;
    s/\@pwsafe_SPECIALBUILD\@/$SPECIAL/;
    s/\@pwsafe_VERSTRING\@/$VERSTRING/;
    print VH;
}

close(TH);
close(VH);

# Replace $OUTFILE with $TMPFILE iff:
# 1. Former doesn't exist
# OR
# 2. The two differ AND the version isn't "local" (otherwise we clobber the rpm build)

if (!-e $OUTFILE) {
    move($TMPFILE, $OUTFILE) || die "Couldn't move $TMPFILE to $OUTFILE: $!\n";
} else {
    `/usr/bin/diff -q $TMPFILE $OUTFILE > /dev/null`;
    if ($VERSTRING ne "local" && $? != 0) {
        unlink $OUTFILE || die "Couldn't remove old $OUTFILE\n";
        move($TMPFILE, $OUTFILE) || die "Couldn't move $TMPFILE to $OUTFILE: $!\n";
    } else { # no changes, cleanup
        unlink $TMPFILE;
    }
}
exit 0;

