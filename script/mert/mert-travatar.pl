#!/usr/bin/perl

use strict;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my ($SEED_WEIGHTS, $SRC, $REF, $LM, $TM, $TRAVATAR_DIR, $MOSES_DIR, $WORKING_DIR, $TRAVATAR, $DECODER_OPTIONS);

my $MAX_ITERS = 20;
my $NBEST = 100;
GetOptions(
    # Necessary
    "seed-weights=s" => \$SEED_WEIGHTS,
    "src=s" => \$SRC,
    "ref=s" => \$REF,
    "lm=s" => \$LM,
    "tm=s" => \$TM,
    "travatar-dir=s" => \$TRAVATAR_DIR,
    "moses-dir=s" => \$MOSES_DIR,
    "working-dir=s" => \$WORKING_DIR,
    # Options
    "travatar=s" => \$TRAVATAR,
    "decoder-options=s" => \$DECODER_OPTIONS,
    "max-iters=i" => \$MAX_ITERS,
    "nbest=i" => \$NBEST,
    # "=s" => \$,
    # "=s" => \$,
    # "=s" => \$,
    # "=s" => \$,
    # "=s" => \$,
);

if((not $SEED_WEIGHTS) or (not $SRC) or (not $REF) or (not $TRAVATAR_DIR) or (not $MOSES_DIR) or (not $WORKING_DIR) or (not $LM) or (not $TM)) {
    die "Must specify seed-weights, src, ref, travatar-dir, moses-dir, lm, tm, and working-dir";
}
$TRAVATAR = "$TRAVATAR_DIR/src/bin/travatar" if not $TRAVATAR;

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

if(@ARGV != 0) {
    print STDERR "Usage: $0\n";
    exit 1;
}

# Make the working directory
safesystem("mkdir $WORKING_DIR") or die "couldn't mkdir";
safesystem("cp $SEED_WEIGHTS $WORKING_DIR/run1.weights") or die "couldn't copy";
my $weight_cnt = `wc -l $WORKING_DIR/run1.weights`; chomp $weight_cnt;

# Do the outer loop
foreach my $iter (1 .. $MAX_ITERS) {
    my $pref = "$WORKING_DIR/run$iter";
    my $next = "$WORKING_DIR/run".($iter+1);
    safesystem("$TRAVATAR $DECODER_OPTIONS -nbest $NBEST -lm_file $LM -tm_file $TM -weight_file $pref.weights -nbest_out $pref.nbest < $SRC > $pref.out 2> $pref.err") or die "couldn't decode";
    safesystem("$TRAVATAR_DIR/script/mert/densify-nbest.pl $pref.weights < $pref.nbest > $pref.nbest-dense") or die "couldn't densify";
    safesystem("$MOSES_DIR/bin/extractor --scconfig case:true --scfile $pref.scores.dat --ffile $pref.features.dat -r $REF -n $pref.nbest-dense") or die "couldn't extract";
    safesystem("$TRAVATAR_DIR/script/mert/make-init-opt.pl < $pref.weights > $pref.init.opt") or die "couldn't make init opt";
    my $feats = join(",", map { "$WORKING_DIR/run$_.features.dat" } (1 .. $iter));
    my $scores = join(",", map { "$WORKING_DIR/run$_.scores.dat" } (1 .. $iter));
    safesystem("$MOSES_DIR/bin/mert -d $weight_cnt --scconfig case:true --scfile $scores --ffile $feats --ifile $pref.init.opt -n 20 > $pref.mert.out 2> $pref.mert.log") or die "couldn't mert"; 
    print `grep Best $pref.mert.log`;
    safesystem("$TRAVATAR_DIR/script/mert/update-weights.pl $pref.weights $pref.mert.log > $next.weights") or die "couldn't make init opt";
}
