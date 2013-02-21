#!/usr/bin/perl

use strict;
use utf8;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

sub getchildren {
    my ($tree, $root) = @_;
    my @children;
    for(@$tree) {
        push @children, $_->[0] if($_->[1] == $root);
    }
    return @children;
}

sub readtree {
    $_ = shift;
    # Split and remove the leading ID
    my @lines = split(/\n/);
    shift @lines if $lines[0] =~ /^ID/;
    my @ret = map { my @arr = split(/ /); $arr[0]--; $arr[1]--; 
                    $arr[3] = "$arr[3]-$arr[2]" if $arr[3] =~ /^助詞$/; \@arr } @lines;
    # Find all values that are a head
    my @ishead = map { 0 } @ret;
    for(@ret) { $ishead[$_->[1]]++ if $_->[1] >= 0; }
    # For verb phrases that look like
    #   A -> B -> C
    # reverse them so that they look like
    #   A <- B <- C
    # as long as there are no incoming dependencies
    for(my $i = $#ret; $i > 0; $i--) {
        next if ($ishead[$i] != 1);
        $ret[$i-1]->[3] = "動詞" if not (($ret[$i]->[2] !~ /^(する|し|さ|せ)$/) or ($ret[$i-1]->[3] !~ /^名詞$/)); # Sahen nouns
        next if
                ( 
                (($ret[$i]->[3] !~ /^(動詞|助動詞|語尾)$/) or ($ret[$i-1]->[3] !~ /^(動詞|助動詞|語尾)$/)) and # Verb phrases
                (($ret[$i]->[2] !~ /^[はも]$/) or ($ret[$i-1]->[3] !~ /^助詞$/)) # "は" or "も" preceded by a different particle
                )
                or
                ($ret[$i-1]->[1] != $i);
        $ret[$i-1]->[1] = $ret[$i]->[1];
        $ret[$i]->[1] = $i-1;
    }
    # For common punctuation, propagate them up to the first head on the left
    for(my $i = $#ret; $i > 0; $i--) {
        next if $ret[$i]->[2] !~ /^[、,。\.：:]$/;
        my @children = getchildren(\@ret, $i);
        next if not @children;
        $ret[$children[-1]]->[1] = $ret[$i]->[1];
        for(@children[0 .. $#children-1], $i) { $ret[$_]->[1] = $ret[$children[-1]]->[0]; }
    }
    return @ret;
}

$/ = "\n\n";
while(<STDIN>) {
    chomp;
    my @deptree = readtree($_);
    print join("\n", map { sprintf("%03d %03d %s %s %s", $_->[0]+1, $_->[1]+1, $_->[2], $_->[3], $_->[4]) } @deptree )."\n\n";
}
