#!/usr/bin/perl

use strict;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my $SYM = "grow";
my $DIAG = 1;
my $FINAL = 1;
my $AND = 1;

if(@ARGV != 0) {
    print STDERR "Usage: $0 --sym=grow [--diag] [--final] [--and]\n";
    exit 1;
}

sub grow {

    (@_ == 2) or die "grow accepts two alignment arrays";
    my ($a, $b) = @_;
    
    # Add the neighboring positions
    my @neighbors = ( [-1,0], [0,-1], [1,0], [0,1] );
    if($DIAG) {
        push @neighbors, [-1,-1], [-1,1], [1,-1], [1,1];
    }
    my @fa = map { 0 } (1 .. @$a);
    my @ea = map { 0 } (1 .. @$b);
    my %A;

    # Symmetric and union alignments
    my (%sym, %uni);

    # Find the alignments in one direction
    foreach my $j (1 .. @$a) {
        if($a->[$j]) {
            my $aj = $a->[$j];
            $uni{"$aj-$j"} = 1;
            # If symmetric
            if($b->[$aj] == $j) {
                $fa[$j] = 1;
                $ea[$aj] = 1;
                $A{"$aj-$j"} = 2;
                $sym{"$aj-$j"} = 1;
            # Otherwise, inverse
            } else {
                $A{"$aj-$j"} = -1;
            }
        }
    }
    # Find all direct alignments
    foreach my $i (1 .. @$b) {
        if($b->[$i] and $a->[$b->[$i]]) {
            $uni{"$i-$b->[$i]"} = 1;
            $A{"$i-$b->[$i]"} = 1;
        }
    }
    
    # Continue to add until we are done
    my $added = 1;
    while($added) {
        $added = 0;
        # Scan the current alignment
        for(keys %sym) {
            my ($si, $sj) = split(/-/);
            # Go over all the neighbors
            for(@neighbors) {
                my ($pi, $pj) = ($_->[0]+$si, $_->[1]+$sj);
                # Make sure we are within boundaries
                next if ($pi <= 0) or ($pi > @$a) or ($pj <= 0) or ($pj > @$b);
                # Make sure we are in the union alignment
                next if ($b->[$pi] != $pj) and ($a->[$pj] != $pi);
                # Make sure we connect at least one uncovered point
                next if ($ea[$pi] and $fa[$pj]);
                # If we satisfy all these conditions, add to the alignment
                $A{"$pi-$pj"} = 2;
                $ea[$pi] = 1;
                $fa[$pj] = 1;
                $added = 1;
            }
        }
    }

    # Add the final alignments
    if($FINAL) {
        for(keys %uni) {
            my ($si, $sj) = split(/-/);
            if($A{"$si-$sj"} == 1) {
                if( ($AND and (not $ea[$si]) and (not $fa[$sj])) or
                    (not $AND) and not ($ea[$si] and $fa[$sj]) ) {
                    $sym{"$si-$sj"} = 1;
                    $A{"$si-$sj"} = 2;
                    $ea[$si] = 1;
                    $fa[$sj] = 1;
                }
            }
        }
        for(keys %uni) {
            my ($si, $sj) = split(/-/);
            if($A{"$si-$sj"} == -1) {
                if( ($AND and (not $ea[$si]) and (not $fa[$sj])) or
                    (not $AND) and not ($ea[$si] and $fa[$sj]) ) {
                    $sym{"$si-$sj"} = 1;
                    $A{"$si-$sj"} = 2;
                    $ea[$si] = 1;
                    $fa[$sj] = 1;
                }
            }
        }
    }
    return map {my ($i, $j) = split(/-/); "".($i-1)."-".($j-1)} sort keys %sym;
}


