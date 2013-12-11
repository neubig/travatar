#!/usr/bin/perl

use strict;
use warnings;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my $FIRST = 0; # First sentence to include
my $LAST = -1; # Last sentence to include (-1 for all)
my $REBASE = 0;  # The number of the first sentence in the output, given FIRST
GetOptions(
"first=s" => \$FIRST,
"last=s" => \$LAST,
"rebase=s" => \$REBASE,
);

if(@ARGV != 0) {
    print STDERR "Usage: $0\n";
    exit 1;
}

my $DIFF = $REBASE-$FIRST;
while(<STDIN>) {
    /^(\d+)( \|\|\| .*)$/ or die "Bad n-best line:\n$_";
    my ($id, $line) = ($1, $2);
    next if $id < $FIRST or (($LAST >= 0) and ($id > $LAST));
    print "".($id+$DIFF).$line."\n";
}
