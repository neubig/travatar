#!/usr/bin/perl

use strict;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);

my $SMOOTH = "none";
my $FOF_FILE;
my $TOP_N;
GetOptions(
    "smooth=s" => \$SMOOTH,
    "fof-file=s" => \$FOF_FILE,
    "top-n=s" => \$TOP_N
);

if(@ARGV != 2) {
    print STDERR "Usage: $0 PT1 PT2\n";
    exit 1;
}

*FILE0 = open_or_zcat($ARGV[0]) or die;
*FILE1 = open_or_zcat($ARGV[1]) or die;

sub add_feat {
    my $hash = shift;
    my $str = shift;
    for(split(/ +/, $str)) {
        my @kv = split(/=/);
        @kv == 2 or die "Bad feature vector $str\n";
        $hash->{$kv[0]} = $kv[1];
    }
}

sub print_queue {
    my @arr = @_;
    if($TOP_N and (@arr > $TOP_N)) {
        @arr = sort { my $val = ($b->[0] <=> $a->[0]); $val = ($a->[1] <=> $b->[1]) if not $val; $val } @_;
        @arr = @arr[0 .. $TOP_N-1];
        @arr = sort { $a->[1] <=> $b->[1] } @arr;
    }
    print join("", map { $_->[2] } @arr);
}

my ($s0, $s1, $curr, @queue, $order);
while(1) {
    my ($next, $id);
    # Read the lines
    my $has0 = defined($s0 = <FILE0>);
    my $has1 = defined($s1 = <FILE1>);
    die "File sizes don't match:\nFILE0: $s0\nFILE1: $s1\n" if($has0 != $has1);
    last if not $has0;
    # Get the arrays and check
    chomp $s0; chomp $s1;
    my @arr0 = split(/ \|\|\| /, $s0);
    my @arr1 = split(/ \|\|\| /, $s1);
    die "Wrong number of columns:\nFILE0: $s0\nFILE1: $s1\n" if ((@arr0 != 4) or (@arr1 != 4));
    die "Rules don't match:\nFILE0: $s0\nFILE1: $s1\n" if ($arr0[0] ne $arr1[0]) or ($arr0[1] ne $arr1[1]);
    # Print the previous set of rules if necessary
    if($arr0[0] ne $curr) {
        print_queue(@queue);
        @queue = ();
        $curr = $arr0[0];
    }
    # Save the features from both
    my %feat; add_feat(\%feat, $arr0[2]); add_feat(\%feat, $arr1[2]);
    my @cnt0 = split(/ /, $arr0[3]);
    my @cnt1 = split(/ /, $arr1[3]);
    ((@cnt0 == 2) and (@cnt1 == 2) and ($cnt0[0] == $cnt1[0])) or die "Bad counts $s0";
    # Make the string
    my $str = "$arr0[0] ||| $arr0[1] ||| ".join(" ", map { "$_=$feat{$_}" } keys %feat)." ||| $cnt0[0] $cnt0[1] $cnt1[1]\n";
    push @queue, [$cnt0[0], $order++, $str];
}
print_queue(@queue);

# utilities
sub open_or_zcat {
  my $fn = shift;
  my $read = $fn;
  $fn = $fn.".gz" if ! -e $fn && -e $fn.".gz";
  $fn = $fn.".bz2" if ! -e $fn && -e $fn.".bz2";
  if ($fn =~ /\.bz2$/) {
      $read = "bzcat $fn|";
  } elsif ($fn =~ /\.gz$/) {
      $read = "gzip -cd $fn|";
  }
  my $hdl;
  open($hdl,$read) or die "Can't read $fn ($read)";
  return $hdl;
}
