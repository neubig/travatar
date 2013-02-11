#!/usr/bin/perl

use strict;
use warnings;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

if(@ARGV != 4) {
    print STDERR "Usage: $0 INPUT_CONFIG OUTPUT_CONFIG OUTPUT_DIR FILTER_CMD\n";
    exit 1;
}

# Read in the configuration file
my $model = "";
my @conf;
my $output = "$ARGV[2]/rule-table";
open FILE0, "<:utf8", $ARGV[0] or die "Couldn't open $ARGV[0]\n";
while(<FILE0>) {
    chomp;
    push @conf, "$_";
    if(/^\[tm_file\]$/) {
        die "Two model files" if $model;
        $model = <FILE0>;
        chomp $model;
        push @conf, $output;
    }
}
close FILE0;

# Do the actual filtering of the rule table
my $CAT = (($model =~ /\.gz$/)? "zcat" : "cat");
safesystem("mkdir $ARGV[2]");
safesystem("$CAT $model | $ARGV[3] > $output") or die;

# Print out the new configuration
open FILE3, ">:utf8", $ARGV[1] or die "Couldn't open $ARGV[1]\n";
print FILE3 join("\n", @conf)."\n";
close FILE3;

# Auxiliary functions
sub safesystem {
  print STDERR "Executing: @_\n";
  system(@_);
  if ($? == -1) {
      warn "Failed to execute: @_\n  $!";
      exit(1);
  } elsif ($? & 127) {
      printf STDERR "Execution of: @_\n  died with signal %d, %s coredump\n",
          ($? & 127),  ($? & 128) ? 'with' : 'without';
      exit(1);
  } else {
    my $exitcode = $? >> 8;
    warn "Exit code: $exitcode\n" if $exitcode;
    return ! $exitcode;
  }
}
