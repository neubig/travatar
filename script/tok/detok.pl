#!/usr/bin/perl

use strict;
use warnings;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my $LANG = "en";
GetOptions(
"lang=s" => \$LANG,
);

if(@ARGV != 0) {
    print STDERR "Usage: $0\n";
    exit 1;
}
while(<STDIN>) {
    chomp;
    # Units
    s/([0-9]) (|T|G|M|k|h|da|d|c|m|μ|n|p)(m|g|l|s|A|K|mol|cd|rad|sr|Hz|N|Pa|J|W|C|V|eV|F|Ω|S|Wb|T|H|lm|lx|Bq|Gy|Sv|kat|rpm|Wd|dB|db)( |\/|-)/$1$2$3$4/g;
    s/([0-9]) (℃|°|K) /$1$2 /g;
    s/(\p{InHiragana}|\p{InKatakana}|\p{InHalfwidthAndFullwidthForms}|\p{InCJKUnifiedIdeographs}|[、。「」　“”]) (\p{InHiragana}|\p{InKatakana}|\p{InHalfwidthAndFullwidthForms}|\p{InCJKUnifiedIdeographs}|[、。「」　“”])/$1$2/g;
    s/(\p{InHiragana}|\p{InKatakana}|\p{InHalfwidthAndFullwidthForms}|\p{InCJKUnifiedIdeographs}|[、。「」　“”]) (\p{InHiragana}|\p{InKatakana}|\p{InHalfwidthAndFullwidthForms}|\p{InCJKUnifiedIdeographs}|[、。「」　“”])/$1$2/g;
    s/ ([,\.\?:。、%])/$1/g;
    s/ (-) /$1/g;
    s/\\\*/*/g;
    s/ \\\/ /\//g;
    s/`` /"/g;
    s/ ''/"/g;
    s/ (n't|'ve|'s|'d|'m|'ll)/$1/gi;
    s/ ?-rrb-/)/gi;
    s/-lrb- ?/(/gi;
    s/ ?-rsb-/]/gi;
    s/-lsb- ?/[/gi;
    s/ ?-rcb-/}/gi;
    s/-lcb- ?/{/gi;
    if($LANG !~ /^(zh|ja)$/) {
        $_ = ucfirst($_);
    }
    print "$_\n";
}
