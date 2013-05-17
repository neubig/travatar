#!/usr/bin/perl

use strict;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

if(@ARGV != 0) {
    print STDERR "Usage: $0\n";
    exit 1;
}

sub make_child {
    my $id = shift;
    ($_ = <STDIN>) or exit 0;
    chomp;
    my ($sent, $span, $src, $trg, $feat) = split(/ \|\|\| /);
    return "" if not $src;
    my @arr = split(/ +/, $src);
    if($arr[0] ne $id) { "Die: $id doesn't match\n$_"; }
    my $ret;
    for(my $pos = 0; $pos < @arr; $pos++) {
        my $sym = $arr[$pos];
        # Wildcard
        if($sym =~ /x\d+:(.*)/) {
            $ret .= make_child($1);
        # Terminal
        } elsif ($sym =~ /^"(.+)"$/) {
            $ret .= $1;
        # Parens
        } elsif ($sym =~ /^[\(\)]$/) {
            $ret .= $sym;
        } elsif ($sym =~ /UNK/) {
            $ret .= make_child($1);
        } else {
            $arr[++$pos] eq "(" or die "No expected paren after $sym: $_\n";
            $ret .= "($sym ";
        }
    }
    return $ret;
}

while(1) {
    my $parse = make_child("root");
    print "$parse\n" if($parse);
}
