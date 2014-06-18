#!/usr/bin/perl

use strict;
use warnings;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my $MAX_LEN = 60;
my $MIN_LEN = 1;
my $IDS;
GetOptions(
    "max_len=s" => \$MAX_LEN,
    "min_len=s" => \$MIN_LEN,
    "ids=s" => \$IDS,
);

if((@ARGV == 0) or (@ARGV % 2 != 0)) {
    print STDERR "Usage: $0 -max_len 60 OLD1 OLD2 ... NEW1 NEW2 ...\n";
    exit 1;
}

print "@ARGV\n";
# Open the files for reading and writing
my (@inhandles, @outhandles);
for(@ARGV[0 .. @ARGV/2-1]) {
    my $fh = IO::File->new("< $_") or die "Couldn't open file $_ for reading";
    binmode $fh, ":utf8";
    push @inhandles, $fh;
}
for(@ARGV[@ARGV/2 .. $#ARGV]) {
    (not -e $_) or die "File $_ already exists, delete it first.";
    my $fh = IO::File->new("> $_") or die "Couldn't open file $_ for writing";
    binmode $fh, ":utf8";
    push @outhandles, $fh;
}
open FILEIDS, ">:utf8", $IDS or die "Couldn't open $IDS\n" if $IDS;

my $id = 0;
while(1) {
    my @instrs = map { my $str = <$_>; $str } @inhandles;
    my $defcnt = sum(map { defined($_) ? 1 : 0 } @instrs);
    last if $defcnt == 0;
    die "Uneven number of lines" if $defcnt != @inhandles;
    my @cnts = map { chomp; my $cnt = split(/ +/); $cnt } @instrs;
    my $okcnt = sum(map { (($_ >= $MIN_LEN) and ($_ <= $MAX_LEN)) ? 1 : 0 } @cnts);
    $id++;
    if($okcnt == @inhandles) {
        for(0 .. $#instrs) {
            my $hand = $outhandles[$_];
            print $hand "$instrs[$_]\n";
        }
        print FILEIDS "$id\n" if($IDS);
    }
}
