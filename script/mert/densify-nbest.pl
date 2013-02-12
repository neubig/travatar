#!/usr/bin/perl

use strict;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

if(@ARGV != 1) {
    print STDERR "Usage: $0 SEED_WEIGHTS < NBEST > DENSIFIED_NBEST\n";
    exit 1;
}

my %poss;
open FILE0, "<:utf8", $ARGV[0] or die "Couldn't open $ARGV[0]\n";
while(<FILE0>) {
    chomp;
    last if $_ eq "[weight_vals]";
}
while(<FILE0>) {
    chomp;
    last if not $_;
    s/=.*//g;
    $poss{$_} = scalar(keys %poss);
}
close FILE0;

while(<STDIN>) {
    chomp;
    my @col = split(/ \|\|\| /);
    @col >= 3 or die "Bad line $_";
    my @arr = map { 0 } keys %poss;
    for(split(/ /, $col[3])) {
        my ($k, $v) = split(/=/);
        die "bad line k==$k, v==$v" if not exists $poss{$k};
        $arr[$poss{$k}] = $v;
    }
    $col[3] = join(" ", @arr);
    print join(" ||| ", $col[0], $col[1], $col[3])."\n";
}
