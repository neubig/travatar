#!/usr/bin/perl

use strict;
use utf8;
use FileHandle;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my $THRESHOLD = 1;

if(@ARGV != 0) {
    print STDERR "Usage: $0\n";
    exit 1;
}

while(<STDIN>) {
    chomp;
    my @arr = split(/ \|\|\| /);
    my @freq = split(/ /, $arr[3]);
    my $bad = ($freq[0] <= $THRESHOLD);
    $bad = 0 if (not $bad) or ($arr[0] =~ /^[^ ]+ \( "/);
    print "$_\n" if not $bad;
}
