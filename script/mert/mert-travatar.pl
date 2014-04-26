#!/usr/bin/perl

use strict;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my ($SRC, $REF, $TRAVATAR_CONFIG, $TRAVATAR_DIR, $MOSES_DIR, $WORKING_DIR, $TRAVATAR, $DECODER_OPTIONS, $NO_FILTER_RT);

# Exclusive command only for tuning geoquery parser
my ($GEOTUNE_COMMAND);

my $MERT_SOLVER = "batch-tune"; # Can be set to "moses" to use Moses's MERT solver
my $EVAL = "bleu"; # The evaluation measure to use
my $TUNE_OPTIONS = ""; # Other options to pass to batch-tune
my $MAX_ITERS = 20;
my $MIN_DIFF = 0.001;
my $CAND_TYPE = "nbest"; # Can be set to "forest" for forest-based mert
my $IN_FORMAT = "penn"; # The format of the input
my $NBEST = 200;
my $TRG_FACTORS = 1;
my $THREADS = 1; # The number of threads to use
GetOptions(
    # Necessary
    "travatar-dir=s" => \$TRAVATAR_DIR,
    "working-dir=s" => \$WORKING_DIR,
    "src=s" => \$SRC,
    "ref=s" => \$REF,
    "travatar-config=s" => \$TRAVATAR_CONFIG,
    # Options
    "moses-dir=s" => \$MOSES_DIR,
    "travatar=s" => \$TRAVATAR,
    "decoder-options=s" => \$DECODER_OPTIONS,
    "tune-options=s" => \$TUNE_OPTIONS,
    "max-iters=i" => \$MAX_ITERS,
    "cand-type=s" => \$CAND_TYPE,
    "nbest=i" => \$NBEST,
    "mert-solver=s" => \$MERT_SOLVER,
    "in-format=s" => \$IN_FORMAT,
    "eval=s" => \$EVAL,
    "threads=i" => \$THREADS,
    "no-filter-rt!" => \$NO_FILTER_RT,
    "trg-factors=i" => \$TRG_FACTORS,
    "geotune-command=s" => \$GEOTUNE_COMMAND,
    # "=s" => \$,
    # "=s" => \$,
    # "=s" => \$,
    # "=s" => \$,
    # "=s" => \$,
);

# Sanity check
if((not $TRAVATAR_CONFIG) or (not $SRC) or (not $REF) or (not $TRAVATAR_DIR) or (not $WORKING_DIR)) {
    die "Must specify travatar-config, src, ref, travatar-dir, tm, and working-dir";
}
($MERT_SOLVER eq "moses") or ($MERT_SOLVER eq "batch-tune") or die "Bad MERT solver: $MERT_SOLVER";
($CAND_TYPE eq "nbest") or ($CAND_TYPE eq "forest") or ($CAND_TYPE eq "nbestgeo") or die "Bad candidate type: $CAND_TYPE";
($MERT_SOLVER eq "moses") and (not $MOSES_DIR) and "Must specify -moses-dir when using the Moses MERT solver.";
($EVAL =~ /^(bleu|ribes|interp|ter)/) or die "Bad evaluation measure $EVAL";
($DECODER_OPTIONS =~ /(in_format|trg_factors)/) and die "Travatar's $1 should not be specified through -decoder-options, but through the option of mert-travatar.pl";
$TRAVATAR = "$TRAVATAR_DIR/src/bin/travatar" if not $TRAVATAR;

if(@ARGV != 0) {
    print STDERR "Usage: $0 -travatar-dir /path/to/travatar -working-dir /path/to/workingdir -src src.txt -ref ref.txt -travatar-config initial-travatar.ini\n";
    exit 1;
}

# Make the working directory and filter the mode
safesystem("mkdir $WORKING_DIR") or die "couldn't mkdir";
if($NO_FILTER_RT) {
    safesystem("cp $TRAVATAR_CONFIG $WORKING_DIR/run1.ini");
} else {
    safesystem("$TRAVATAR_DIR/script/train/filter-model.pl $TRAVATAR_CONFIG $WORKING_DIR/run1.ini $WORKING_DIR/filtered \"$TRAVATAR_DIR/script/train/filter-rt.pl -src $SRC -src-format $IN_FORMAT\"") or die "Couldn't filter";
}

# Find the weights contained in the model
my %init_weights = load_weights($TRAVATAR_CONFIG);
my $weight_cnt = keys %init_weights;
die "Couldn't find any weights in the model" if not $weight_cnt;
# `Print these if needed for batch-tune
if($MERT_SOLVER eq "batch-tune") {
    open FILE0, ">:utf8", "$WORKING_DIR/run1.weights" or die "Couldn't open $WORKING_DIR/run1.weights\n";
    while(my ($k,$v) = each(%init_weights)) { print FILE0 "$k=$v\n"; }
    close FILE0;
}

# Do the outer loop
my ($iter, $prev, $next);
foreach $iter (1 .. $MAX_ITERS) {
    $prev = "$WORKING_DIR/run$iter";
    $next = "$WORKING_DIR/run".($iter+1);
    # Candidate options
    my $CAND_OPTIONS;
    if($CAND_TYPE =~ "^nbest") { $CAND_OPTIONS = "-nbest $NBEST -nbest_out $prev.nbest"; }
    elsif($CAND_TYPE eq "forest") { $CAND_OPTIONS = "-forest_out $prev.forest -forest_nbest_trim $NBEST"; }
    # Do the decoding
    safesystem("$TRAVATAR -threads $THREADS -in_format $IN_FORMAT -trg_factors $TRG_FACTORS -config_file $prev.ini $DECODER_OPTIONS $CAND_OPTIONS  < $SRC > $prev.out 2> $prev.err") or die "couldn't decode";
    safesystem("cp $prev.out $WORKING_DIR/last.out") or die "couldn't copy to last.out";
    safesystem("cp $prev.ini $WORKING_DIR/last.ini") or die "couldn't copy to last.out";
    if($MERT_SOLVER eq "moses") {
        safesystem("$TRAVATAR_DIR/script/mert/densify-nbest.pl $prev.ini < $prev.nbest > $prev.nbest-dense") or die "couldn't densify";
        safesystem("$MOSES_DIR/bin/extractor --scconfig case:true --scfile $prev.scores.dat --ffile $prev.features.dat -r $REF -n $prev.nbest-dense") or die "couldn't extract";
        safesystem("$TRAVATAR_DIR/script/mert/make-init-opt.pl < $prev.ini > $prev.init.opt") or die "couldn't make init opt";
        my $feats = join(",", map { "$WORKING_DIR/run$_.features.dat" } (1 .. $iter));
        my $scores = join(",", map { "$WORKING_DIR/run$_.scores.dat" } (1 .. $iter));
        safesystem("$MOSES_DIR/bin/mert -d $weight_cnt --scconfig case:true --scfile $scores --ffile $feats --ifile $prev.init.opt -n 20 > $prev.mert.out 2> $prev.mert.log") or die "couldn't mert"; 
        print `grep Best $prev.mert.log`;
        safesystem("$TRAVATAR_DIR/script/mert/update-weights.pl -log $prev.mert.log $prev.ini > $next.ini") or die "couldn't make init opt";
    } elsif($MERT_SOLVER eq "batch-tune") {
        if($CAND_TYPE eq "nbest") {
            my $nbests = join(" ", map { "$WORKING_DIR/run$_.uniq" } (1 .. $iter-1));
            safesystem("$TRAVATAR_DIR/script/mert/nbest-uniq.pl $nbests < $WORKING_DIR/run$iter.nbest > $WORKING_DIR/run$iter.uniq");
            safesystem("$TRAVATAR_DIR/src/bin/batch-tune -threads $THREADS -nbest $WORKING_DIR/run$iter.uniq -stat_out $WORKING_DIR/run$iter.stats -eval \"$EVAL\" $REF 2> $prev.stats.log") or die "batch-tune stats extraction failed";
            $nbests = join(",", map { "$WORKING_DIR/run$_.uniq" } (1 .. $iter));
            my $stats = join(",", map { "$WORKING_DIR/run$_.stats" } (1 .. $iter));
            safesystem("$TRAVATAR_DIR/src/bin/batch-tune -threads $THREADS -nbest $nbests -stat_in $stats -eval \"$EVAL\" -weight_in $prev.weights $TUNE_OPTIONS $REF > $next.weights 2> $prev.tune.log") or die "batch-tune failed";
        } elsif($CAND_TYPE eq "forest") {
            my $forests = join(",", map { "$WORKING_DIR/run$_.forest" } (1 .. $iter));
            safesystem("$TRAVATAR_DIR/src/bin/batch-tune -threads $THREADS -forest $forests -eval \"$EVAL\" -weight_in $prev.weights $TUNE_OPTIONS $REF > $next.weights 2> $prev.tune.log") or die "batch-tune failed";
        } elsif ($CAND_TYPE eq "nbestgeo") {
            my $nbests = join(" ", map { "$WORKING_DIR/run$_.uniq" } (1 .. $iter-1));
            safesystem("$TRAVATAR_DIR/script/mert/nbest-uniq.pl $nbests < $WORKING_DIR/run$iter.nbest > $WORKING_DIR/run$iter.uniq");
            safesystem("$GEOTUNE_COMMAND -prefix $WORKING_DIR/run$iter -ref $REF -output $WORKING_DIR/run$iter.stats");
            $nbests = join(",", map { "$WORKING_DIR/run$_.uniq" } (1 .. $iter));
            my $stats = join(",", map { "$WORKING_DIR/run$_.stats" } (1 .. $iter));
            safesystem("$TRAVATAR_DIR/src/bin/batch-tune -threads $THREADS -nbest $nbests -stat_in $stats -eval \"ribes\" -weight_in $prev.weights $TUNE_OPTIONS $REF > $next.weights 2> $prev.tune.log") or die "batch-tune failed";

		}
        safesystem("$TRAVATAR_DIR/script/mert/update-weights.pl -weights $next.weights $prev.ini > $next.ini") or die "couldn't make init opt";
    }
    my %wprev = load_weights("$prev.ini");
    my %wnext = load_weights("$next.ini");
    my $diff = 0;
    for(keys %wprev) { $diff += abs($wprev{$_} - $wnext{$_}); }
    last if($diff < $MIN_DIFF);
}

safesystem("$TRAVATAR_DIR/script/mert/update-weights.pl -model $next.ini $TRAVATAR_CONFIG > $WORKING_DIR/travatar.ini") or die "couldn't make init opt";

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

# Load weights
sub load_weights {
    my $fname = shift;
    open FILE0, "<:utf8", $fname or die "Couldn't open $fname\n";
    my %ret;
    while(<FILE0>) {
        chomp;
        if(/^\[weight_vals\]$/) {
            while(<FILE0>) {
                chomp;
                last if not $_;
                my ($k, $v) = split(/=/);
                $ret{$k} = $v;
            }
            last;
        }
    }
    close FILE0;
    return %ret;
}
