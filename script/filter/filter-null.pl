#!/usr/bin/perl

# A script to remove phrases that delete content words
# from the phrase table

use strict;
use utf8;
use FileHandle;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my $SHOW_FILTERED = 0;
GetOptions(
"show-filtered" => \$SHOW_FILTERED,
);

if(@ARGV != 0) {
    print STDERR "Usage: $0\n";
    exit 1;
}

sub can_be_null {
    return $_[0] =~ /^(pn|prp|dt|助|語尾)/i;
}

while(<STDIN>) {
    chomp;
    my @arr = split(/ \|\|\| /);
    my $ok = 1;
    if($arr[1] !~ /"/) {
        while($ok and ($arr[0] =~ /([^ ]+) \( "/g)) {
            $ok = can_be_null($1);
        }
    }
    print "$_\n" if 
        ($ok and (not $SHOW_FILTERED)) or 
        ((not $ok) and $SHOW_FILTERED);
}
