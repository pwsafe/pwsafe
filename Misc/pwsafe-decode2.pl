#!/usr/bin/perl

# Updated April 5, 2005 by Jason Diamond.

# This is an updated version of the original script written by Paul Pluzhnikov.
# This version knows how to parse the different record types introduced in the
# Version 2 File Format.  It doesn't do any checking to see if the specified
# file actually is a Version 2 file--it just assumes that it is.

use Crypt::Blowfish;
use Digest::SHA1;
use Getopt::Std;

$verify = 0;

getopts("c"); # '-c' => produce CSV file

if ($#ARGV < 1) {
	die "usage: $0 [-c] password file.dat\n";
}

$pass = $ARGV[0];
open(IN, $ARGV[1]) || die "unable to open $AGV[1]: $!";

sysread(IN, $rnd, 8);
sysread(IN, $rndhash, 20);

if ($verify) {
	# verify password is correct; doesn't work: see below
	$sha = new Digest::SHA1;
	$sha->add($rnd, pack("xx"), $pass);

	$salt = $sha->digest();

	# perl does Blowfish in Network byte order, but pwsafe does it in Host
	# hence conversion back and forth
	$rnd = pack("N2", unpack("L2", $rnd));

	$cypher = new Crypt::Blowfish $salt;
	for (0 .. 999) {
		$rnd = $cypher->encrypt($rnd);
	}
	$rnd = pack("L2", unpack("N2", $rnd));

	$sha->add($rnd, pack("xx"));

	# $sha->digest() ought to match $rndhash, but doesn't
	# due to the fact that SHA1Final() doesn't reset to the same state as SHA1Init().
	# print $sha->hexdigest(), "\n";
}

sysread(IN, $salt, 20);
sysread(IN, $ip, 8);

$sha = new Digest::SHA1;
$sha->add($pass, $salt);
$cypher = new Crypt::Blowfish($sha->digest());

# This hash is here for debugging purposes.
%recordTypes = (
	0 => "None",
	1 => "UUID",
	2 => "Group",
	3 => "Title",
	4 => "Username",
	5 => "Notes",
	6 => "Password",
	7 => "Creation Time",
	8 => "Password Modification Time",
	9 => "Last Access Time",
	10 => "Password Lifetime",
	11 => "Password Policy",
	12 => "Last Mod. Time",
	255 => "End of Entry"
);

while (sysread(IN, $len, 8)) {
	$ipNext = $len;
	$len = pack("N2", unpack("L2", $len));
	$len = $cypher->decrypt($len);
	$len = pack("N2", unpack("L2", $len));
	($len, $rt) = unpack("L2", $len ^ $ip);
	$ip = $ipNext;

	#print "length of record is $len\n";
	#print "record type is $recordTypes{$rt}\n";

	if ($len > 1000) {
		die "the password was probably incorrect";
	}

	if ($len == 0) { $len = 8; }
	$str = "";
	while (0 < $len)
	{
		sysread(IN, $d, 8); $ipNext = $d;
		$d = pack("N2", unpack("L2", $d));
		$d = $cypher->decrypt($d);
		$d = pack("N2", unpack("L2", $d));
		$d ^= $ip;
		$str .= unpack("A*", $d);
		$ip = $ipNext;
		$len -= 8;
	}

	# Skip the first three records.
	$rc += 1;
	if ($rc < 4) { next; }

	# Format the data based on the record type.
	if ($rt == 1) {
		@uuid = unpack("C16", $str);
		$str = sprintf "%02X" x 4 . "-" . ("%02X" x 2 . "-") x 3 . "%02X" x 6, @uuid;
	} elsif ($rt > 6) {
		$str = "<not supported>";
	}

	if ($opt_c) {
		# Output the CSV data. This depends on all entries having the
		# same records in the same order.
		if ($rt != 255) {
			$str =~ s/,/\\,/g;
			$str =~ s/\r*\n/\\n/g;
			print "$str,";
		} else {
			print "\n";
		}
	} else {
		# Output the non-CSV data. One record per line.
		if ($rt != 255) {
			print "$recordTypes{$rt}: $str\n";
		} else {
			print "=" x 60, "\n";
		}
	}
}

exit(0);

