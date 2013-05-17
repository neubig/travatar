#!/usr/bin/perl

use strict;
use warnings;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my $MAX_LEN = 60;
my $MIN_LEN = 1;
GetOptions(
    "max_len=s" => \$MAX_LEN,
    "min_len=s" => \$MIN_LEN,
);

if(@ARGV != 4) {
    print STDERR "Usage: $0 -max_len 60 OLD_SRC OLD_TRG NEW_SRC NEW_TRG\n";
    exit 1;
}

open FILE0, "<:utf8", $ARGV[0] or die "Couldn't open $ARGV[0]\n";
open FILE1, "<:utf8", $ARGV[1] or die "Couldn't open $ARGV[1]\n";
open FILE2, ">:utf8", $ARGV[2] or die "Couldn't open $ARGV[2]\n";
open FILE3, ">:utf8", $ARGV[3] or die "Couldn't open $ARGV[3]\n";

my ($f, $e);
while(1) {
    $f = <FILE0>; $e = <FILE1>;
    last if((not defined $f) and (not defined $e));
    die "Uneven number of lines" if((not defined $f) or (not defined $e));
    chomp $f; chomp $e;
    my @fa = split(/ +/, $f);
    my @ea = split(/ +/, $e);
    if((@fa >= $MIN_LEN) and (@fa <= $MAX_LEN) and (@ea >= $MIN_LEN) and (@ea <= $MAX_LEN)) {
        print FILE2 "$f\n";
        print FILE3 "$e\n";
    }
}
