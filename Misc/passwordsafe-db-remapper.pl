#!/usr/bin/perl
#
# passwordsafe-db-remapper v1.0.0
#
# Written by Eddy L O Jansson <eddy@klopper.net>
# Donated into the Public Domain
#
# This program will convert an exported Password Safe v3.0BETA1 database
# into a modern format that can be imported into the 3.1x series, which
# do not natively support this database format.
#
# To export from PS, use File -> Export To -> Plain text and enable '^' as
# the line delimiter. Then run the output through this program.
#
# Usage:
#  $ passwordsafe-db-remapper <exported_30b1.txt >new.txt
#
# Notes:
#  The code is a bit verbose, it was written to be easy to extend to handle
#  any future similar remappings.
#
# History
# ===========================================================================
# 2007-12-30  1.0.0  eloj  First version done.
#
# To do:
# ===========================================================================
#  * Nothing
#
use strict;
# Currently disabled since we only have one mapping and don't need the extra dependency.
#use Getopt::Long;
#use POSIX "strftime";

my $program_version = "1.0.0-20071230";

my %cfg = (
 ESCAPECHAR => '^',
 SEPARATOR => "\t",
 DEFAULT_MAPPING => "30b1",
);

my %columns = (
  0 => { NAME => 'Group/Title' },
  1 => { NAME => 'Username' },
  2 => { NAME => 'Password' },
  3 => { NAME => 'URL' },
  4 => { NAME => 'AutoType' },
  5 => { NAME => 'Created Time' },
  6 => { NAME => 'Password Modified Time' },
  7 => { NAME => 'Last Access Time' },
  8 => { NAME => 'Password Expiry Date' },
  9 => { NAME => 'Record Modified Time' },
 10 => { NAME => 'History' },
 11 => { NAME => 'Notes' },
);

# Defines a mapping from 3.0beta1 to modern format.
my %ps30b1_to_ps31x = (
  NAME => "PasswordSafe v3.0BETA1 to PasswordSafe v3.1x conversion",
  HEADER_SUB => \&modern_header,
  COLMAP => {
    0 => { COLID => 0, TRANSFORM_SUB => \&fixup_grouptitle },
    1 => { COLID => 1 },
    2 => { COLID => 2 },
    3 => { COLID => 3 },
    4 => { COLID => 4 },
    5 => { COLID => 5 },
    6 => { COLID => 11 },
  }
);

# For the user to be able to select between mappings, use a tag code.
my %mappings = (
  "30b1" => \%ps30b1_to_ps31x
);

sub output_version
{
  print "passwordsafe-db-remapper version ".$program_version."\n";
  print "Written 2007 by Eddy L O Jansson <eddy\@klopper.net>\n";
  print "This is software is donated into the PUBLIC DOMAIN.\n";
  print "There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE, ";
  print "to the extent permitted by law.\n";
  exit 0;
}

sub output_help
{
  print "usage: $0 [OPTIONS] <infile >outfile\n\n";

  print "optional:\n";

  print " --list             Show available mappings.\n";
  print " --mapping CODE     Which mapping to use.\n";
  print "\n";
  print " --version          Print version number and exit.\n";
  print " --help             This help text and exit.\n";
  print "\n";
  print "Go to http://gazonk.org/~eloj/ for the latest version or bug reports.\n";
  exit 0;
}

sub output_mappings
{
  print "="x65; print "\n";
  print "Code - Name\n";
  print "="x65; print "\n";
  foreach my $m (keys %mappings)
  {
    print $m." - ".$mappings{$m}->{NAME}."\n";
  }
  exit 0;
}


#
# Remove quotes and convert periods to escape character.
#
sub fixup_grouptitle
{
  my $cfg = shift;
  my $param = shift;

  # Split into group and title on first period
  my @gt = split(/\./, $param, 2);

  if( $gt[1] ) {
    $gt[1] =~ s/\"//g;    # Remove quotes
    $gt[1] =~ s/\./$cfg->{ESCAPECHAR}/g;   # Escape periods
    return $gt[0].".".$gt[1];
  } else {
    return $param;
  }
}

#
# Just a real busy way of outputting all the headers, tab separated.
#
sub modern_header
{
  my $cfg = shift;
  my $param = shift;
  my $cols = keys(%columns);

  for(my $i=0 ; $i < $cols ; ++$i)
  {
    print $columns{$i}->{NAME};
    print $cfg->{SEPARATOR} if $i < $cols-1;
  }
  print "\n";
}

#############################################################################
#
# main
#
#############################################################################

my $code = $cfg{DEFAULT_MAPPING};

#
# Handle command line options
#
#  GetOptions(
#    'mapping=s' => \$code,
#    'list' => sub { output_mappings(); },
#    'version' => sub { output_version(); },
#    'help|?' => sub { output_help(); },
#  );

my $mapping = $mappings{$code};

if( ! defined $mapping->{HEADER_SUB} )
{
  print "Error: Mapping '".$code."' seems not be defined. Try --list.\n";
  exit 1;
}

#
# With everything set up, let's start the conversion:
#

# Output header
$mapping->{HEADER_SUB}(\%cfg, $mapping);

while( my $line = <STDIN> )
{
  chomp $line;
  my @fields = split(/$cfg{SEPARATOR}/o, $line);
  my @outline;

  # For each field, we check the conversion mapping to see if, what and
  # where to output it.
  for(my $i=0 ; $i < @fields ; ++$i)
  {
    if( defined $mapping->{COLMAP}->{$i}->{COLID} )
    {
      my $s = defined $mapping->{COLMAP}->{$i}->{TRANSFORM_SUB} ?
        $mapping->{COLMAP}->{$i}->{TRANSFORM_SUB}(\%cfg, $fields[$i]) : $fields[$i];
      $outline[ $mapping->{COLMAP}->{$i}->{COLID} ] = $s;
    }
  }
  print join($cfg{SEPARATOR}, @outline)."\n";
}

exit 0;
