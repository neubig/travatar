#!/usr/bin/perl

use strict;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

GetOptions(
);

if(@ARGV != 0) {
    print STDERR "Usage: $0 < INPUT > OUTPUT\n";
    exit 1;
}

sub print_counts {
    my $id = shift;
    my $counts = shift;
    my $sum = sum(values %$counts);
    for(sort keys %$counts) {
        printf "$id ||| $_ ||| %f 2.718\n", $counts->{$_}/$sum;
    }
}

my (%counts, $curr_id);
while(<STDIN>) {
    chomp;
    my @arr = split(/ \|\|\| /);
    if($arr[0] ne $curr_id) {
        print_counts($curr_id, \%counts);
        %counts = ();
        $curr_id = $arr[0];
    }
    $counts{$arr[1]} += $arr[2];
}
print_counts($curr_id, \%counts);
