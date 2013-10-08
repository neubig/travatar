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

my $SHOW_FILTERED = 0;
my $SKIP_SAHEN = 0;
my $SKIP_NUMBER = 0;
GetOptions(
"show-filtered" => \$SHOW_FILTERED,
"skip-sahen" => \$SKIP_SAHEN,
"skip-number" => \$SKIP_NUMBER,
);

if(@ARGV != 0) {
    print STDERR "Usage: $0\n";
    exit 1;
}

# Make sure that "suru" is not assigned to overly many verbs
#  Currently a simple rule that says if the Japanese side starts with
#  "し" "する" or "さ", the only verb acceptable is "do" "did" "does" "doing"
sub sahen_ok {
    if(($_[1] =~ /^"(し|する|さ")"/) and
       ($_[0] !~ /"(do|did|does|doing)"/)) {
        return 0;
    }
    return 1;
}

# Make sure that all single-word numbers are translated as the identity
sub number_ok {
    my %num;
    while($_[0] =~ /"([0-9]+)"/g) { $num{$1}++; }
    if(keys(%num)) { while($_[1] =~ /"([0-9]+)"/g) { $num{$1}--; } }
    for(values %num) {
        return 0 if $_ != 0;
    }
    return 1;
}

while(<STDIN>) {
    chomp;
    my @arr = split(/ \|\|\| /);
    my $ok = 1;
    if($ok and not $SKIP_SAHEN) { $ok = sahen_ok(@arr); }
    if($ok and not $SKIP_NUMBER) { $ok = number_ok(@arr); }
    if($ok and not $SKIP_HIRA) { $ok = hira_ok(@arr); }
    print "$_\n" if 
        ($ok and (not $SHOW_FILTERED)) or 
        ((not $ok) and $SHOW_FILTERED);
}
