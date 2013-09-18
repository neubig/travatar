#!/usr/bin/perl

use strict;
use utf8;
use FileHandle;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

if(@ARGV != 0) {
    print STDERR "Usage: $0\n";
    exit 1;
}

sub parse_s {
    $_ = shift;
    my @stack = ([]);    
    while(/ *(\([^ ]*|\)|[^\(\)]*)/g) {
        my $s = $1;
        if($s =~ /^\((.*)/) {
            push @stack, [$1];
        } elsif($s eq ")") {
            my $top = pop(@stack);
            push @{$stack[-1]}, $top;
        } else {
            push @{$stack[-1]}, $s;
        }
    }
    return $stack[0]->[0];
}

sub print_s {
    my $ref = shift;
    if("ARRAY" eq ref($ref)) {
        return "(".join(" ", map { print_s($_) } @$ref).")";
    } else { 
        return $ref;
    }
}

sub raise {
    my $ref = shift;
    my $lastnonpunc = 0;
    foreach my $i (1 .. @$ref-1) {
        my $c = $ref->[$i];
        next if ref($c) ne "ARRAY";
        raise($c);
        $lastnonpunc = $i if $c->[0] !~ /^\$*[\.,]/;
    }
    # Skip ones with no or all punctuation
    return if ($lastnonpunc == 0) or ($lastnonpunc == @$ref-1);
    # $ref= ["$ref[0]PUNC" [$ref[0 .. $lastnonpunc]] $ref[$lastnonpunc+1,$#ref]]
    my @middle = @$ref[0 .. $lastnonpunc];
    my @punc;
    foreach my $i ($lastnonpunc+1 .. @$ref-1) { push @punc, $ref->[$i]; }
    my @newref = ("$ref->[0]PUNC", \@middle, @punc);
    @$ref = @newref;
}

while(<STDIN>) {
    chomp;
    my $tree = parse_s($_);
    raise($tree);
    print print_s($tree)."\n";
}
