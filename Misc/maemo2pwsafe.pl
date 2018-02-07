#!/usr/bin/env perl
#
# Quick hack to convert text files exported by maemo pwsafe clone
# to something that PasswordSafe can import as text.
#
# Thanks to OMOIKANE for requesting this and submitting samples.
#
# Usage: maemo2pwsafe.pl infile > outfile
# where infile is the exported text file, and outfile is the name
# of the file that will be importable by PasswordSafe.
#################################################################
#
# Copyright (c) 2012-2018 Rony Shapiro <ronys@pwsafe.org>.
# All rights reserved. Use of the code is allowed under the
# Artistic License 2.0 terms, as specified in the LICENSE file
# distributed with this code, or available from
# http://www.opensource.org/licenses/artistic-license-2.0.php
#
#################################################################

use strict;
use warnings;

my ($group, $title, $user, $password, $notes);
my $dummy;
my $inNotes = 0;

my $sep = pack("CC", 0xc2, 0xbb);

print "Group/Title\tUsername\tPassword\tNotes\n";

while (<>) {
    if (!$inNotes) {
        if (/^Group:/) {
            ($dummy, $group) = split ": "; chomp $group;
        } elsif (/^Title:/) {
            ($dummy, $title) = split ": "; chomp $title;
            # Following because tr/// can't take a var as the second arg.
            my ($c, $st);
            foreach $c (split //, $title) {
                if ($c ne ".") {
                    $st .= $c;
                } else {
                    $st .= $sep;
                }
            }
            $title = $st;
        } elsif (/^User:/) {
            ($dummy, $user) = split ": "; chomp $user;
        } elsif (/^Password/) {
            ($dummy, $password) = split ": "; chomp $password;
        } elsif (/^Notes/) {
            ($dummy, $notes) = split ": ";
            if ($notes) {
                $inNotes = 1;
            }
        } elsif (/^--$/) {
            $inNotes = 0;
            if ($notes) {chomp $notes;}
            &print_rec($group, $title, $user, $password, $notes);
        }
    } else { # inNotes
        if (/^--$/) {
            $inNotes = 0;
            if ($notes) {
                chomp $notes; chomp $notes;
            }
            &print_rec($group, $title, $user, $password, $notes);
        } else {
            $notes .= $_;
        }
    }
}

exit 0;

sub print_rec {
    my ($g, $t, $u, $p, $n) = @_;
    if (!$n) {$n = "";}
    print "$g.$t\t$u\t$p\t\"$n\"\n";
}

