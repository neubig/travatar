#!/usr/bin/perl

use strict;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my $TOP_N = 10;
my $SRC_MIN_FREQ = 2;
my $LEX_PROB_FILE = "";
my $PREFIX = "egf";
GetOptions(
    "top-n=i" => \$TOP_N,                 # Only extract the top N patterns
    "src-min-freq=i" => \$SRC_MIN_FREQ,   # Minimum frequency of a src pattern
    "lex-prob-file=s" => \$LEX_PROB_FILE, # File of lexical probabilities for
                                          # calculating model 1
    "prefix=s" => \$PREFIX,               # Prefix for model 1
);

if(@ARGV != 0) {
    print STDERR "Usage: $0 < INPUT > OUTPUT\n";
    exit 1;
}

my %lex;
if($LEX_PROB_FILE) {
    print STDERR "Loading from $LEX_PROB_FILE\n";
    open FILE, "<:utf8", $LEX_PROB_FILE or die "Couldn't open $LEX_PROB_FILE\n";
    while(<FILE>) {
        chomp;
        my @arr = split(/\t/);
        if(@arr != 3) {
            print STDERR "Bad line $_\n";
            exit 1;
        }
        $lex{"$arr[0]\t$arr[1]"} = $arr[2];
    }
    close FILE;
    print STDERR "Done loading lexical probabilities\n";
}

sub strip_arr {
    my $str = shift;
    my @ret;
    for(split(/ /, $str)) {
        if(/^"(.*)"$/) {
            push @ret, $1;
        }
    }
    return @ret;
}

# Calculate the m1 probability of f given e
my $min_prob = 1e-7;
sub m1prob {
    my ($earr, $farr) = @_;
    my $ret = 0;
    foreach my $f (@$farr) {
        my $prob = 0;
        foreach my $e (@$earr, "") {
            $prob += max($min_prob, $lex{"$f\t$e"});
        }
        $ret += log($prob/(@$earr+1));
    }
    return $ret;
}

sub print_counts {
    my $e = shift;
    my $counts = shift;
    my $sum = sum(values %$counts);
    return if $sum < $SRC_MIN_FREQ;
    my @keys = keys %$counts;
    if($TOP_N and (@keys > $TOP_N)) {
        @keys = sort { $counts->{$b} <=> $counts->{$a} } @keys;
        @keys = @keys[0 .. $TOP_N-1];
    }
    foreach my $f (sort @keys) {
        my $words = 0;
        my @earr = strip_arr($e);
        my @farr = strip_arr($f);
        printf "$e ||| $f ||| p=1 lfreq=%f ${PREFIX}p=%f", log($counts->{$f}), log($counts->{$f}/$sum);
        printf " ${PREFIX}l=%f", m1prob(\@earr, \@farr) if $LEX_PROB_FILE;
        print " w=".scalar(@farr) if (@farr);
        print "\n";
    }
}

my (%counts, $curr_id);
while(<STDIN>) {
    chomp;
    my @arr = split(/ \|\|\| /);
    if($arr[0] ne $curr_id) {
        print_counts($curr_id, \%counts);
        %counts = ();
        $curr_id = $arr[0];
    }
    $counts{$arr[1]} += $arr[2];
}
print_counts($curr_id, \%counts);
