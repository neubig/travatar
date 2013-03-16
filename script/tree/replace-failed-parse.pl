#!/usr/bin/perl

use strict;
use warnings;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my $ARG = "";
my $NOBUF = "";
GetOptions(
    "arg=s" => \$ARG,
    "nobuf!" => \$NOBUF,
);

if(@ARGV != 2) {
    print STDERR "Usage: $0 complete_parses.txt possibly_failed_parses.txt\n";
    exit 1;
}

open FILE0, "<:utf8", $ARGV[0] or die "Couldn't open $ARGV[0]\n";
open FILE1, "<:utf8", $ARGV[1] or die "Couldn't open $ARGV[1]\n";

$| = 1 if $NOBUF;

my ($s0, $s1);
while(($s0 = <FILE0>) and ($s1 = <FILE1>)) {
    chomp $s0; chomp $s1;
    # If s1 is failed, print $s0
    if(($s1 eq "") or ($s1 =~ /^\(\(\)\)$/)) {
        print "$s0\n";
    } else {
        print "$s1\n";
    }
}
