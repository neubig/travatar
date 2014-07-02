#!/usr/bin/perl

use strict;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my $SRC_MIN_FREQ = 0;
my $LEX_PROB_FILE = "";
my $TRG_SYNTAX = 0;
my $SRC_LABEL = 0;
my $TRG_LABEL = 0;
my $SRC_TRG_LABEL = 0;
my $PREFIX = "";
my $JOINT = 0;
my $COND_PREFIX = "egf";
my $FOF_MAX = 20;
my $KEEP_EMPTY = 0;
my $FOF_FILE;
GetOptions(
    "src-min-freq=i" => \$SRC_MIN_FREQ,   # Minimum frequency of a src pattern
    "lex-prob-file=s" => \$LEX_PROB_FILE, # File of lexical probabilities for
                                          # calculating model 1
    "cond-prefix=s" => \$COND_PREFIX,     # Prefix for model 1
    "prefix=s" => \$PREFIX,               # Prefix for all features
    "joint" => \$JOINT,                   # word/phrase/lfreq features or not
    "trg-syntax" => \$TRG_SYNTAX,         # Use target side syntax
    "src-label" => \$SRC_LABEL,           # Calculate sparse features for the source labels
    "trg-label" => \$TRG_LABEL,           # Calculate sparse features for the target labels
    "src-trg-label" => \$SRC_TRG_LABEL,   # Calculate sparse features for the source/target labels
    "fof-file=s" => \$FOF_FILE,           # Save frequencies of frequencies to a file
    "keep-empty" => \$KEEP_EMPTY,         # Keep rules that have an empty string on one side
);

if(@ARGV != 0) {
    print STDERR "Usage: $0 < INPUT > OUTPUT\n";
    exit 1;
}

my %lex;
if($LEX_PROB_FILE) {
    print STDERR "Loading from $LEX_PROB_FILE\n";
    open FILE, "<:utf8", $LEX_PROB_FILE or die "Couldn't open $LEX_PROB_FILE $!\n";
    while(<FILE>) {
        chomp;
        my @arr = split(/[\t ]/);
        if(@arr != 3) {
            print STDERR "WARNING: Bad line in lexical probability file $_\n";
        } else {
            $lex{"$arr[0]\t$arr[1]"} = $arr[2];
        }
    }
    close FILE;
    print STDERR "Done loading lexical probabilities\n";
}

sub strip_arr {
    my $str = shift;
    my $isstring = !($str =~ /^[^ ]+ \( /);
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
    my ($srcarr, $trgarr) = @_;
    my $ret = 0;
    foreach my $f (@$trgarr) {
        my $prob = 0;
        foreach my $e (@$srcarr, "NULL") {
            $prob += max($min_prob, $lex{"$f\t$e"});
        }
        $ret += ($prob ? log($prob/(@$srcarr+1)) : -99);
    }
    return $ret;
}

my @fof;
my $SYNTAX_FEATS = ($TRG_SYNTAX or $SRC_LABEL or $TRG_LABEL or $SRC_TRG_LABEL);
sub print_counts {
    my $src = shift;
    my $counts = shift;
    return if ($src eq "") and not $KEEP_EMPTY;
    my @srcarr = strip_arr($src);
    my $sum = sum(map { $_->[1] } @$counts);
    return if $sum < $SRC_MIN_FREQ;
    my $lsum = log($sum);
    foreach my $kv (@$counts) {
        my $trg = $kv->[0];
        my $cnt = $kv->[1];
        next if ($trg eq "") and not $KEEP_EMPTY;
        my $words = 0;
        my @trgarr = strip_arr($trg);
        # If we are using target side syntax and the rule is bad
        my $extra_feat;
        if($SYNTAX_FEATS) {
            $src =~ /^([^ ]+) /;
            my $src_lab = $1;
            $trg =~ /@ ([^ ]+)/; #~ / @ ([^ ]+)/
            my $trg_lab = $1;
            $extra_feat .= " ${PREFIX}isx=1" if($TRG_SYNTAX and $trg_lab eq "\@X\@");
            $extra_feat .= " ${PREFIX}sl_${src_lab}=1" if $SRC_LABEL and $src_lab;
            $extra_feat .= " ${PREFIX}tl_${trg_lab}=1" if $TRG_LABEL and $trg_lab;
            $extra_feat .= " ${PREFIX}stl_${src_lab}_${trg_lab}=1" if $SRC_TRG_LABEL and $src_lab and $trg_lab;
        }
        # Find the counts/probabilities
        my $lfreq = ($cnt > 0) ? log($cnt) : -99;
        my $lprob = $lfreq-$lsum;
        print "$src ||| $trg ||| ".sprintf("${PREFIX}${COND_PREFIX}p=%f$extra_feat", $lfreq, $lprob);
        printf " ${PREFIX}${COND_PREFIX}l=%f", m1prob(\@srcarr, \@trgarr) if $LEX_PROB_FILE;
        if($JOINT) {
            printf " ${PREFIX}p=1 ${PREFIX}lfreq=%f";
            print " ${PREFIX}w=".scalar(@trgarr) if (@trgarr);
        }
        print " ||| $cnt $sum";
        print "\n";
        # Count the frequencies of frequencies
        $cnt = int($cnt);
        $fof[$cnt]++ if $FOF_FILE and ($cnt <= $FOF_MAX);
    }
}
my (@counts, $curr_id);
while(<STDIN>) {
    chomp;
    my @arr = split(/ \|\|\| /);
    if(@counts and ($arr[0] ne $curr_id)) {
        print_counts($curr_id, \@counts);
        @counts = ();
    }
    $curr_id = $arr[0];
    # Add to the count
    if(@counts and ($counts[-1]->[0] eq $arr[1])) {
        $counts[-1]->[1] += $arr[2];
    } else {
        push @counts, [$arr[1], $arr[2]];
    }
}
print_counts($curr_id, \@counts);

if($FOF_FILE) {
    open FILE0, ">:utf8", $FOF_FILE or die "Couldn't open $FOF_FILE\n";
    print FILE0 join("\n", @fof)."\n";
    close FILE0;
}
