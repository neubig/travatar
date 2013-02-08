#!/usr/bin/perl

use strict;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my $SYM = "grow";
my $DIAG = "true";
my $FINAL = "true";
my $AND = "true";
GetOptions(
    "sym=s" => \$SYM,
    "diag=s" => \$DIAG,
    "final=s" => \$FINAL,
    "and=s" => \$AND,
);
$DIAG = ($DIAG eq "true");
$FINAL = ($FINAL eq "true");
$AND = ($AND eq "true");

if(@ARGV != 2) {
    print STDERR "Usage: $0 [--sym=grow] [--diag=true/false] [--final=true/false] [--and=true/false] FE EF\n";
    exit 1;
}

sub grow {

    (@_ == 2) or die "grow accepts two alignment arrays";
    my ($ain, $bin) = @_;
    
    # Add the neighboring positions
    my @neighbors = ( [-1,0], [0,-1], [1,0], [0,1] );
    if($DIAG) {
        push @neighbors, [-1,-1], [-1,1], [1,-1], [1,1];
    }
    my @fa = map { 0 } (1 .. @$ain);
    my @ea = map { 0 } (1 .. @$bin);
    my %A;

    # Symmetric and union alignments
    my (@sym, @uni);

    # Find the alignments in one direction
    foreach my $j (1 .. @$ain) {
        if($ain->[$j]) {
            my $aj = $ain->[$j];
            push @uni, [$aj, $j];
            # If symmetric
            if($bin->[$aj] == $j) {
                $fa[$j] = 1;
                $ea[$aj] = 1;
                $A{"$aj-$j"} = 2;
                push @sym, [$aj, $j];
            # Otherwise, inverse
            } else {
                $A{"$aj-$j"} = -1;
            }
        }
    }
    # Find all direct alignments
    foreach my $i (1 .. @$bin) {
        if($bin->[$i] and ($ain->[$bin->[$i]]!=$i)) {
            push @uni, [$i, $bin->[$i]];
            $A{"$i-$bin->[$i]"} = 1;
        }
    }
    @uni = sort { my $val = ($a->[0] <=> $b->[0]); $val = ($a->[1] <=> $b->[1]) if not $val; $val } @uni;
    # print STDERR "uni: ".join(" ",map { "".$_->[0]."-".$_->[1] } @uni)."\n";
    # print STDERR "sym1: ".join(" ",map { "".$_->[0]."-".$_->[1] } @sym)."\n";
    
    # Continue to add until we are done
    my $added = 1;
    while($added) {
        $added = 0;
        # Scan the current alignment
        for(@sym) {
            my ($si, $sj) = @$_;
            # Go over all the neighbors
            for(@neighbors) {
                my ($pi, $pj) = ($_->[0]+$si, $_->[1]+$sj);
                # Make sure we are within boundaries
                next if (($pi <= 0) or ($pi > @$bin) or ($pj <= 0) or ($pj > @$ain));
                # Make sure we are in the union alignment
                next if (($bin->[$pi] != $pj) and ($ain->[$pj] != $pi));
                # Make sure we connect at least one uncovered point
                next if ($ea[$pi] and $fa[$pj]);
                # If we satisfy all these conditions, add to the alignment
                push @sym, [$pi, $pj];
                $A{"$pi-$pj"} = 2;
                $ea[$pi] = 1;
                $fa[$pj] = 1;
                $added = 1;
            }
        }
    }

    # Add the final alignments
    if($FINAL) {
        for(@uni) {
            my ($si, $sj) = @$_;
            if($A{"$si-$sj"} == -1) {
                if( ($AND and (not $ea[$si]) and (not $fa[$sj])) or
                    ((not $AND) and not ($ea[$si] and $fa[$sj])) ) {
                    push @sym, [$si, $sj];
                    $A{"$si-$sj"} = 2;
                    $ea[$si] = 1;
                    $fa[$sj] = 1;
                }
            }
        }
        for(@uni) {
            my ($si, $sj) = @$_;
            if($A{"$si-$sj"} == 1) {
                if( ($AND and (not $ea[$si]) and (not $fa[$sj])) or
                    ((not $AND) and not ($ea[$si] and $fa[$sj])) ) {
                    push @sym, [$si, $sj];
                    $A{"$si-$sj"} = 2;
                    $ea[$si] = 1;
                    $fa[$sj] = 1;
                }
            }
        }
    }

    @sym = sort { my $val = ($a->[1] <=> $b->[1]); $val = ($a->[0] <=> $b->[0]) if not $val; $val } @sym;
    return map { "". ($_->[0]-1) ."-". ($_->[1]-1) } @sym;
}

sub align_arr {
    my $str = shift;
    my $id = 0;
    my @arr = ();
    while($str =~ /\({ ([0-9 ]*)}\)/g) {
        for(split(/ /, $1)) {
            $arr[$_] = $id if $_;
        }
        $id++;
    }
    return @arr;
}

open FE, "<:utf8", $ARGV[0] or die "Couldn't open $ARGV[0]\n";
open EF, "<:utf8", $ARGV[1] or die "Couldn't open $ARGV[1]\n";
my ($fe, $ef);
while(1) {
    $fe = <FE>; $fe = <FE>; $fe = <FE>; chomp $fe;
    $ef = <EF>; $ef = <EF>; $ef = <EF>; chomp $ef;
    die "Uneven sentences in input and output" if(defined($fe) != defined($ef));
    last if not defined($fe);
    # print STDERR "fe=$fe\nef=$ef\n";
    my @fea = align_arr($fe);
    my @efa = align_arr($ef);
    # print STDERR "fea=@fea\nefa=@efa\n";
    my @alarr;
    if($SYM eq "grow") {
        @alarr = grow(\@fea, \@efa);
    } else {
        die "Bad symmetrization type $SYM\n";
    }
    print join(" ", @alarr)."\n";
}
close FE;
close EF;
