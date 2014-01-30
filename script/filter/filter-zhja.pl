#!/usr/bin/perl

# A script to remove phrases that delete content words
# from the phrase table

use strict;
use utf8;
use FileHandle;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my $SRC_COL = 0;
my $TRG_COL = 1;
my $SHOW_FILTERED = 0;
my $SKIP_PUNC = 0;
my $SKIP_QUEST = 0;
GetOptions(
"src-col=s" => \$SRC_COL,
"trg-col=s" => \$TRG_COL,
"show-filtered" => \$SHOW_FILTERED,
"skip-punc"    => \$SKIP_PUNC,
"skip-quest"    => \$SKIP_QUEST,
);

if(@ARGV != 0) {
    print STDERR "Usage: $0\n";
    exit 1;
}

sub punc_ok {
    my ($src, $trg, $srcw, $trgw) = @_;
    if(($srcw =~ /^。$/) != ($trgw =~ /^。$/)) { return 0; }
    return 1;
}

sub quest_ok {
    my ($src, $trg, $srcw, $trgw) = @_;
    my $zh_polar = ($srcw =~ /(\?|？|吗 。)$/) ? 1 : 0;
    my $ja_polar = ($trgw =~ /(\?|？|か 。|か い 。|かしら 。)$/) ? 1 : 0;
    return ($zh_polar == $ja_polar) ? 1 : 0;
}

sub words {
    my @arr;
    while($_[0] =~ /"([^ "]+)"/g) {
        push @arr, $1;
    }
    return "@arr";
}

while(<STDIN>) {
    chomp;
    my @arr = split(/ \|\|\| /);
    # Get the values
    my $src = $arr[$SRC_COL];
    my $trg = $arr[$TRG_COL];
    my $srcw = words($src);
    my $trgw = words($trg);
    # Check if OK
    my $ok = 1;
    if($ok and not $SKIP_QUEST) { $ok = quest_ok($src, $trg, $srcw, $trgw); }
    if($ok and not $SKIP_PUNC)  { $ok = punc_ok($src, $trg, $srcw, $trgw); }
    # Print
    print "$_\n" if 
        ($ok and (not $SHOW_FILTERED)) or 
        ((not $ok) and $SHOW_FILTERED);
}
