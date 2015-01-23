#!/usr/bin/perl

use strict;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my $SIDE = "src";
GetOptions(
"side=s" => \$SIDE,
);

if(@ARGV != 0) {
    print STDERR "Usage: $0\n";
    exit 1;
}

sub make_src {
    my $id = shift;
    ($_ = <STDIN>) or exit 0;
    chomp;
    my ($sent, $span, $src, $trg, $feat) = split(/ \|\|\| /);
    return "" if not $src;
    $src =~ s/ @ .*//g;
    $trg =~ s/ @ .*//g;
    my @arr = split(/ +/, $src);
    if($arr[0] ne $id) { "Die: $id doesn't match\n$_"; }
    my $ret;
    for(my $pos = 0; $pos < @arr; $pos++) {
        my $sym = $arr[$pos];
        # Wildcard
        if($sym =~ /x\d+:(.*)/) {
            $ret .= make_src($1);
        # Terminal
        } elsif ($sym =~ /^"(.+)"$/) {
            $ret .= $1;
        # Parens
        } elsif ($sym =~ /^[\(\)]$/) {
            $ret .= $sym;
        } elsif ($sym =~ /UNK/) {
            $ret .= make_src($1);
        } else {
            $arr[++$pos] eq "(" or die "No expected paren after $sym: $_\n";
            $ret .= "($sym ";
        }
    }
    return $ret;
}

sub make_trg {
    ($_ = <STDIN>) or exit 0;
    chomp;
    my ($sent, $span, $src, $trg, $feat) = split(/ \|\|\| /);
    return "" if not $src;
    my @arr = split(/ +/, $trg);
    my $ret;
    my (@retarr, @unfilled);
    for(my $pos = 0; $pos < @arr; $pos++) {
        my $sym = $arr[$pos];
        # Wildcard
        if($sym =~ /^x(\d+)$/) {
            $unfilled[$1] = scalar(@retarr);
            push @retarr, "FILLER";
        # Terminal
        } elsif ($sym =~ /^"(.+)"$/) {
            push @retarr, "(X $1)";
        } else {
            die "Bad symbol: $sym\n$_";
        }
    }
    foreach my $i (@unfilled) {
        @retarr[$i] = make_trg();
    }
    return "(X ".join("", @retarr).")";
}

while(1) {
    my $parse;
    if($SIDE eq "src") {
        $parse = make_src("root");
    } elsif($SIDE eq "trg") {
        $parse = make_trg();
    } else {
        die "Bad -side $SIDE";
    }
    print "$parse\n" if($parse);
}
