#!/usr/bin/perl

# This is a script to reduce the features of a translation model. The easiest way to use it is in combination
# with the filter-model.pl script as follows:
#
# $ filter-model.pl \
#     -set-features rule=1 travatar.ini travatar-out.ini travatar-out "reduce-features.pl travatar.ini"
#
# Where
# * travatar.ini:       is the input config file
# * travatar-out.ini:   is the output config file
# * travatar-out:       is the directory in which the model is placed

use strict;
use warnings;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

if(@ARGV != 1) {
    print STDERR "Usage: $0\n";
    exit 1;
}

open FILE0, "<:utf8", $ARGV[0] or die "Couldn't open $ARGV[0]\n";
my %weights;
my $done = 0;
while(<FILE0>) {
    chomp;
    if(/^\[weight_vals\]$/) {
        while(<FILE0>) {
            chomp;
            last if not $_;
            die "Bad line $_" if not /^(.+)=([^=]+)$/;
            $weights{$1} = $2;
        }
        last;
    }
}
close FILE0;

while(<STDIN>) {
    chomp;
    my @cols = split(/ \|\|\| /);
    my $val = 0;
    foreach my $str (split(/ /, $cols[2])) {
        die "Bad line $_" if not ($str =~ /^(.+)=([^=]+)$/);
        $val += $weights{$1} * $2;
    }
    $cols[2] = sprintf("rule=%.6f", $val);
    print "$cols[0] ||| $cols[1] ||| $cols[2]\n";
}
