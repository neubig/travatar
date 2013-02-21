#!/usr/bin/perl

use strict;
use utf8;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my $BINARIZE = 1;

sub makesafe {
    $_ = shift;
    $_ =~ s/\(/-LRB-/g;
    $_ =~ s/\)/-RRB-/g;
    $_ =~ s/\[/-LSB-/g;
    $_ =~ s/\]/-RSB-/g;
    $_ =~ s/\{/-LCB-/g;
    $_ =~ s/\}/-RCB-/g;
    return $_;
}

sub readtree {
    $_ = shift;
    # Split and remove the leading ID
    my @lines = split(/\n/);
    shift @lines if $lines[0] =~ /^ID/;
    my @ret = map { my @arr = split(/ /); $arr[0]--; $arr[1]--; $arr[2] = makesafe($arr[2]); $arr[3] = makesafe($arr[3]); \@arr } @lines;
    return @ret;
}
sub getchildren {
    my ($tree, $root) = @_;
    my @children;
    for(@$tree) {
        push @children, $_->[0] if($_->[1] == $root);
    }
    return @children;
}

sub buildcfg {
    my ($tree, $root) = @_;
    my @child = getchildren($tree, $root);
    # Get the initial single-word node
    my $str = "(".$tree->[$root]->[3]." ".$tree->[$root]->[2].")";
    # If we want to binarize the tree
    if($BINARIZE) {
        # Traverse left->right for children that fall on the right hand side
        # And then right->left for children that fall on the left hand side
        # Then, finally, traverse right-hand-side punctuation
        @child = sort { 
            my $aa = ($a < $root ? 1e4 - $a : ($tree->[$a]->[3] =~ /記号/ ? 2e4 + $a : $a));
            my $bb = ($b < $root ? 1e4 - $b : ($tree->[$b]->[3] =~ /記号/ ? 2e4 + $b : $b));
            $aa <=> $bb } @child;
        # print "root=$root\tchild=@child\n";
        # Build the phrase constituents
        foreach my $c (@child) {
            my $child_str = buildcfg($tree, $c);
            $str = 
                "(".$tree->[$root]->[3]."P ".
                    ($c < $root ? "$child_str $str)" : "$str $child_str)");
        }
    # Otherwise, sort
    } else {
        push @child, $root;
        $str = "(".$tree->[$root]->[3]."P ".
            join(" ", map { ($_ == $root?$str:buildcfg($tree,$_)) } sort { $a <=> $b } @child).")";
    }
    return $str;
}

# Converts a dependency tree to a CFG parse
$/ = "\n\n";
while(<STDIN>) {
    chomp;
    my @deptree = readtree($_);
    print buildcfg(\@deptree, getchildren(\@deptree, -1))."\n";
}
