#!/usr/bin/perl

use strict;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my $TOP_N = 10;
my $SRC_MIN_FREQ = 0;
my $LEX_PROB_FILE = "";
my $TRG_SYNTAX = 0;
my $SRC_LABEL = 0;
my $TRG_LABEL = 0;
my $SRC_TRG_LABEL = 0;
my $PREFIX = "egf";
GetOptions(
    "top-n=i" => \$TOP_N,                 # Only extract the top N patterns
    "src-min-freq=i" => \$SRC_MIN_FREQ,   # Minimum frequency of a src pattern
    "lex-prob-file=s" => \$LEX_PROB_FILE, # File of lexical probabilities for
                                          # calculating model 1
    "prefix=s" => \$PREFIX,               # Prefix for model 1
    "trg-syntax" => \$TRG_SYNTAX,         # Use target side syntax
    "src-label" => \$SRC_LABEL,           # Calculate sparse features for the source labels
    "trg-label" => \$TRG_LABEL,           # Calculate sparse features for the target labels
    "src-trg-label" => \$SRC_TRG_LABEL,   # Calculate sparse features for the source/target labels
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
        my @arr = split(/[\t ]/);
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
    my $isstring = shift;
    my @ret;
    my @arr = split(/ +/, $str);
    for(@arr) {
        return @ret if($isstring and ($_ eq "@")); # Skip syntactic labels
        # Check if there are quotes and remove them
        # Doing this with substrings is uglier than a regex but faster
        if((substr($_,0,1) eq "\"") and (substr($_,-1) eq "\"")) {
            push @ret, substr($_, 1, -1);
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
        $ret += ($prob ? log($prob/(@$earr+1)) : -99);
    }
    return $ret;
}

my $SYNTAX_FEATS = ($TRG_SYNTAX or $SRC_LABEL or $TRG_LABEL or $SRC_TRG_LABEL);
sub print_counts {
    my $e = shift;
    my $counts = shift;
    my $ist2s = ($e =~ /^[^ ]+ \( /); # Check if this is a string
    my @earr = strip_arr($e, !$ist2s);
    my $sum = sum(values %$counts);
    return if $sum < $SRC_MIN_FREQ;
    my @keys = keys %$counts;
    if($TOP_N and (@keys > $TOP_N)) {
        @keys = sort { $counts->{$b} <=> $counts->{$a} } @keys;
        @keys = @keys[0 .. $TOP_N-1];
    }
    foreach my $f (sort @keys) {
        my $words = 0;
        my @farr = strip_arr($f, $ist2s);
        # If we are using target side syntax and the rule is bad
        my $extra_feat;
        if($SYNTAX_FEATS) {
            $e =~ /^([^ ]+) /;
            my $src_lab = $1;
            $f =~ / @ ([^ ]+)/;
            my $trg_lab = $1;
            $extra_feat .= " isx=1" if($TRG_SYNTAX and $trg_lab eq "\@X\@");
            $extra_feat .= " sl_${src_lab}=1" if $SRC_LABEL and $src_lab;
            $extra_feat .= " tl_${trg_lab}=1" if $TRG_LABEL and $trg_lab;
            $extra_feat .= " stl_${src_lab}_${trg_lab}=1" if $SRC_TRG_LABEL and $src_lab and $trg_lab;
        }
        # Find the counts/probabilities
        my $lfreq = ($counts->{$f} ? log($counts->{$f}) : 0);
        my $lprob = ($counts->{$f} ? log($counts->{$f}/$sum) : -99);
        printf "$e ||| $f ||| p=1 lfreq=%f ${PREFIX}p=%f$extra_feat", $lfreq, $lprob;
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
