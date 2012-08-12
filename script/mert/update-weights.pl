#!/usr/bin/perl

use strict;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

if(@ARGV != 2) {
    print STDERR "Usage: $0 WEIGHTS MERT_LOG > NEW_WEIGHTS\n";
    exit 1;
}

open FILE0, "<:utf8", $ARGV[0] or die "Couldn't open $ARGV[0]\n";
my @names = map { chomp; s/=.*//g; $_ } <FILE0>;
close FILE0;

my @weights;
open FILE1, "<:utf8", $ARGV[1] or die "Couldn't open $ARGV[1]\n";
while(<FILE1>) {
    chomp;
    if(/Best point: (.*)  =>/) {
        @weights = split(/ /, $1);
        last;
    }
    
}
close FILE1;

die "Mismatched arrays\n@names\n@weights\n" if(@names != @weights);
for(0 .. $#names) {
    print "$names[$_]=$weights[$_]\n";
}
