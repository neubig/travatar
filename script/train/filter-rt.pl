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
my $LEN = 1e6;
my $NTLEN = 1e6;
my $MAX_RULE_LEN = 1e6;
GetOptions(
    "src=s" => \$SRC,
    "src-format=s" => \$SRC_FORMAT,
    "len=i" => \$LEN,
    "ntlen=i" => \$NTLEN,
    "max-rule-len=s" => \$MAX_RULE_LEN
);

if(@ARGV != 0) {
    print STDERR "Usage: $0 < IN_TABLE > OUT_TABLE\n";
    exit 1;
}

my %src;
if($SRC) {
    open FILE0, "<:utf8", $SRC or die "Couldn't open $SRC\n";
    while(<FILE0>) {
        my @sent;
        if($SRC_FORMAT eq "penn") {
            chomp;
            while(/[^\(\)]+ ([^\(\)]+)/g) {
                push @sent, $1;
            }
        } elsif ($SRC_FORMAT eq "egret") {
            next if($_ !~ /^sentence /);
            $_ = <FILE0>;
            chomp;
            @sent = split(/ +/);
        } elsif ($SRC_FORMAT eq "word") {
            @sent = split(/ +/);
        } else {
            die "Illegal source format: $SRC_FORMAT";
        }
        foreach my $i (0 .. $#sent) {
            foreach my $j ($i .. min($#sent, $i+$MAX_RULE_LEN-1)) {
                $src{join(" ", @sent[$i .. $j])}++;
            }
        }
    }
    close FILE0;
}

my $line;
while($line = <STDIN>) {
    chomp $line;
    my @cols = split(/ \|\|\| /, $line);
    die "Wrong number of columns in $line" if @cols < 2;
    if(@cols == 2) { push @cols, ""; }
    my ($term, $nonterm, $bad, @currsrc);
    while($cols[0] =~ / ("([^ ]+)"|x\d+:[^ ]+) /g) {
        my $myid = $2;
        if($myid) {
            $term++;
            push @currsrc, $myid;
        } else {
            $nonterm++;
            if(@currsrc and not exists $src{"@currsrc"}) {
                $bad = 1;
                last;
            }
            @currsrc = ();
        }
    }
    $bad = 1 if ($bad or ($term > $LEN) or ($nonterm > $NTLEN) or (@currsrc and not exists $src{"@currsrc"}));
    if(!$bad) { print "$line\n"; }
}
