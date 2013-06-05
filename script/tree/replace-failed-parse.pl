#!/usr/bin/perl

use strict;
use warnings;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my $FORMAT = "penn";
my $NOBUF = "";
my $ROOT = "ROOT";
GetOptions(
    "format=s" => \$FORMAT,
    "nobuf!" => \$NOBUF,
    "root=s" => \$ROOT,
);

if(@ARGV != 2) {
    print STDERR "Usage: $0 complete_parses.txt possibly_failed_parses.txt\n";
    exit 1;
}

open FILE0, "<:utf8", $ARGV[0] or die "Couldn't open $ARGV[0]\n";
open FILE1, "<:utf8", $ARGV[1] or die "Couldn't open $ARGV[1]\n";

$| = 1 if $NOBUF;


if($FORMAT eq "penn") {
    my ($s0, $s1);
    while(defined($s0 = <FILE0>) and defined($s1 = <FILE1>)) {
        chomp $s0; chomp $s1;
        # If s1 is failed, print $s0
        if(($s1 eq "") or ($s1 =~ /^\(\(\)\)$/)) {
            $s0 =~ s/^\( /($ROOT /g;
            print "$s0\n";
        } else {
            $s1 =~ s/^\( /($ROOT /g;
            print "$s1\n";
        }
    }
} elsif ($FORMAT eq "egret") {
    while(1) {
        my $correct = &get_egret(\*FILE0);
        my $sketchy = &get_egret(\*FILE1);
        die "Found sentence in suspicious file, but not correcft: $sketchy\n" if $sketchy and not $correct;
        last if not $correct;
        print ($sketchy ? $sketchy : $correct);
    }
} else {
    die "Bad tree format $FORMAT\n";
}

sub get_egret {
    my $handle = shift;
    my ($ret, $buf);
    if(defined($buf = <$handle>)) {
        $buf =~ /^sentence/ or die "Bad forest header: $buf";
        $ret .= $buf;
    } else {
        return undef;
    }
    defined($buf = <$handle>) or die "Stopped in the middle of a forest";
    $ret .= $buf;
    defined($buf = <$handle>) or die "Stopped in the middle of a forest";
    $ret .= $buf;
    chomp $buf;
    # Failed parses get no tree!
    if(not $buf) {
        $buf = <$handle>;
        return undef;
    }
    while($buf = <$handle>) {
        $ret .= $buf;
        chomp $buf;
        last if not $buf;
    }
    return $ret;
}
