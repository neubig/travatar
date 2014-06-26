#!/usr/bin/perl

use strict;
use warnings;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my $NOBUF = 0;
GetOptions(
    "nobuf" => \$NOBUF,
);

if(@ARGV != 0) {
    print STDERR "Usage: $0 < input.txt > output.txt\n";
    exit 1;
}

if($NOBUF) {
    $| = 1;
}

while(<STDIN>) {
    $_ = lc($_);
    $_ =~ tr/Ａ-Ｚ/ａ-ｚ/;
    print $_;
}
