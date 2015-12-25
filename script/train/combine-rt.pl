#!/usr/bin/perl

use strict;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);

my $SMOOTH = "none"; # The type of smoothing to use
my $KN_SIZE = 3;
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

# Load the FOF file if it's specified
my @fof;
if($FOF_FILE) {
    open FILE0, "<:utf8", $FOF_FILE or die "Couldn't open $FOF_FILE\n";
    @fof = map { chomp; $_ } <FILE0>;
    close FILE0;
}

# Calculate the Kneser-Ney discounts if necessary
my @kndisc;
if($SMOOTH eq "kn") {
    die "Must have FOF file of size at least $KN_SIZE" if (@fof <= $KN_SIZE+1);
    my $Y = $fof[1]/($fof[1]+2*$fof[2]);
    foreach my $i (1 .. $KN_SIZE) {
        $kndisc[$i] = max(0, $i-($i+1)*$Y*$fof[$i+1]/$fof[$i]);
    }
    print STDERR "Using Kneser-Ney Discounting: @kndisc[1 .. $KN_SIZE]\n";
}

*FILE0 = open_or_zcat($ARGV[0]) or die;
*FILE1 = open_or_zcat($ARGV[1]) or die;

sub add_feat {
    my $hash = shift;
    my $str = shift;
    for(split(/ +/, $str)) {
        my @kv = split(/=/);
        @kv == 2 or die "Bad feature vector $str\n";
        $hash->{$kv[0]} = $kv[1] if not defined $hash->{$kv[0]};
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
    my @arr0 = split(/ \|\|\| /, $s0, -1);
    my @arr1 = split(/ \|\|\| /, $s1, -1);
    die "Wrong number of columns:\nFILE0: $s0\nFILE1: $s1\n" if ((@arr0 != 5) or (@arr1 != 5));
    die "Rules don't match:\nFILE0: $s0\nFILE1: $s1\n" if ($arr0[0] ne $arr1[0]) or ($arr0[1] ne $arr1[1]);
    # Print the previous set of rules if necessary
    my $my_sym = $arr0[0];
    $my_sym =~ s/ @ [^ ]+//g;
    if($my_sym ne $curr) {
        print_queue(@queue);
        @queue = ();
        $curr = $my_sym;
    }
    # Save the features from both
    my %feat; add_feat(\%feat, $arr0[2]); add_feat(\%feat, $arr1[2]);
    my @cnt0 = split(/ /, $arr0[3]);
    my @cnt1 = split(/ /, $arr1[3]);
    ((@cnt0 == 2) and (@cnt1 == 2) and ($cnt0[0] == $cnt1[0])) or die "Bad counts $s0";
    # Perform modified Kneser-Ney smoothing, linearly scaling discounts between
    my $cnt = $cnt0[0];
    if(@kndisc) {
        my $down = int($cnt);
        my $up = $down+1;
        my $upfrac = $cnt-$down;
        my $downfrac = 1-$upfrac;
        my $disc = (($down >= $#kndisc) ? $kndisc[-1] : ($kndisc[$down]*$downfrac+$kndisc[$up]*$upfrac));
        $cnt -= $disc;
        $feat{"lfreq"} = $cnt ? log($cnt) : -99;
        $feat{"fgep"} = $cnt ? log($cnt/$cnt1[1]) : -99;
        $feat{"egfp"} = $cnt ? log($cnt/$cnt0[1]) : -99;
    } elsif($SMOOTH ne "none") {
        die "Unknown smoothing type: $SMOOTH\n";
    }
    # Make the string
    my $str = "$arr0[0] ||| $arr0[1] ||| ".join(" ", map { "$_=$feat{$_}" } keys %feat)." ||| $cnt0[0] $cnt0[1] $cnt1[1] ||| $arr0[4]\n";
    push @queue, [$cnt, $order++, $str];
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
