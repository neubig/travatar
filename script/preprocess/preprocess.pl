#!/usr/bin/perl
##############################################################################
# Preprocessing script for Travatar
#  by Graham Neubig
#
# This is a preprocessing script for machine translation,
# specifically using Travatar. It can be used by running
#   $ preprocess.pl -src SRC -trg TRG INPUT_SRC INPUT_TRG OUTPUT_DIR
#
# Where INPUT_SRC and INPUT_TRG are source and target files of parallel
# sentences, and OUTPUT_DIR is the directory where you will put the output.
#
# The full battery of settings can be found below, and you will likely have
# to change some paths to make it work in your environment. Common settings
# for training and testing data will include:
#
# Training (run with multiple threads, remove long sentences, and align):
# $ preprocess.pl IN_SRC IN_TRG OUT -threads 10 -clean-len 60 -align
#
# Testing (multiple threads, make forests):
# $ preprocess.pl IN_SRC IN_TRG OUT -threads 10 -forest-src
#
# If you want to resume a partially finished preprocessing run, you can do so
# by deleting any files that have failed or were still in progress (including
# both the file itself and the file with the ".DONE" prefix and re-running the
# same command.
#
# Before running the script you should install:
#
# English:
#  Stanford Parser: http://nlp.stanford.edu/software/lex-parser.shtml
#  Egret:  http://code.google.com/p/egret-parser/
#
# Japanese:
#  KyTea:  http://www.phontron.com/kytea/
#  Eda:    http://plata.ar.media.kyoto-u.ac.jp/tool/EDA/home_en.html
#
# Chinese:
#  Stanford Segmenter: http://nlp.stanford.edu/software/segmenter.shtml
#  Stanford Parser: http://nlp.stanford.edu/software/lex-parser.shtml
#  Egret:  http://code.google.com/p/egret-parser/
#
# Aligner:
#  GIZA++: http://code.google.com/p/giza-pp/
#  Nile:   http://code.google.com/p/nile/
#
# If you install KyTea using "make install" and install all other tools to
# ~/usr/local and remove version numbers from the directory names, and move
# the Stanford Parser model file from stanford-parser-models-VERSION.jar to
# stanford-parser-models.jar, this script can be run without any extra path
# settings. Otherwise, you will need to specify the paths using the command line

use Env;

############### Settings ##################
my $THREADS = 1;
my $CLEAN_LEN = 0;

# Directories and settings
my $PROGRAM_DIR = $ENV{"HOME"}."/usr/local";
my $TRAVATAR_DIR;
my $STANFORD_DIR;
my $STANFORD_SEG_DIR;
my $STANFORD_POS_DIR;
my $STANFORD_JARS;
my $EGRET_DIR;
my $EGRET_FOREST_OPT = "-nbest4threshold=100";
my $SPLIT_WORDS_SRC;
my $SPLIT_WORDS_TRG;
my $EDA_DIR;
my $EDA_VOCAB;
my $EDA_WEIGHT;
my $GIZA_DIR;
my $NILE_DIR;
my $NILE_MODEL;
my $NILE_SEGMENTS = 1000;
my $NILE_BEAM = 64;
my $KYTEA = "kytea";
my $FOREST_SRC;
my $FOREST_TRG;
my $ALIGN;
my $SRC = "ja";
my $TRG = "en";

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
    "nile-dir=s" => \$NILE_DIR,
    "nile-model=s" => \$NILE_MODEL,
    "nile-beam=i" => \$NILE_BEAM,
    "nile-segments=i" => \$NILE_SEGMENTS,
    "egret-forest-opt=s" => \$EGRET_FOREST_OPT,
    "split-words-src=s" => \$SPLIT_WORDS_SRC,
    "split-words-trg=s" => \$SPLIT_WORDS_TRG,
    "travatar-dir=s" => \$TRAVATAR_DIR,
    "forest-src" => \$FOREST_SRC,
    "forest-trg" => \$FOREST_TRG,
    "align" => \$ALIGN,
    "src=s" => \$SRC,
    "trg=s" => \$TRG,
);
$STANFORD_DIR = "$PROGRAM_DIR/stanford-parser" if not $STANFORD_DIR;
$STANFORD_SEG_DIR = "$PROGRAM_DIR/stanford-segmenter" if not $STANFORD_SEG_DIR;
$STANFORD_POS_DIR = "$PROGRAM_DIR/stanford-postagger" if not $STANFORD_POS_DIR;
$STANFORD_JARS = "$STANFORD_DIR/stanford-parser.jar:$STANFORD_DIR/stanford-parser-models.jar" if not $STANFORD_JARS;
$EDA_DIR = "$PROGRAM_DIR/eda" if not $EDA_DIR;
$EGRET_DIR = "$PROGRAM_DIR/Egret" if not $EGRET_DIR;
$EDA_VOCAB = "$EDA_DIR/data/jp-0.1.0-utf8-vocab-small.dat" if not $EDA_VOCAB;
$EDA_WEIGHT = "$EDA_DIR/data/jp-0.1.0-utf8-weight-small.dat" if not $EDA_WEIGHT;
$GIZA_DIR = "$PROGRAM_DIR/giza-pp" if not $GIZA_DIR;
$NILE_DIR = "$PROGRAM_DIR/nile" if not $NILE_DIR;

if(not $TRAVATAR_DIR) {
    $TRAVATAR_DIR = abs_path($0);
    $TRAVATAR_DIR =~ s!/script/preprocess/.*\.pl!!g;
}

# Sanity check of input
if(@ARGV != 3) {
    print STDERR "Usage: $0 INPUT_SRC INPUT_TRG OUTPUT_DIR\n";
    exit 1;
}
(-f $ARGV[0]) or die "$SRC file $ARGV[0] doesn't exist.";
(-f $ARGV[1]) or die "$TRG file $ARGV[1] doesn't exist.";
my ($SRCORIG, $TRGORIG, $PREF) = @ARGV;
-e "$PREF" or mkdir "$PREF";

###### Get and check the lengths #######
my $SRCLEN = file_len($ARGV[0]);
my $TRGLEN = file_len($ARGV[1]);
die "$SRC and $TRG lengths don't match ($SRCLEN != $TRGLEN)" if ($SRCLEN != $TRGLEN);
my $len = int($SRCLEN/$THREADS+1);
my $suflen = int(log($THREADS)/log(10)+1);

###### Split the input files #######
if(-e "$PREF/orig") {
    wait_done("$PREF/orig.DONE");
} else {
    -e "$PREF/orig" or mkdir "$PREF/orig";
    -e "$PREF/orig/$SRC" or safesystem("ln -s ".abs_path($ARGV[0])." $PREF/orig/$SRC");
    -e "$PREF/orig/$TRG" or safesystem("ln -s ".abs_path($ARGV[1])." $PREF/orig/$TRG");
    safesystem("split -l $len -a $suflen -d $SRCORIG $PREF/orig/$SRC.") or die;
    safesystem("split -l $len -a $suflen -d $TRGORIG $PREF/orig/$TRG.") or die;
    safesystem("touch $PREF/orig.DONE") or die;
}
my @suffixes;
foreach my $f (`ls $PREF/orig`) {
    chomp $f;
    if($f =~ /$SRC\.(.*)/) {
        push @suffixes, $1;
    }
}

###### Tokenization and Lowercasing #######
sub tokenize_cmd {
    if($_[0] eq "en") { return "java -cp $STANFORD_JARS edu.stanford.nlp.process.PTBTokenizer -preserveLines INFILE | sed \"s/(/-LRB-/g; s/)/-RRB-/g; s/[\t ]+/ /g; s/^ +//g; s/ +\$//g\" > OUTFILE"; }
    elsif($_[0] eq "fr") { return "java -cp $STANFORD_JARS edu.stanford.nlp.process.PTBTokenizer -options asciiQuotes -preserveLines INFILE | sed \"s/(/-LRB-/g; s/)/-RRB-/g; s/[\t ]+/ /g; s/^ +//g; s/ +\$//g\" > OUTFILE"; }
    elsif($_[0] eq "ja") { return "$KYTEA -notags -wsconst D INFILE | sed \"s/[\t ]+/ /g; s/^ +//g; s/ +\$//g\" > OUTFILE"; }
    elsif($_[0] eq "zh") { return "$STANFORD_SEG_DIR/segment.sh ctb INFILE UTF-8 0 | sed \"s/(/-LRB-/g; s/)/-RRB-/g; s/[\t ]+/ /g; s/^ +//g; s/ +\$//g\" > OUTFILE"; }
    else { die "Cannot tokenize $_[0]"; }
}
run_parallel("$PREF/orig", "$PREF/tok", $SRC, tokenize_cmd($SRC));
run_parallel("$PREF/orig", "$PREF/tok", $TRG, tokenize_cmd($TRG));

###### Cleaning #######
if($CLEAN_LEN == 0) {
    safesystem("ln -s ".abs_path("$PREF/tok")." $PREF/clean") if not -e "$PREF/clean";
} elsif (not -e "$PREF/clean/$SRC") {
    -e "$PREF/clean" or mkdir "$PREF/clean";
    foreach my $i (@suffixes) {
        safesystem("bash -c '$TRAVATAR_DIR/script/train/clean-corpus.pl -max_len $CLEAN_LEN $PREF/tok/$SRC.$i $PREF/tok/$TRG.$i $PREF/clean/$SRC.$i $PREF/clean/$TRG.$i; touch $PREF/clean/$SRC.$i.DONE' &")  if not -e "$PREF/clean/$SRC.$i";
    }
    wait_done(map { "$PREF/clean/$SRC.$_.DONE" } @suffixes);
    safesystem("cat ".join(" ", map { "$PREF/clean/$SRC.$_" } @suffixes)." > $PREF/clean/$SRC");
    safesystem("cat ".join(" ", map { "$PREF/clean/$TRG.$_" } @suffixes)." > $PREF/clean/$TRG");
}

###### 1-best Parsing ######
sub run_tree_parsing {
    my $lang = shift;
    my $split_words = shift;
    if($lang =~ /^(en|zh)$/) {
        if($lang eq "en") {
            # EN Parsing with Stanford Parser
            run_parallel("$PREF/clean", "$PREF/stanford", $lang, "java -mx2000m -cp $STANFORD_JARS edu.stanford.nlp.parser.lexparser.LexicalizedParser -tokenized -sentences newline -outputFormat oneline edu/stanford/nlp/models/lexparser/englishPCFG.ser.gz INFILE 2> OUTFILE.log > OUTFILE");
    
            # EN Parsing with Egret
            run_parallel("$PREF/clean", "$PREF/egret", $lang, "$EGRET_DIR/egret -lapcfg -i=INFILE -data=$EGRET_DIR/eng_grammar 2> OUTFILE.log > OUTFILE");
        } else {

            # ZH Convert to GB encoding, same as the penn treebank
            run_parallel("$PREF/clean", "$PREF/cleangb", $lang, "iconv -sc -f UTF-8 -t GB18030 < INFILE | sed \"s/^[ \t]*\$/FAILED/g\" > OUTFILE");

            # ZH Parsing with Stanford Parser
            run_parallel("$PREF/cleangb", "$PREF/stanford", $lang, "java -mx2000m -cp $STANFORD_JARS edu.stanford.nlp.parser.lexparser.LexicalizedParser -tLPP \"edu.stanford.nlp.parser.lexparser.ChineseTreebankParserParams\" -chineseFactored -encoding GB18030 -tokenized -sentences newline -outputFormat oneline edu/stanford/nlp/models/lexparser/chinesePCFG.ser.gz INFILE 2> OUTFILE.log | iconv -f GB18030 -t UTF-8 > OUTFILE");
    
            # ZH Parsing with Egret
            run_parallel("$PREF/cleangb", "$PREF/egret", $lang, "$EGRET_DIR/egret -lapcfg -i=INFILE -data=$EGRET_DIR/chn_grammar 2> OUTFILE.log | iconv -f GB18030 -t UTF-8 > OUTFILE");
        }
    
        # Combine Stanford and Egret (for now this is not parallel)
        -e "$PREF/tree" or mkdir "$PREF/tree";
        foreach my $i ("", map{".$_"} @suffixes) {
            if(not -e "$PREF/tree/$lang$i") {
                my $SPLIT_CMD = "";
                $SPLIT_CMD = "| $TRAVATAR_DIR/src/bin/tree-converter -split \"$split_words\"" if $split_words;
                safesystem("$TRAVATAR_DIR/script/tree/replace-failed-parse.pl $PREF/stanford/$lang$i $PREF/egret/$lang$i $SPLIT_CMD > $PREF/tree/$lang$i") ;
                die "Combining trees failed on $lang$i" if(file_len("$PREF/stanford/$lang$i") != file_len("$PREF/tree/$lang$i"));
            }
        }
    } elsif($lang eq "ja") {
        # JA Parsing with Eda
        run_parallel("$PREF/clean", "$PREF/edain", $lang, "cat INFILE | $TRAVATAR_DIR/script/tree/han2zen.pl -nospace -remtab | $KYTEA -in tok -out eda > OUTFILE", 1);
        run_parallel("$PREF/edain", "$PREF/eda", $lang, "$EDA_DIR/src/eda/eda -e INFILE -v $EDA_VOCAB -w $EDA_WEIGHT > OUTFILE");
        run_parallel("$PREF/eda", "$PREF/tree", $lang, "cat INFILE | $TRAVATAR_DIR/script/tree/ja-adjust-dep.pl | $TRAVATAR_DIR/script/tree/ja-dep2cfg.pl > OUTFILE", 1);
    } elsif($lang =~ /^(fr)$/) {
        my $SPLIT_CMD = "";
        $SPLIT_CMD = "| $TRAVATAR_DIR/src/bin/tree-converter -split \"$split_words\"" if $split_words;
        run_parallel("$PREF/clean", "$PREF/tree", $lang, "java -mx2000m -cp $STANFORD_JARS edu.stanford.nlp.parser.lexparser.LexicalizedParser -tokenized -sentences newline -outputFormat oneline edu/stanford/nlp/models/lexparser/frenchFactored.ser.gz INFILE 2> OUTFILE.log $SPLIT_CMD > OUTFILE");
    } else { die "Cannot parse $lang"; }

    # Lowercase and print
    run_parallel("$PREF/tree", "$PREF/treelow", $lang, "$TRAVATAR_DIR/script/tree/lowercase.pl < INFILE > OUTFILE");
    run_parallel("$PREF/treelow", "$PREF/low", $lang, "$TRAVATAR_DIR/src/bin/tree-converter -output_format word < INFILE > OUTFILE");
}
run_tree_parsing($SRC, $SPLIT_WORDS_SRC);
run_tree_parsing($TRG, $SPLIT_WORDS_TRG);

###### Forest Parsing ######

sub run_forest_parsing {
    my $lang = shift;
    my $split_words = shift;
    if($lang =~ /^(en|zh)$/) {
        # Convert Tree to Forest
        run_parallel("$PREF/tree", "$PREF/treefor", $lang, "$TRAVATAR_DIR/src/bin/tree-converter -input_format penn -output_format egret < INFILE 2> OUTFILE.log > OUTFILE", 1);
        
        if($lang eq "en") {
            # EN Parse with Egret
            run_parallel("$PREF/clean", "$PREF/egretfor", $lang, "$EGRET_DIR/egret -lapcfg -i=INFILE -printForest $EGRET_FOREST_OPT -data=$EGRET_DIR/eng_grammar 2> OUTFILE.log | sed \"s/\\^g//g\" > OUTFILE", 1);
        } else {
            # ZH Parse with Egret
            run_parallel("$PREF/cleangb", "$PREF/egretfor", $lang, "$EGRET_DIR/egret -lapcfg -i=INFILE -printForest $EGRET_FOREST_OPT -data=$EGRET_DIR/chn_grammar 2> OUTFILE.log  | iconv -f GB18030 -t UTF-8 | sed \"s/\\^g//g\" > OUTFILE", 1);
        }
        
        # ZH Combine Stanford and Egret (for now this is not parallel)
        -e "$PREF/for" or mkdir "$PREF/for";
        foreach my $i ("", map{".$_"} @suffixes) {
            my $SPLIT_CMD = "";
            $SPLIT_CMD = "| $TRAVATAR_DIR/src/bin/tree-converter -split \"$split_words\" -input_format egret -output_format egret" if $split_words;
            safesystem("$TRAVATAR_DIR/script/tree/replace-failed-parse.pl -format egret $PREF/treefor/$lang$i $PREF/egretfor/$lang$i $SPLIT_CMD > $PREF/for/$lang$i") if not -e "$PREF/for/$lang$i";
            die "Combining forests failed on en$i" if(file_len("$PREF/for/$lang$i") == 0);
        }
    } else { die "Cannot forest parse $lang"; }

    # Combine and lowercase the forest
    run_parallel("$PREF/for", "$PREF/forlow", $lang, "$TRAVATAR_DIR/script/tree/lowercase.pl < INFILE > OUTFILE");
}
run_forest_parsing($SRC, $SPLIT_WORDS_SRC) if $FOREST_SRC;
run_forest_parsing($TRG, $SPLIT_WORDS_TRG) if $FOREST_TRG;

###### Alignment ######

# Run the training script
if($ALIGN) {

    # Align using GIZA++
    safesystem("$TRAVATAR_DIR/script/train/train-travatar.pl -last_step lex -work_dir $PREF/train -no_lm true -src_words $PREF/low/$SRC -src_file $PREF/treelow/$SRC -trg_file $PREF/low/$TRG -travatar_dir $TRAVATAR_DIR -bin_dir $GIZA_DIR -threads $THREADS > $PREF/train.log") if not -e "$PREF/train";
    safesystem("mkdir $PREF/giza") if not -e "$PREF/giza";
    safesystem("cp $PREF/train/align/align.txt $PREF/giza/$SRC$TRG") if not -e "$PREF/giza/$SRC$TRG";
    safesystem("cat $PREF/giza/$SRC$TRG | sed \"s/\\([0-9][0-9]*\\)-\\([0-9][0-9]*\\)/\\2-\\1/g\" > $PREF/giza/$TRG$SRC") if not -e "$PREF/giza/$TRG$SRC";

    # Run nile if a model is specified
    if($NILE_MODEL) {
        # Check file
        die "Could not find nile at $NILE_DIR/nile.py" if not -e "$NILE_DIR/nile.py";
        die "Could not find nile model $NILE_MODEL" if not -e "$NILE_MODEL";
        # Get the splits
        my $CLEANLEN = file_len("$PREF/low/$TRG");
        my $nilelen = int($CLEANLEN/$NILE_SEGMENTS+1);
        my $nilesuflen = int(log($NILE_SEGMENTS)/log(10)+1);
        my @nilesuffixes = map { sprintf("%0${nilesuflen}d", $_) } (0 .. $NILE_SEGMENTS-1);
        # Binarize the English trees
        run_parallel("$PREF/treelow", "$PREF/treelowbin", $TRG, "$TRAVATAR_DIR/src/bin/tree-converter -binarize right < INFILE > OUTFILE");
        # Create and split GIZA++ union alignments
        (safesystem("mkdir $PREF/gizau") or die) if not -e "$PREF/gizau";
        (safesystem("$TRAVATAR_DIR/script/train/symmetrize.pl -sym union $PREF/train/align/src-trg.giza.A3.final $PREF/train/align/trg-src.giza.A3.final > $PREF/gizau/$SRC$TRG") or die) if not -e "$PREF/gizau/$SRC$TRG";
        (safesystem("cat $PREF/gizau/$SRC$TRG | sed \"s/\\([0-9][0-9]*\\)-\\([0-9][0-9]*\\)/\\2-\\1/g\" > $PREF/gizau/$TRG$SRC") or die) if not -e "$PREF/gizau/$TRG$SRC";
        # Creating splits
        (safesystem("mkdir $PREF/nilein") or die) if not -e "$PREF/nilein";
        if(not -e "$PREF/nilein/low-$TRG.$nilesuffixes[0]") {
            safesystem("split -l $nilelen -a $nilesuflen -d $PREF/low/$TRG $PREF/nilein/low-$TRG.") or die;
            safesystem("split -l $nilelen -a $nilesuflen -d $PREF/low/$SRC $PREF/nilein/low-$SRC.") or die;
            # # Forward order
            # safesystem("split -l $nilelen -a $nilesuflen -d $PREF/giza/$SRC$TRG $PREF/nilein/giza.") or die;
            # safesystem("split -l $nilelen -a $nilesuflen -d $PREF/gizau/$SRC$TRG $PREF/nilein/gizau.") or die;
            # Reverse order
            safesystem("split -l $nilelen -a $nilesuflen -d $PREF/giza/$TRG$SRC $PREF/nilein/giza.") or die;
            safesystem("split -l $nilelen -a $nilesuflen -d $PREF/gizau/$TRG$SRC $PREF/nilein/gizau.") or die;
            safesystem("split -l $nilelen -a $nilesuflen -d $PREF/treelow/$SRC $PREF/nilein/tree-$SRC.") or die;
            safesystem("split -l $nilelen -a $nilesuflen -d $PREF/treelowbin/$TRG $PREF/nilein/tree-$TRG.") or die;
            foreach my $f (@nilesuffixes) {
                safesystem("$NILE_DIR/prepare-vocab.py < $PREF/nilein/low-$TRG.$f > $PREF/nilein/vcb-$TRG.$f");
                safesystem("$NILE_DIR/prepare-vocab.py < $PREF/nilein/low-$SRC.$f > $PREF/nilein/vcb-$SRC.$f");
            }
            safesystem("touch $PREF/nilein/prep.DONE") or die;
        }
        wait_done("$PREF/nilein/prep.DONE");
        # Use Nile to create alignments for each segment
        (safesystem("mkdir $PREF/nile") or die) if not -e "$PREF/nile";
        foreach my $s (@nilesuffixes) {
            # # Do nile in forward order
            # if(not -e "$PREF/nile/$SRC$TRG.$s") {
            #     safesystem("touch $PREF/nile/$SRC$TRG.$s");
            #     safesystem("mpiexec -n $THREADS python $NILE_DIR/nile.py --f $PREF/nilein/low-$SRC.$s --e $PREF/nilein/low-$TRG.$s --etrees $PREF/nilein/tree-$TRG.$s --ftrees $PREF/nilein/tree-$SRC.$s --evcb $PREF/nilein/vcb-$TRG.$s --fvcb $PREF/nilein/vcb-$SRC.$s --pef $PREF/train/lex/trg_given_src.lex --pfe $PREF/train/lex/src_given_trg.lex --a1 $PREF/nilein/giza.$s --a2 $PREF/nilein/gizau.$s --align --langpair ja_en --weights $NILE_MODEL --out $PREF/nile/$SRC$TRG.$s --k $NILE_BEAM") or die;
            #     safesystem("touch $PREF/nile/$SRC$TRG.$s.DONE");
            # }
            # Do nile in reverse order
            if(not -e "$PREF/nile/$TRG$SRC.$s") {
                safesystem("touch $PREF/nile/$TRG$SRC.$s");
                safesystem("mpiexec -n $THREADS python $NILE_DIR/nile.py --f $PREF/nilein/low-$TRG.$s --e $PREF/nilein/low-$SRC.$s --etrees $PREF/nilein/tree-$SRC.$s --ftrees $PREF/nilein/tree-$TRG.$s --evcb $PREF/nilein/vcb-$SRC.$s --fvcb $PREF/nilein/vcb-$TRG.$s --pef $PREF/train/lex/src_given_trg.lex --pfe $PREF/train/lex/trg_given_src.lex --a1 $PREF/nilein/giza.$s --a2 $PREF/nilein/gizau.$s --align --langpair ${SRC}_${TRG} --weights $NILE_MODEL --out $PREF/nile/$TRG$SRC.$s --k $NILE_BEAM") or die;
                safesystem("touch $PREF/nile/$TRG$SRC.$s.DONE");
            }
        }
        wait_done(map { "$PREF/nile/$TRG$SRC.$_.DONE" } @nilesuffixes);
        safesystem("cat $PREF/nile/$TRG$SRC.* > $PREF/nile/$TRG$SRC") if not -e "$PREF/nile/$TRG$SRC";
        safesystem("cat $PREF/nile/$TRG$SRC | sed \"s/\\([0-9][0-9]*\\)-\\([0-9][0-9]*\\)/\\2-\\1/g\" > $PREF/nile/$SRC$TRG") if not -e "$PREF/nile/$SRC$TRG";
    }

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

sub split_lens {
    my ($file, $suffs, $lens) = @_;
    open FILE0, "<:utf8", $file or die "Couldn't open $file\n";
    foreach my $i (0 .. @$suffs-1) {
        open FILE1, ">:utf8", "$file.$suffs->[$i]" or die "Couldn't open $file.$suffs->[$i]\n";
        my $len = $lens->[$i];
        my $line;
        for(1 .. $len) {
            my $line = <FILE0> or die "Ran out of lines to read in split";
            print FILE1 $line;
        }
        close FILE1; 
    }
    close FILE0;
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
