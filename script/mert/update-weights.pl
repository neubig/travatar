#!/usr/bin/perl

use strict;
use warnings;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my $LOG = "";
my $MODEL = "";
my $WEIGHTS = "";
GetOptions(
    "log=s" => \$LOG,
    "model=s" => \$MODEL,
    "weights=s" => \$WEIGHTS,
);

if(@ARGV != 1) {
    print STDERR "Usage: $0 [-log MERT_LOG] [-model TRAVATAR_MODEL] INPUT > OUTPUT\n";
    exit 1;
}

my (@weights, @names, %wmap);
if($LOG) {
    open FILE1, "<:utf8", $LOG or die "Couldn't open $LOG\n";
    while(<FILE1>) {
        chomp;
        if(/Best point: (.*)  =>/) {
            @weights = split(/ /, $1);
            last;
        }
    }
    close FILE1;
} elsif($MODEL) {
    open FILE1, "<:utf8", $MODEL or die "Couldn't open $MODEL\n";
    while(<FILE1>) {
        chomp;
        last if $_ eq "[weight_vals]";
    }
    while(<FILE1>) {
        chomp;
        last if not $_;
        my ($name, $weight) = split(/=/);
        push @names, $name;
        push @weights, $weight;
    }
    close FILE1;
} elsif($WEIGHTS) {
    open FILE1, "<:utf8", $WEIGHTS or die "Couldn't open $WEIGHTS\n";
    while(<FILE1>) {
        chomp;
        for(split(/ /)) { my ($k,$v) = split(/=/); $wmap{$k} = $v; }
    }
    close FILE1;
} else {
    die "Must specify either -log or -model";
}

open FILE0, "<:utf8", $ARGV[0] or die "Couldn't open $ARGV[0]\n";
# Print everything until the weights
while(<FILE0>) {
    chomp;
    print "$_\n";
    last if $_ eq "[weight_vals]";
}
# Update the weights
while(<FILE0>) {
    chomp;
    last if not $_;
    my ($oldname, $oldweight) = split(/=/);
    my $newweight;
    if(!%wmap) {
        die "Weight sizes don't match" if not @weights;
        my $newname = shift(@names);
        $newweight = shift(@weights);
        die "newname ($newname) and oldname ($oldname) don't match" if($newname and ($newname ne $oldname));
        print "$oldname=$newweight\n";
    }
}
if(%wmap) {
    while(my ($k,$v) = each(%wmap)) {
        print "$k=$v\n";
    }
} else {
    die "Weight sizes don't match" if @weights;
}
print "\n";
# Print the rest of the file
while(<FILE0>) {
    print $_;
}
