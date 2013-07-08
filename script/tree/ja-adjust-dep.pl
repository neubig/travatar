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

my %particles = (
    # Case particles
    "が"=>1,"の"=>1,"を"=>1,"に"=>1,"へ"=>1,"と"=>1,"から"=>1,"より"=>1,"で"=>1,
    # Parallel particles
    "と"=>1,"や"=>1,"か"=>1,
    # Adverbial particles
    "ばかり"=>1,"まで"=>1,"だけ"=>1,"ほど"=>1,"くらい"=>1,"ぐらい"=>1,"など"=>1,
    # Particles that are actually part of conjugation often
    "て"=>1,"つつ"=>1,"ば"=>1,
);

sub readtree {
    $_ = shift;
    # Split and remove the leading ID
    my @lines = split(/\n/);
    shift @lines if $lines[0] =~ /^ID/;
    my @ret = map { my @arr = split(/ +/); $arr[0]--; $arr[1] = max($arr[1]-1,-1); 
                    $arr[3] = "$arr[3]$arr[2]" if ((($arr[3] =~ /^助詞$/) and $particles{$arr[2]})   or 
                                                    (($arr[3] =~ /^助動詞$/) and ($arr[2] =~ /^で$/)) or
                                                    (($arr[3] =~ /^形容詞$/) and ($arr[2] =~ /^な$/))); \@arr } @lines;
    # Find all values that are a head
    my @ishead = map { 0 } @ret;
    for(@ret) { $ishead[$_->[1]]++ if $_->[1] >= 0; }
    # Relabel sa-hen nouns
    foreach my $i ( 1 .. $#ret ) {
        $ret[$i-1]->[3] = "動詞" if not (($ret[$i]->[2] !~ /^(する|す|し|さ|せ)$/) or ($ret[$i-1]->[3] !~ /^名詞$/));
    }
    # Find chunks. These include:
    #  - a verb (including sa-hen) followed by auxiliaries, word endings, or other verbs
    #  - an adjective followed by word endings
    #  - strings of particles
    my @chunks = ( 0 );
    my @head_type; push @head_type, $ret[0]->[3] if @ret;
    for(my $i = 1; $i < @ret; $i++) {
        my $connect = 0;
        $connect = 1 if(
            ($ishead[$i] == 1) and
            ((($head_type[-1] =~ /^(動詞|形容詞|助動詞)/) and 
               (($ret[$i]->[3] =~ /^(語尾|助動詞)/) or
                ($ret[$i]->[3] =~ /^助詞(て|つつ|ば)/) or
                (($ret[$i]->[3] =~ /^(動詞|助詞も|助詞は|形容詞な)/) and ($head_type[-1] eq "助動詞で")) or
                (($ret[$i]->[3] =~ /^動詞$/) and ($head_type[-1] eq "動詞")))) or
             (($head_type[-1] =~ /^助詞(で|に|と)/) and ($ret[$i]->[3] =~ /^助詞(は|も)/))));
             # (($head_type[-1] =~ /^名詞$/) and ($ret[$i]->[3] =~ /^接尾辞$/))));
        # print "$ret[$i]->[3] $head_type[-1] $connect\n";
        if($connect) {
            push @chunks, $chunks[-1];
        } else {
            push @chunks, $chunks[-1]+1;
            push @head_type, $ret[$i]->[3];
        }
    }
    my $last_verb = undef;
    # Reverse phrases in the same chunk
    for(my $i = $#ret; $i > 0; $i--) {
        if($chunks[$i-1] == $chunks[$i]) {
            $ret[$i-1]->[1] = $ret[$i]->[1];
            $ret[$i]->[1] = $i-1;
            if($ret[$i-1]->[3] eq "動詞") {
                $ret[$last_verb]->[1] = $i-1 if(defined $last_verb);
                $last_verb = $i-1;
            }
        } else {
            $last_verb = undef;
        }
    }
    # For common punctuation, propagate them up to the first head on the left
    for(my $i = $#ret; $i > 0; $i--) {
        next if $ret[$i]->[2] !~ /^[、,。\.：:．，]$/;
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
