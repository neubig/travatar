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
my $SKIP_SAHEN = 0;
my $SKIP_NUMBER = 0;
my $SKIP_TRGPART = 0;
my $SKIP_POLAR = 0;
GetOptions(
"src-col=s" => \$SRC_COL,
"trg-col=s" => \$TRG_COL,
"show-filtered" => \$SHOW_FILTERED,
"skip-sahen"    => \$SKIP_SAHEN,
"skip-number"   => \$SKIP_NUMBER,
"skip-trgpart"  => \$SKIP_TRGPART,
"skip-polar"  => \$SKIP_POLAR,
);

if(@ARGV != 0) {
    print STDERR "Usage: $0\n";
    exit 1;
}

# Make sure that "suru" is not assigned to overly many verbs
#  Currently a simple rule that says if the Japanese side starts with
#  "し" "する" or "さ", the only verb acceptable is "do" "did" "does" "doing"
sub sahen_ok {
    # # For now, this should not apply to modals
    # for($_[$SRC_COL] =~ /(x\d+):v(p|b.*) /g) {
    #     my $vs = $1;
    #     $_[$TRG_COL] =~ s/$vs/VP/g;
    # }
    my @arr = split(/ /,$_[$TRG_COL]);
    # This is too agressive for now, but eventually we'd like to remove ones
    # that take variables as well
    # while($arr[0] =~ /^(x|"を")/) { shift @arr; }
    if(($arr[0] =~ /^"(し|する|さ)"/) and
       ($_[$SRC_COL] !~ /"(do|did|does|doing)"/)) {
        return 0;
    }
    return 1;
}

# Make sure that all single-word numbers are translated as the identity
sub number_ok {
    my %num;
    while($_[$SRC_COL] =~ /"([0-9]+)"/g) { $num{$1}++; }
    my $newj = $_[$TRG_COL];
    $newj =~ tr/一ニ三四五六七八九〇/1234567890/;
    $newj =~ tr/１２３４５６７８９０/1234567890/;
    if(keys(%num)) { while($newj =~ /"([0-9]+)"/g) { $num{$1}--; } }
    for(values %num) { return 0 if $_ != 0; }
    return 1;
}

# Make sure that no content word is translated into a single Hiragana
sub trgpart_ok {
    return 1 if (($_[$SRC_COL] !~ /^[^pit][^ ]* \( "/) and ($_[$SRC_COL] !~ / [^pit][^ ]* \( "/));
    my $size = 0;
    my $str;
    while($_[$TRG_COL] =~ /"([^" ]+)"/g) { $size++; $str = $1; }
    return 1 if (($size != 1) or ($str !~ /^\p{Hiragana}$/));
    return 0;
}

sub polar_ok {
    my $en_polar = ($_[$SRC_COL] =~ /"(not|n't|no|without)"/) ? 1 : 0;
    my $ja_polar = ($_[$TRG_COL] =~ /("ま" "せ" "ん"|"な" "い"|"な" "かっ" "た"|"な" "く"|"な" "けれ"|"いいえ")/) ? 1 : 0;
    $ja_polar = 0 if ($ja_polar == 1) and ($_[$TRG_COL] =~ /"か" "も"/);
    return ($en_polar == $ja_polar) ? 1 : 0;
}

while(<STDIN>) {
    chomp;
    my @arr = split(/ \|\|\| /);
    my $ok = 1;
    if($ok and not $SKIP_TRGPART) { $ok = trgpart_ok(@arr); }
    if($ok and not $SKIP_NUMBER) { $ok = number_ok(@arr); }
    if($ok and not $SKIP_SAHEN) { $ok = sahen_ok(@arr); }
    if($ok and not $SKIP_POLAR) { $ok = polar_ok(@arr); }
    print "$_\n" if 
        ($ok and (not $SHOW_FILTERED)) or 
        ((not $ok) and $SHOW_FILTERED);
}
