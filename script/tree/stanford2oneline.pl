#!/usr/bin/perl

use strict;
use utf8;
use List::Util qw(sum min max shuffle);
use Getopt::Long;
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

$| = 1;

$/ = "\n\n";
while(<STDIN>) {
    chomp;
    print join(" ", map { s/^ *//g; s/ $//g; $_ } split(/\n/))."\n";
}
