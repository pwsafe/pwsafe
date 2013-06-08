#!/usr/bin/perl

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

while (sysread(IN, $len, 8)) {
    if ($rc % 3 == 0 && ! $opt_c) {
	print "=" x 60, "\n";
    }
    $rc += 1; # record counter
    $ipNext = $len;
    $len = pack("N2", unpack("L2", $len));
    $len = $cypher->decrypt($len);
    $len = pack("N2", unpack("L2", $len));
    $len = unpack("L2", $len ^ $ip);
    $ip = $ipNext;

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
    if ($opt_c) {
	$str =~ s/,/\\,/g;
	$str =~ s/\r*\n/\\n/g;
	print $str;
	print $rc % 3 ? "," : "\n";
    } else {
	print $str, "\n";
    }
}
exit(0);
