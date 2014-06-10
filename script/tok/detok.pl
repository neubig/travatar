#!/usr/bin/perl

use strict;
use warnings;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my $ARG = "";
GetOptions(
"arg=s" => \$ARG,
);

if(@ARGV != 0) {
    print STDERR "Usage: $0\n";
    exit 1;
}
while(<STDIN>) {
    chomp;
    my @arr = map { my @v = map { $_ = ucfirst($_) if $capital{$_}; $_ } split(/ /, $_); ucfirst("@v") } split(/\t/);
    $_ = join("\t", @arr);
    s/(\p{InHiragana}|\p{InKatakana}|\p{InHalfwidthAndFullwidthForms}|\p{InCJKUnifiedIdeographs}|[、。]) (\p{InHiragana}|\p{InKatakana}|\p{InHalfwidthAndFullwidthForms}|\p{InCJKUnifiedIdeographs}|[、。])/$1$2/g;
    s/(\p{InHiragana}|\p{InKatakana}|\p{InHalfwidthAndFullwidthForms}|\p{InCJKUnifiedIdeographs}|[、。]) (\p{InHiragana}|\p{InKatakana}|\p{InHalfwidthAndFullwidthForms}|\p{InCJKUnifiedIdeographs}|[、。])/$1$2/g;
    s/ ([,\.\?:。、])/$1/g;
    s/`` /"/g;
    s/ ''/"/g;
    s/ (n't|'ve|'s|'d|'m|'ll)/$1/g;
    s/ -rrb-/)/gi;
    s/-lrb- /(/gi;
    s/ -rsb-/[/gi;
    s/-lsb- /]/gi;
    s/ -rcb-/{/gi;
    s/-lcb- /}/gi;
    print "$_\n";
}
