#!/usr/bin/perl
##############################################################################
# ja-en preprocessing script
#  by Graham Neubig
#
# This is a preprocessing script for Japanese-English machine translation,
# specifically using Travatar. It can be used by running
#   $ preprocess-ja-en.pl INPUT_JA INPUT_EN OUTPUT_DIR
#
# Where INPUT_JA and INPUT_EN are Japanese and English files of parallel
# sentences, and OUTPUT_DIR is the directory where you will put the output.
#
# The full battery of settings can be found below, and you will likely have
# to change some paths to make it work in your environment. Common settings
# for training and testing data will include:
#
# Training (run with multiple threads, remove long sentences, and align):
# $ preprocess-ja-en.pl JA EN OUT -threads 10 -clean-len 60 -align
#
# Testing (multiple threads, make forests):
# $ preprocess-ja-en.pl JA EN OUT -threads 10 -forest
#
# If you want to resume a partially finished preprocessing run, you can do so
# by deleting any files that have failed or were still in progress (including
# both the file itself and the file with the ".DONE" prefix and re-running the
# same command.
#
# Before running the script you should install:
#  Stanford Parser: http://nlp.stanford.edu/software/lex-parser.shtml
#  Egret:  http://code.google.com/p/egret-parser/
#  KyTea:  http://www.phontron.com/kytea/
#  Eda:    http://plata.ar.media.kyoto-u.ac.jp/tool/EDA/home_en.html
#  GIZA++: http://code.google.com/p/giza-pp/
#  Nile:   http://code.google.com/p/nile/
#
# If you install KyTea using "make install" and install all other tools to
# ~/usr/local and remove version numbers from the director names, and move
# the Stanford Parser model file from stanford-parser-models-VERSION.jar to
# stanford-parser-models.jar, this script can be run without any extra path
# settings. Otherwise, you will need to specify the paths using the command line

use Env;

############### Settings ##################
my $THREADS = 1;
my $CLEAN_LEN = 60;

# Directories and settings
my $PROGRAM_DIR = $ENV{"HOME"}."/usr/local";
my $TRAVATAR_DIR;
my $STANFORD_DIR;
my $STANFORD_JARS;
my $EGRET_DIR;
my $EGRET_FOREST_OPT = "-nbest4threshold=100";
my $EDA_DIR;
my $EDA_VOCAB;
my $EDA_WEIGHT;
my $GIZA_DIR;
my $KYTEA = "kytea";
my $FOREST;
my $ALIGN;

############## Definitions ################
use strict;
use warnings;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
use Cwd 'abs_path';
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

GetOptions(
    "program-dir=s" => \$PROGRAM_DIR,
    "threads=s" => \$THREADS,
    "clean-len=i" => \$CLEAN_LEN,
    "kytea=s" => \$KYTEA,
    "stanford-jars=s" => \$STANFORD_JARS,
    "eda-dir=s" => \$EDA_DIR,
    "eda-vocab=s" => \$EDA_VOCAB,
    "eda-weight=s" => \$EDA_WEIGHT,
    "giza-dir=s" => \$GIZA_DIR,
    "egret-forest-opt=s" => \$EGRET_FOREST_OPT,
    "travatar-dir=s" => \$TRAVATAR_DIR,
    "forest" => \$FOREST,
    "align" => \$ALIGN,
);
$STANFORD_DIR = "$PROGRAM_DIR/stanford-parser" if not $STANFORD_DIR;
$STANFORD_JARS = "$STANFORD_DIR/stanford-parser.jar:$STANFORD_DIR/stanford-parser-models.jar" if not $STANFORD_JARS;
$EDA_DIR = "$PROGRAM_DIR/eda" if not $EDA_DIR;
$EGRET_DIR = "$PROGRAM_DIR/Egret" if not $EGRET_DIR;
$EDA_VOCAB = "$EDA_DIR/data/jp-0.1.0-utf8-vocab-small.dat" if not $EDA_VOCAB;
$EDA_WEIGHT = "$EDA_DIR/data/jp-0.1.0-utf8-weight-small.dat" if not $EDA_WEIGHT;
$GIZA_DIR = "$PROGRAM_DIR/giza-pp" if not $GIZA_DIR;

if(not $TRAVATAR_DIR) {
    $TRAVATAR_DIR = abs_path($0);
    $TRAVATAR_DIR =~ s!/script/preprocess/.*\.pl!!g;
}

# Sanity check of input
if(@ARGV != 3) {
    print STDERR "Usage: $0 INPUT_JA INPUT_EN OUTPUT_DIR\n";
    exit 1;
}
(-f $ARGV[0]) or die "Japanese file $ARGV[0] doesn't exist.";
(-f $ARGV[1]) or die "English file $ARGV[1] doesn't exist.";
my ($JAORIG, $ENORIG, $PREF) = @ARGV;
-e "$PREF" or mkdir "$PREF";

###### Split the input files #######
if(-e "$PREF/orig") {
    wait_done("$PREF/orig.DONE");
} else {
    -e "$PREF/orig" or mkdir "$PREF/orig";
    -e "$PREF/orig/ja" or safesystem("ln -s $ARGV[0] $PREF/orig/ja");
    -e "$PREF/orig/en" or safesystem("ln -s $ARGV[1] $PREF/orig/en");
    my $JALEN = file_len($ARGV[0]);
    my $ENLEN = file_len($ARGV[1]);
    die "ja and en lengths don't match ($JALEN != $ENLEN)" if ($JALEN != $ENLEN);
    my $len = int($JALEN/$THREADS+1);
    my $suflen = int(log($THREADS)/log(10)+1);
    safesystem("split -l $len -a $suflen -d $JAORIG $PREF/orig/ja.") or die;
    safesystem("split -l $len -a $suflen -d $ENORIG $PREF/orig/en.") or die;
    safesystem("touch $PREF/orig.DONE") or die;
}
my @suffixes;
foreach my $f (`ls $PREF/orig`) {
    chomp $f;
    if($f =~ /ja\.(.*)/) {
        push @suffixes, $1;
    }
}

###### Tokenization and Lowercasing #######
# JA
run_parallel("$PREF/orig", "$PREF/tok", "ja", "$KYTEA -notags -wsconst D INFILE > OUTFILE");

# EN
run_parallel("$PREF/orig", "$PREF/tok", "en", "java -cp $STANFORD_JARS edu.stanford.nlp.process.PTBTokenizer -preserveLines INFILE | sed \"s/(/-LRB-/g; s/)/-RRB-/g\" > OUTFILE");

###### Cleaning #######
if($CLEAN_LEN == 0) {
    safesystem("ln -s $PREF/tok $PREF/clean");
} elsif (not -e "$PREF/clean/ja") {
    -e "$PREF/clean" or mkdir "$PREF/clean";
    foreach my $i (@suffixes) {
        safesystem("bash -c '$TRAVATAR_DIR/script/train/clean-corpus.pl -max_len $CLEAN_LEN $PREF/tok/ja.$i $PREF/tok/en.$i $PREF/clean/ja.$i $PREF/clean/en.$i; touch $PREF/clean/ja.$i.DONE' &")  if not -e "$PREF/clean/ja.$i";
    }
    wait_done(map { "$PREF/clean/ja.$_.DONE" } @suffixes);
    safesystem("cat ".join(" ", map { "$PREF/clean/ja.$_" } @suffixes)." > $PREF/clean/ja");
    safesystem("cat ".join(" ", map { "$PREF/clean/en.$_" } @suffixes)." > $PREF/clean/en");
}
run_parallel("$PREF/clean", "$PREF/low", "ja", "$TRAVATAR_DIR/script/tree/lowercase.pl < INFILE > OUTFILE");
run_parallel("$PREF/clean", "$PREF/low", "en", "$TRAVATAR_DIR/script/tree/lowercase.pl < INFILE > OUTFILE");

###### 1-best Parsing ######
# EN Parsing with Stanford Parser
run_parallel("$PREF/clean", "$PREF/stanford", "en", "java -mx2000m -cp $STANFORD_JARS edu.stanford.nlp.parser.lexparser.LexicalizedParser -tokenized -sentences newline -outputFormat oneline edu/stanford/nlp/models/lexparser/englishPCFG.ser.gz INFILE 2> OUTFILE.log > OUTFILE");

# EN Parsing with Egret
run_parallel("$PREF/clean", "$PREF/egret", "en", "$EGRET_DIR/egret -lapcfg -i=INFILE -data=$EGRET_DIR/eng_grammar 2> OUTFILE.log > OUTFILE");

# EN Combine Stanford and Egret (for now this is not parallel)
-e "$PREF/tree" or mkdir "$PREF/tree";
foreach my $i ("", map{".$_"} @suffixes) {
    safesystem("$TRAVATAR_DIR/script/tree/replace-failed-parse.pl $PREF/stanford/en$i $PREF/egret/en$i > $PREF/tree/en$i") if not -e "$PREF/tree/en$i";
    die "Combining trees failed on en$i" if(file_len("$PREF/stanford/en$i") != file_len("$PREF/tree/en$i"));
}

# EN Lowercase the trees
run_parallel("$PREF/tree", "$PREF/treelow", "en", "$TRAVATAR_DIR/script/tree/lowercase.pl < INFILE > OUTFILE");

# JA Parsing with Eda
run_parallel("$PREF/clean", "$PREF/edain", "ja", "cat INFILE | $TRAVATAR_DIR/script/tree/han2zen.pl -nospace -remtab | $KYTEA -in tok -out eda > OUTFILE", 1);
run_parallel("$PREF/edain", "$PREF/eda", "ja", "$EDA_DIR/src/eda/eda -e INFILE -v $EDA_VOCAB -w $EDA_WEIGHT > OUTFILE");
run_parallel("$PREF/eda", "$PREF/tree", "ja", "cat INFILE | $TRAVATAR_DIR/script/tree/ja-adjust-dep.pl | $TRAVATAR_DIR/script/tree/ja-dep2cfg.pl > OUTFILE", 1);
run_parallel("$PREF/tree", "$PREF/treelow", "ja", "$TRAVATAR_DIR/script/tree/lowercase.pl < INFILE > OUTFILE");

###### Forest Parsing ######

if($FOREST) {

# EN Convert Stanford to Forest
run_parallel("$PREF/stanford", "$PREF/stanfordfor", "en", "$TRAVATAR_DIR/src/bin/tree-converter -input_format penn -output_format egret < INFILE 2> OUTFILE.log > OUTFILE", 1);

# EN Parse with Egret
run_parallel("$PREF/clean", "$PREF/egretfor", "en", "$EGRET_DIR/egret -lapcfg -i=INFILE -printForest $EGRET_FOREST_OPT -data=$EGRET_DIR/eng_grammar 2> OUTFILE.log | sed \"s/\\^g//g\" > OUTFILE", 1);

# EN Combine Stanford and Egret (for now this is not parallel)
-e "$PREF/for" or mkdir "$PREF/for";
foreach my $i ("", map{".$_"} @suffixes) {
    safesystem("$TRAVATAR_DIR/script/tree/replace-failed-parse.pl -format egret $PREF/stanfordfor/en$i $PREF/egretfor/en$i > $PREF/for/en$i") if not -e "$PREF/for/en$i";
    die "Combining forests failed on en$i" if(file_len("$PREF/for/en$i") == 0);
}

# Combine and lowercase the forest
run_parallel("$PREF/for", "$PREF/forlow", "en", "$TRAVATAR_DIR/script/tree/lowercase.pl < INFILE > OUTFILE");

} # End if ($FOREST)

###### Alignment ######

# Run the training script
if($ALIGN) {

    # Align using GIZA++
    safesystem("$TRAVATAR_DIR/script/train/train-travatar.pl -last_step lex -work_dir $PREF/train -no_lm true -src_words $PREF/low/ja -src_file $PREF/treelow/ja -trg_file $PREF/low/en -travatar_dir $TRAVATAR_DIR -bin_dir $GIZA_DIR -threads $THREADS > $PREF/train.log") if not -e "$PREF/train";
    safesystem("mkdir $PREF/giza") if not -e "$PREF/giza";
    safesystem("cp $PREF/train/align/align.txt $PREF/giza/jaen") if not -e "$PREF/jaen";
    safesystem("cat $PREF/train/align/align.txt | sed \"s/\\([0-9][0-9]*\\)-\\([0-9][0-9]*\\)/\\2-\\1/g\" > $PREF/giza/enja") if not -e "$PREF/enja";

    # TODO: Align using Nile

} # End if ($ALIGN)


###################### Auxiliary Functions ##########################

# Adapted from Moses's train-model.perl
sub safesystem {
  print STDERR "Executing: @_\n";
  system(@_);
  if ($? == -1) {
      print STDERR "ERROR: Failed to execute: @_\n  $!\n";
      exit(1);
  }
  elsif ($? & 127) {
      printf STDERR "ERROR: Execution of: @_\n  died with signal %d, %s coredump\n",
          ($? & 127),  ($? & 128) ? 'with' : 'without';
      exit(1);
  }
  else {
    my $exitcode = $? >> 8;
    print STDERR "Exit code: $exitcode\n" if $exitcode;
    return ! $exitcode;
  }
}

# Run a command in parallel over several files
sub run_parallel {
    my ($in_dir, $out_dir, $prefix, $cmd, $nocheck) = @_;
    return if -e "$out_dir/$prefix.DONE";
    my @files = map { "$prefix.$_" } @suffixes;
    -e $out_dir or mkdir $out_dir;
    foreach my $f (@files) {
        my $my_cmd = $cmd;
        $my_cmd =~ s/INFILE/$in_dir\/$f/g;
        $my_cmd =~ s/OUTFILE/$out_dir\/$f/g;
        safesystem("bash -c '$my_cmd; touch $out_dir/$f.DONE' &") if not -e "$out_dir/$f";
    }
    wait_done(map { "$out_dir/$_.DONE" } @files);
    safesystem("cat ".join(" ", map { "$out_dir/$_" } @files)." > $out_dir/$prefix");
    my $old_lines = file_len("$in_dir/$prefix");
    my $new_lines = file_len("$out_dir/$prefix");
    die "ERROR: while creating $out_dir/$prefix in parallel\nFile sizes don't match: $in_dir/$prefix=$old_lines, $out_dir/$prefix=$new_lines" if (not $nocheck) and ($old_lines != $new_lines);
    safesystem("touch $out_dir/$prefix.DONE");
    print "Successfully created $out_dir/$prefix\n";
}

# Wait for files
sub wait_done {
    foreach my $f (@_) {
        print "Waiting on $f\n" if(not -e $f);
        while(not -e $f) {
            sleep 1;
        }
    }
}

# Get a file's length in lines
my %file_lens;
sub file_len {
    my $f = shift;
    return $file_lens{$f} if defined $file_lens{$f};
    my $len = `wc -l $f`; $len =~ s/[ \n].*//g;
    $file_lens{$f} = $len;
    return $len;
}
