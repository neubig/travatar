#!/usr/bin/perl

use strict;
use warnings;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

if(@ARGV != 5) {
    print STDERR "Usage: $0 FTXT ETXT ALIGN F_GIVEN_E E_GIVEN_F\n";
    exit 1;
}

open FTXT, "<:utf8", $ARGV[0] or die "Couldn't open $ARGV[0]\n";
open ETXT, "<:utf8", $ARGV[1] or die "Couldn't open $ARGV[1]\n";
open ALIGN, "<:utf8", $ARGV[2] or die "Couldn't open $ARGV[2]\n";

my (%fc, %ec, %fec);
my ($f, $e, $a);
while(1) {
    $f = <FTXT>; $e = <ETXT>; $a = <ALIGN>;
    last if((not defined $f) and (not defined $e) and (not defined $a));
    die "File lengths don't match\n$f\n$e\n$a\n"
        if((not defined $f) or (not defined $e) or (not defined $a));
    chomp $f; $f =~ s/^ +//g; $f =~ s/ +$//g;
    chomp $e; $e =~ s/^ +//g; $e =~ s/ +$//g;
    chomp $a; $a =~ s/^ +//g; $a =~ s/ +$//g;
    my @fa=split(/ +/, $f); my @ea=split(/ +/, $e); my @aa=split(/ +/, $a);
    my (@fcov, @ecov);
    # Add the null alignment
    $fc{"NULL"}++; $ec{"NULL"}++;
    # Enumerate all the alignments
    for(@aa) {
        my ($j, $i) = split(/-/);
        die "Bad alignment\n$f\n$e\n$a\n" if(($j >= @fa) or ($i >= @ea));
        $fcov[$j]++; $ecov[$i]++;
        $fec{"$fa[$j] $ea[$i]"}++;
        $fc{"$fa[$j]"}++; $ec{"$ea[$i]"}++;
    }
    # Cover the uncovered alignments with nulls
    foreach my $i (0 .. $#ea) {
        if(not $ecov[$i]) {
            $fec{"NULL $ea[$i]"}++; $fc{"NULL"}++;
        }
    }
    foreach my $j (0 .. $#fa) {
        if(not $fcov[$j]) {
            $fec{"$fa[$j] NULL"}++; $ec{"NULL"}++;
        }
    }
}
close FTXT;
close ETXT;
close ALIGN;

# Print the alignments
open FGIVENE, ">:utf8", $ARGV[3] or die "Couldn't open $ARGV[3]\n";
open EGIVENF, ">:utf8", $ARGV[4] or die "Couldn't open $ARGV[4]\n";
while(my($k,$v) = each(%fec)) {
    my ($f, $e) = split(/ +/, $k);
    print FGIVENE "$f $e ".($v/$ec{$e})."\n" if $f ne "NULL";
    print EGIVENF "$e $f ".($v/$fc{$f})."\n" if $e ne "NULL";
}
close FGIVENE;
close EGIVENF;
