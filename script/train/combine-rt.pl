#!/usr/bin/perl

use strict;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);

my $OPERATOR = "intersection";
GetOptions(
    "operator=s" => \$OPERATOR
);

if(@ARGV != 2) {
    print STDERR "Usage: $0 PT1 PT2\n";
    exit 1;
}

*FILE0 = open_or_zcat($ARGV[0]) or die;
*FILE1 = open_or_zcat($ARGV[1]) or die;

my @ss;
my %buffer;
while(1) {
    my ($next, $id);
    # load the lines approximately in order
    if($ss[0] le $ss[1]) {
        $id = 0;
        if(not ($next = <FILE0>)) {
            $id = 1;
            if(not ($next = <FILE1>)) {
                last;
            }
        }
    } else {
        $id = 1;
        if(not ($next = <FILE1>)) {
            $id = 0;
            if(not ($next = <FILE0>)) {
                last;
            }
        }
    }
    # Get the array
    chomp $next;
    my @arr = split(/ \|\|\| /, $next);
    @arr == 3 or die "bad line $next\n";
    my $pid = "$arr[0] ||| $arr[1]";
    $ss[$id] = $pid;
    my $has = $buffer{$pid};
    $buffer{$pid} = {} if not $has;
    for(split(/ +/, $arr[2])) {
        my @kv = split(/=/);
        @kv == 2 or die "bad line $next\n";
        $buffer{$pid}->{$kv[0]} = $kv[1] if (($id == 1) or ($kv[0] ne "w"));
    }
    if($has) {
        print "$pid ||| ".join(" ", map { "$_=".$buffer{$pid}->{$_} } sort keys %{$buffer{$pid}})."\n";
        delete $buffer{$pid};
    }
}

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
