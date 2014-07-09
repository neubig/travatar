#!/usr/bin/perl

use strict;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

if(@ARGV != 0) {
    print STDERR "Usage: $0 < INPUT > OUTPUT\n";
    exit 1;
}

while(<STDIN>) {
    chomp;
    my @arr = split(/ \|\|\| /, $_, -1);
    die "Wrong number of columns (must >= 4):\n$_" if (@arr < 4);
    my $tmp = $arr[0]; $arr[0] = $arr[1]; $arr[1] = $tmp;
    my $align = pop @arr;
    my @out;
    while($align =~ /(\d+)-(\d+)/g) { 
        push @out, "$2-$1";
    }
    print join(" ||| ", @arr, "@out")."\n";
}
