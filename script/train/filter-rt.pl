#!/usr/bin/perl

use strict;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my $SRC = "";
my $SRC_FORMAT = "penn";
my $LEN = 0;
my $NTLEN = 0;
GetOptions(
    "src=s" => \$SRC,
    "src-format=s" => \$SRC_FORMAT,
    "len=i" => \$LEN,
    "ntlen=i" => \$NTLEN,
);

my %src;
if($SRC) {
    open FILE0, "<:utf8", $SRC or die "Couldn't open $SRC\n";
    if($SRC_FORMAT eq "penn") {
        while(<FILE0>) {
            chomp;
            while(/[^\(\)]+ ([^\(\)]+)/g) {
                # print STDERR "HERE: $1\n";
                $src{$1}++;
            }
        }
    } elsif ($SRC_FORMAT eq "egret") {
        while(<FILE0>) {
            if(/^sentence /) {
                $_ = <FILE0>;
                chomp;
                for(split(/ +/)) { $src{$_}++; }
            }
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
    die "Wrong number of columns in $line" if @cols < 3;
    my ($term, $nonterm, $bad);
    while($cols[0] =~ / "([^ ]+)" /g) {
        $term++;
        if($SRC and (not exists $src{$1})) {
            $bad = 1;
            last;
        }
    }
    $bad = 1 if ($LEN and $term > $LEN);
    if(!$bad and $NTLEN) {
        while($cols[0] =~ / x[0-9]+:/g) {
            $nonterm++;
        }
        $bad = 1 if $nonterm > $NTLEN;
    }
    if(!$bad) { print "$line\n"; }
}
