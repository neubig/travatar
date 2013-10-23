#!/usr/bin/perl

use strict;
use utf8;
use FileHandle;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

if(@ARGV != 1) {
    print STDERR "Usage: $0 LENGTH\n";
    exit 1;
}

$| = 1;

while(<STDIN>) {
    chomp;
    my @arr = split(/ /);
    if(@arr > $ARGV[0]) {
        @arr = @arr[0 .. $ARGV[0]-1];
    }
    print "@arr\n";
}
