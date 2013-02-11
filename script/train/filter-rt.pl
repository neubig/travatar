#!/usr/bin/perl

use strict;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my $SRC = "";
my $LEN = 7;
my $NTLEN = 5;
GetOptions(
    "src=s" => \$SRC,
    "len=i" => \$LEN,
    "ntlen=i" => \$NTLEN,
);

my %src;
if($SRC) {
    open FILE0, "<:utf8", $SRC or die "Couldn't open $SRC\n";
    while(<FILE0>) {
        chomp;
        while(/[^\(\)]+ ([^\(\)]+)/g) {
            # print STDERR "HERE: $1\n";
            $src{$1}++;
        }
    }
    close FILE0;
}

if(@ARGV != 0) {
    print STDERR "Usage: $0 < IN_TABLE > OUT_TABLE\n";
    exit 1;
}


my $line;
while($line = <STDIN>) {
    chomp $line;
    my @cols = split(/ \|\|\| /, $line);
    die "bad line $line" if @cols != 3;
    my ($term, $nonterm, $bad);
    for(split(/ /, $cols[0])) {
        if(/^"(.*)"$/) {
            $term++;
            if($SRC and (not exists $src{$1})) {
                $bad = 1;
                last;
            }
        } elsif(/^x\d*:/) {
            $nonterm++;
        }
    }
    $bad = 1 if $term > $LEN;
    $bad = 1 if $nonterm > $NTLEN;
    if(!$bad) { print "$line\n"; }
}
