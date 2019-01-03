#!/usr/bin/env perl
#
# Copyright (c) 2003-2019 Rony Shapiro <ronys@pwsafe.org>.
# All rights reserved. Use of the code is allowed under the
# Artistic License 2.0 terms, as specified in the LICENSE file
# distributed with this code, or available from
# http://www.opensource.org/licenses/artistic-license-2.0.php
#
# A simple utility to generate C++ mapping code between wchar_t values from
# an ISO-10646 encoded string and its corresponding X Unicode keysym.
# 
# Many of the keysyms have same values as the UNICODE char value + 0x01000000
# but not all.  Hence, we need to maintain the mapping ourselves
#
# Usage: $0 <full path to keysymdef.h>
#
# keysymdef.h is usually in /usr/include/X11
#
# Output is written to stdout.  For pwsafe, it should go to src/os/unix/unicode2keysym.cpp
#
use strict;
use warnings;

sub StartFile;
sub EndFile;

sub usage {
    print "Usage: $0 <full path to keysymdef.h>\n";
    exit 1;
}

&usage unless (@ARGV == 1);

# These come from keysymdef.h itself, except that I had to escape the + after U to U\+
my $re_good       = qr'^\#define XK_([a-zA-Z_0-9]+)\s+0x([0-9a-f]+)\s*\/\* U\+([0-9A-F]{4,6}) (.*) \*\/\s*$';
my $re_deprecated = qr'^\#define XK_([a-zA-Z_0-9]+)\s+0x([0-9a-f]+)\s*\/\*\(U\+([0-9A-F]{4,6}) (.*)\)\*\/\s*$';
# This doesn't match anything
#my $re_algo     = qr'^\#define XK_([a-zA-Z_0-9]+)\s+0x([0-9a-f]+)\s*(\/\*\s*(.*)\s*\*\/)?\s*$/';

&StartFile;

my %charsymmap;

sub saveSymbol {
    my ($symname, $keysym, $wchar)  = @_;
    # don't need in map if can be gotten by adding 0x01000000, or is LATIN1 (i.e. < 256)
	my %newkey = (keysym => $keysym, symname => $symname);
	$charsymmap{$wchar} = \%newkey if ((hex $wchar | 0x01000000) != hex $keysym && hex $wchar > 255 && hex $keysym <= 0x20ff);
}

while (<>) {
  if (/$re_good/) {
    saveSymbol($1, $2, $3);
    
    # Use these for testing
    #print "{0x$wchar, 0x$keysym},\n";                                                     # output everything
    #print "{0x$wchar, 0x$keysym},\n" if (hex $wchar == hex $keysym);                      # only if unicode == keysym (LATIN1 and 0x20AC)
    #print "{0x$wchar, 0x$keysym},\n" if (hex $wchar != hex $keysym);                      # only if unicode != keysym (except LATIN1 and 0x20AC)
    #print "{0x$wchar, 0x$keysym},\n" if ($keysym =~/^10+$wchar$/i);                       # unicode + 0x01000000 == keysym
    #print "{0x$wchar, 0x$keysym},\n" if (hex $wchar | 0x01000000) == hex $keysym;         # unicode + 0x01000000 == keysym
    #print "{0x$wchar, 0x$keysym},\n" if (($keysym =~/^10+$wchar$/i) && hex $wchar > 255); # same as above, using regex
  }
  elsif (/$re_deprecated/) {
	# don't overwrite existing entries with deprecated ones
	if (not exists $charsymmap{$3}) {
		saveSymbol($1, $2, $3);
	}
  }
}

# sort the keys to generate same output every time unless keysymdefs.h changes
foreach my $wchar (sort keys %charsymmap) {
	print "\tcase 0x$wchar: return 0x$charsymmap{$wchar}{keysym};\t\t// $charsymmap{$wchar}{symname}\n";
}

&EndFile;

sub StartFile {
  print << "EOT";
/*
* Copyright (c) 2003-2019 Rony Shapiro <ronys\@pwsafe.org>.
* All rights reserved. Use of the code is allowed under the
* Artistic License 2.0 terms, as specified in the LICENSE file
* distributed with this code, or available from
* http://www.opensource.org/licenses/artistic-license-2.0.php
*/

/*
 * unicode2keysym - Contains a map between UNICODE wchar_t values and X keysyms
 *
 * This file is auto-generated using Misc/unicode2keysym.pl.  Do not edit by hand!!
 *
 */

#include <X11/Intrinsic.h>
#include "./unicode2keysym.h"


KeySym unicode2keysym(wchar_t wc)
{
  switch (wc) {
EOT
}

sub EndFile {
  print << "EOT";
	default: return NoSymbol;
  };
}

EOT
}
