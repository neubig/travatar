#!/usr/bin/perl

use strict;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle reduce);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my $SRC_MIN_FREQ = 0;
my $LEX_PROB_FILE = "";
my $LEX_TYPE = "all";
my $TRG_SYNTAX = 0;
my $SRC_LABEL = 0;
my $TRG_LABEL = 0;
my $SRC_TRG_LABEL = 0;
my $PREFIX = "";
my $JOINT = 0;
my $COND_PREFIX = "egf";
my $FOF_MAX = 20;
my $FOF_FILE;
GetOptions(
    "src-min-freq=i" => \$SRC_MIN_FREQ,   # Minimum frequency of a src pattern
    "lex-prob-file=s" => \$LEX_PROB_FILE, # File of lexical probabilities for
                                          # calculating model 1
    "lex-type=s" => \$LEX_TYPE,           # Calculate lexical probs. using "all" or "aligned" words
    "cond-prefix=s" => \$COND_PREFIX,     # Prefix for model 1
    "prefix=s" => \$PREFIX,               # Prefix for all features
    "joint" => \$JOINT,                   # word/phrase/lfreq features or not
    "trg-syntax" => \$TRG_SYNTAX,         # Use target side syntax
    "src-label" => \$SRC_LABEL,           # Calculate sparse features for the source labels
    "trg-label" => \$TRG_LABEL,           # Calculate sparse features for the target labels
    "src-trg-label" => \$SRC_TRG_LABEL,   # Calculate sparse features for the source/target labels
    "fof-file=s" => \$FOF_FILE,           # Save frequencies of frequencies to a file
);
my $LEX_ALL = ($LEX_TYPE eq "all");

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
    my @ret;
    while($_[0] =~ /"([^ ]+)"/g) {
        push @ret, $1;
    }
    return @ret;
}

# Calculate the m1 probability of f given e
my $min_log = -20;
sub m1prob {
    my ($srcarr, $trgarr, $align) = @_;
    my (@probs, @num, $ret);
    while($align =~ /([0-9]+)-([0-9]+)/g) {
        $probs[$1] += $lex{"$trgarr->[$2]\t$srcarr->[$1]"};
        $num[$1] += 1;
    }
    for(0 .. @$trgarr-1) {
        my $prob = ($num[$_] ? $probs[$_]/$num[$_] : $lex{"$trgarr->[$_]\tNULL"});
        $ret += ($prob ? log($prob) : $min_log);
    }
    return $ret;
}

my @fof;
my $SYNTAX_FEATS = ($TRG_SYNTAX or $SRC_LABEL or $TRG_LABEL or $SRC_TRG_LABEL);
sub print_counts {
    my ($src, $counts) = @_;
    my $sum;
    for(@$counts) { $sum += $_->[1]; }
    return if $sum < $SRC_MIN_FREQ;
    my @srcarr = strip_arr($src);
    # my $lsum = log($sum);
    foreach my $kv (@$counts) {
        my ($trg, $cnt, $align) = @$kv;
        # Find the number of words
        my @trgarr = strip_arr($trg);
        # Find the best alignment
        my ($bestalign, $maxalign);
        while(my ($k,$v) = each(%$align)) {
            if($v > $maxalign) {
                $bestalign = $k;
                $maxalign = $v;
            }
        }
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
        my $lprob = ($cnt > 0) ? log($cnt/$sum) : -99;
        print "$src ||| $trg ||| ".sprintf("${PREFIX}${COND_PREFIX}p=%f$extra_feat", $lprob);
        printf " ${PREFIX}${COND_PREFIX}l=%f", m1prob(\@srcarr, \@trgarr, $bestalign) if $LEX_PROB_FILE;
        if($JOINT) {
            printf " ${PREFIX}p=1";
            print " ${PREFIX}w=".scalar(@trgarr) if (@trgarr);
        }
        print " ||| $cnt $sum ||| $bestalign\n";
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
        $counts[-1]->[2]->{$arr[3]} += $arr[2];
    } else {
        push @counts, [$arr[1], $arr[2], {$arr[3] => $arr[2]}];
    }
}
print_counts($curr_id, \@counts);

if($FOF_FILE) {
    open FILE0, ">:utf8", $FOF_FILE or die "Couldn't open $FOF_FILE\n";
    print FILE0 join("\n", @fof)."\n";
    close FILE0;
}
