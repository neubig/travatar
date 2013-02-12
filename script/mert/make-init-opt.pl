#!/usr/bin/perl

use strict;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

if(@ARGV != 0) {
    print STDERR "Usage: $0\n";
    exit 1;
}

while(<STDIN>) {
    chomp;
    last if $_ eq "[weight_vals]";
}

my @arr;
while(<STDIN>) {
    chomp;
    last if not $_;
    s/.*=//g;
    push @arr, $_;
}
my $s = sum(map { abs($_) } @arr);
print join(" ", map { $_/$s } @arr)."\n";
print join(" ", map { 0 } @arr)."\n";
print join(" ", map { 1 } @arr)."\n";
