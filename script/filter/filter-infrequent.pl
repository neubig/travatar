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
my $MIN_LEN = 0;
GetOptions(
"threshold=s" => \$THRESHOLD,
"min-len=s" => \$MIN_LEN,
);

if(@ARGV != 0) {
    print STDERR "Usage: $0\n";
    exit 1;
}

while(<STDIN>) {
    chomp;
    my @arr = split(/ \|\|\| /);
    my @freq = split(/ /, $arr[3]);
    my @src = split(/ /, $arr[0]);
    my $bad = (($freq[0] <= $THRESHOLD) and ((not $MIN_LEN) or (@src <= $MIN_LEN)));
    $bad = 0 if (not $bad) or ($arr[0] =~ /^[^ ]+ \( "/);
    print "$_\n" if not $bad;
}
