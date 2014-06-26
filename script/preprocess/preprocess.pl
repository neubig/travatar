#!/usr/bin/perl
##############################################################################
# Preprocessing script for Travatar
#  by Graham Neubig
#
# This is a preprocessing script for machine translation,
# specifically using Travatar. It can be used by running
#   $ preprocess.pl -src SRC -trg TRG INPUT_SRC INPUT_TRG OUTPUT_DIR
# or
#   $ preprocess.pl -src SRC INPUT_SRC OUTPUT_DIR
#
# Where INPUT_SRC and INPUT_TRG are source and target files of parallel
# sentences, and OUTPUT_DIR is the directory where you will put the output.
# The default file encoding is UTF-8 for all languages.
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
# French/German:
#  Stanford Parser: http://nlp.stanford.edu/software/lex-parser.shtml
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
use IO::File;
print STDERR "PREPROC ARGS:\n@ARGV\n";

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
my $EGRET_SRC_MODEL;
my $EGRET_TRG_MODEL;
my $EGRET_FOREST_OPT = "-nbest4threshold=100";
my $SPLIT_WORDS_SRC;
my $SPLIT_WORDS_TRG;
my $TRUECASE_SRC;
my $TRUECASE_TRG;
my $TRUECASE_SRC_MODEL;
my $TRUECASE_TRG_MODEL;
# How to split words, choose "interleave" for efficiency
# when using a corpus that may consist of different blocks
# with widely different sentence lengths (such as when you
# concatenate a corpus and a dictionary)
my $SPLIT_TYPE = "consecutive";
my $EDA_DIR;
my $EDA_VOCAB;
my $EDA_WEIGHT;
my $GIZA_DIR;
my $NILE_DIR;
my $NILE_GIZATYPE = "intersect";
my $NILE_MODEL;
my $NILE_BINARIZE = "rightrp";
my $NILE_SEGMENTS = 200;
my $NILE_BEAM = 64;
my $NILE_ORDER = "srctrg";
my $KYTEA = "kytea";
my $FOREST_SRC;
my $FOREST_TRG;
my $ALIGN;
my $SRC;
my $TRG;

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
    "align" => \$ALIGN,
    "clean-len=i" => \$CLEAN_LEN,
    "eda-dir=s" => \$EDA_DIR,
    "eda-vocab=s" => \$EDA_VOCAB,
    "eda-weight=s" => \$EDA_WEIGHT,
    "egret-dir=s" => \$EGRET_DIR,
    "egret-forest-opt=s" => \$EGRET_FOREST_OPT,
    "egret-src-model=s" => \$EGRET_SRC_MODEL,
    "egret-trg-model=s" => \$EGRET_TRG_MODEL,
    "forest-src" => \$FOREST_SRC,
    "forest-trg" => \$FOREST_TRG,
    "giza-dir=s" => \$GIZA_DIR,
    "kytea=s" => \$KYTEA,
    "nile-beam=i" => \$NILE_BEAM,
    "nile-dir=s" => \$NILE_DIR,
    "nile-model=s" => \$NILE_MODEL,
    "nile-gizatype=s" => \$NILE_GIZATYPE,
    "nile-segments=i" => \$NILE_SEGMENTS,
    "nile-order=s" => \$NILE_ORDER,
    "program-dir=s" => \$PROGRAM_DIR,
    "split-words-src=s" => \$SPLIT_WORDS_SRC,
    "split-words-trg=s" => \$SPLIT_WORDS_TRG,
    "split-type=s" => \$SPLIT_TYPE,
    "src=s" => \$SRC,
    "stanford-jars=s" => \$STANFORD_JARS,
    "threads=s" => \$THREADS,
    "travatar-dir=s" => \$TRAVATAR_DIR,
    "truecase-src-model=s" => \$TRUECASE_SRC_MODEL,
    "truecase-trg-model=s" => \$TRUECASE_TRG_MODEL,
    "truecase-src" => \$TRUECASE_SRC,
    "truecase-trg" => \$TRUECASE_TRG,
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
my %EGRET_DEFAULT_MODEL = ( "en" => "$EGRET_DIR/eng_grammar",  "zh" => "$EGRET_DIR/chn_grammar" );

### Sanity check
foreach my $f ($NILE_MODEL, $EGRET_SRC_MODEL, $EGRET_TRG_MODEL, $TRUECASE_SRC_MODEL, $TRUECASE_TRG_MODEL) {
    die "Model file $f specified but not found" if($f and not -e $f);
}

if(not $TRAVATAR_DIR) {
    $TRAVATAR_DIR = abs_path($0);
    $TRAVATAR_DIR =~ s!/script/preprocess/.*\.pl!!g;
}

# Sanity check of input
my ($SRCORIG, $TRGORIG, $PREF);
if((@ARGV == 3) and $SRC and $TRG) {
    (-f $ARGV[0]) or die "$SRC file $ARGV[0] doesn't exist.";
    (-f $ARGV[1]) or die "$TRG file $ARGV[1] doesn't exist.";
    ($SRCORIG, $TRGORIG, $PREF) = @ARGV;
} elsif((@ARGV == 2) and $SRC and not $TRG) {
    (-f $ARGV[0]) or die "$SRC file $ARGV[0] doesn't exist.";
    ($SRCORIG, $PREF) = @ARGV;
} else {
    print STDERR "Usage: $0 -src SRC -trg TRG INPUT_SRC INPUT_TRG OUTPUT_DIR\n";
    print STDERR " or\n";
    print STDERR "Usage: $0 -src SRC INPUT_SRC OUTPUT_DIR\n";
    exit 1;
}
-e "$PREF" or mkdir "$PREF";

###### Get and check the lengths #######
my ($SRCLEN, $TRGLEN);
$SRCLEN = file_len($ARGV[0]);
my $len = int($SRCLEN/$THREADS+1);
my $suflen = int(log($THREADS)/log(10)+1);
my @suffixes = map { sprintf("%0${suflen}d", $_) } (0 .. $THREADS-1);
if($TRG) {
    $TRGLEN = file_len($ARGV[1]);
    die "$SRC and $TRG lengths don't match ($SRCLEN != $TRGLEN)" if ($SRCLEN != $TRGLEN);
}

###### Split the input files #######
if(-e "$PREF/orig") {
    wait_done("$PREF/orig.DONE");
} else {
    -e "$PREF/orig" or mkdir "$PREF/orig";
    # Copy the original, but remove carriage returns unicode newlines
    -e "$PREF/orig/$SRC" or safesystem("sed 's/\\r\\|\\xE2\\x80\\xA8//g' < $ARGV[0] > $PREF/orig/$SRC");
    split_file($len, $suflen, \@suffixes, "$PREF/orig/$SRC", "$PREF/orig/$SRC.");
    if($TRG) {
        -e "$PREF/orig/$TRG" or safesystem("sed 's/\\r\\|\\xE2\\x80\\xA8//g' < $ARGV[1] > $PREF/orig/$TRG");
        split_file($len, $suflen, \@suffixes, "$PREF/orig/$TRG", "$PREF/orig/$TRG.");
    }
    safesystem("touch $PREF/orig.DONE") or die;
}

###### Tokenization and Lowercasing #######
sub tokenize_cmd {
    if($_[0] =~ /^(en|de)$/) { return "java -cp $STANFORD_JARS edu.stanford.nlp.process.PTBTokenizer -preserveLines INFILE | sed \"s/(/-LRB-/g; s/)/-RRB-/g; s/[\t ]+/ /g; s/^ +//g; s/ +\$//g\" > OUTFILE"; }
    elsif($_[0] eq "fr") { return "java -cp $STANFORD_JARS edu.stanford.nlp.process.PTBTokenizer -options asciiQuotes -preserveLines INFILE | sed \"s/(/-LRB-/g; s/)/-RRB-/g; s/[\t ]+/ /g; s/^ +//g; s/ +\$//g\" > OUTFILE"; }
    elsif($_[0] eq "ja") { return "$KYTEA -notags -wsconst D INFILE | sed \"s/[\t ]+/ /g; s/^ +//g; s/ +\$//g\" > OUTFILE"; }
    elsif($_[0] eq "zh") { return "$STANFORD_SEG_DIR/segment.sh ctb INFILE UTF-8 0 | sed \"s/(/-LRB-/g; s/)/-RRB-/g; s/[\t ]+/ /g; s/^ +//g; s/ +\$//g\" > OUTFILE"; }
    else { die "Cannot tokenize $_[0]"; }
}
run_parallel("$PREF/orig", "$PREF/tok", $SRC, tokenize_cmd($SRC));
run_parallel("$PREF/orig", "$PREF/tok", $TRG, tokenize_cmd($TRG)) if $TRG;

###### Cleaning #######
if($CLEAN_LEN == 0) {
    safesystem("ln -s ".abs_path("$PREF/tok")." $PREF/clean") if not -e "$PREF/clean";
} elsif (not -e "$PREF/clean/$SRC") {
    $TRG or die "Currently, cleaning is only supported for bilingual preprocessing.";
    -e "$PREF/clean" or mkdir "$PREF/clean";
    foreach my $i (@suffixes) {
        safesystem("bash -c '$TRAVATAR_DIR/script/train/clean-corpus.pl -max_len $CLEAN_LEN $PREF/tok/$SRC.$i $PREF/tok/$TRG.$i $PREF/clean/$SRC.$i $PREF/clean/$TRG.$i; touch $PREF/clean/$SRC.$i.DONE' &")  if not -e "$PREF/clean/$SRC.$i";
    }
    wait_done(map { "$PREF/clean/$SRC.$_.DONE" } @suffixes);
    cat_files("$PREF/clean/$SRC", map { "$PREF/clean/$SRC.$_" } @suffixes);
    cat_files("$PREF/clean/$TRG", map { "$PREF/clean/$TRG.$_" } @suffixes);
}

###### 1-best Parsing ######
# TODO: clean this up
sub run_tree_parsing {
    my $lang = shift;
    my $split_words = shift;
    my $is_src = shift;
    my $EGRET_MODEL = ($is_src ? $EGRET_SRC_MODEL : $EGRET_TRG_MODEL);
    my $SPLIT_CMD = "";
    $SPLIT_CMD = "| $TRAVATAR_DIR/src/bin/tree-converter -split \"$split_words\"" if $split_words;
    if($lang =~ /^(en|zh)$/) {
        if($lang eq "en") {
            # EN Parsing with Stanford Parser
            run_parallel("$PREF/clean", "$PREF/stanford", $lang, "java -mx2000m -cp $STANFORD_JARS edu.stanford.nlp.parser.lexparser.LexicalizedParser -tokenized -sentences newline -outputFormat oneline edu/stanford/nlp/models/lexparser/englishPCFG.ser.gz INFILE 2> OUTFILE.log > OUTFILE");
    
            # EN Parsing with Egret
            $EGRET_MODEL = "$EGRET_DIR/eng_grammar" if not $EGRET_MODEL;
            run_parallel("$PREF/clean", "$PREF/egret", $lang, "$EGRET_DIR/egret -lapcfg -i=INFILE -data=$EGRET_MODEL 2> OUTFILE.log > OUTFILE");
        } else {

            # ZH Convert to GB encoding, same as the penn treebank
            run_parallel("$PREF/clean", "$PREF/cleangb", $lang, "iconv -sc -f UTF-8 -t GB18030 < INFILE | sed \"s/^[ \t]*\$/FAILED/g\" > OUTFILE");

            # ZH Parsing with Stanford Parser
            run_parallel("$PREF/cleangb", "$PREF/stanford", $lang, "java -mx2000m -cp $STANFORD_JARS edu.stanford.nlp.parser.lexparser.LexicalizedParser -tLPP \"edu.stanford.nlp.parser.lexparser.ChineseTreebankParserParams\" -chineseFactored -encoding GB18030 -tokenized -sentences newline -outputFormat oneline edu/stanford/nlp/models/lexparser/chinesePCFG.ser.gz INFILE 2> OUTFILE.log | iconv -f GB18030 -t UTF-8 > OUTFILE");
    
            # ZH Parsing with Egret
            $EGRET_MODEL = "$EGRET_DIR/chn_grammar" if not $EGRET_MODEL;
            run_parallel("$PREF/cleangb", "$PREF/egret", $lang, "$EGRET_DIR/egret -lapcfg -i=INFILE -data=$EGRET_MODEL 2> OUTFILE.log | iconv -f GB18030 -t UTF-8 > OUTFILE");
        }
    
        # Combine Stanford and Egret (for now this is not parallel)
        -e "$PREF/tree" or mkdir "$PREF/tree";
        foreach my $i ("", map{".$_"} @suffixes) {
            if(not -e "$PREF/tree/$lang$i") {
                safesystem("$TRAVATAR_DIR/script/tree/replace-failed-parse.pl $PREF/stanford/$lang$i $PREF/egret/$lang$i $SPLIT_CMD > $PREF/tree/$lang$i") ;
                die "Combining trees failed on $lang$i" if(file_len("$PREF/stanford/$lang$i") != file_len("$PREF/tree/$lang$i"));
            }
        }
    } elsif($lang eq "ja") {
        # JA Parsing with Eda
        run_parallel("$PREF/clean", "$PREF/edain", $lang, "cat INFILE | $TRAVATAR_DIR/script/tree/han2zen.pl -nospace -remtab | $KYTEA -nows -in tok -out eda > OUTFILE", 1);
        run_parallel("$PREF/edain", "$PREF/eda", $lang, "$EDA_DIR/src/eda/eda -e INFILE -v $EDA_VOCAB -w $EDA_WEIGHT > OUTFILE");
        if($EGRET_MODEL) {
            run_parallel("$PREF/eda", "$PREF/edacfg", $lang, "cat INFILE | $TRAVATAR_DIR/script/tree/ja-adjust-dep.pl | $TRAVATAR_DIR/script/tree/ja-dep2cfg.pl > OUTFILE", 1);
            run_parallel("$PREF/clean", "$PREF/egret", $lang, "cat INFILE | sed \"s/(/-LRB-/g; s/)/-RRB-/g\" | $EGRET_DIR/egret -lapcfg -i=/dev/stdin -data=$EGRET_MODEL 2> OUTFILE.log > OUTFILE");
            # Combine Eda and Egret (for now this is not parallel)
            -e "$PREF/tree" or mkdir "$PREF/tree";
            foreach my $i ("", map{".$_"} @suffixes) {
                if(not -e "$PREF/tree/$lang$i") {
                    safesystem("$TRAVATAR_DIR/script/tree/replace-failed-parse.pl $PREF/edacfg/$lang$i $PREF/egret/$lang$i $SPLIT_CMD > $PREF/tree/$lang$i") ;
                    die "Combining trees failed on $lang$i" if(file_len("$PREF/edacfg/$lang$i") != file_len("$PREF/tree/$lang$i"));
                }
            }
        } else {
            run_parallel("$PREF/eda", "$PREF/tree", $lang, "cat INFILE | $TRAVATAR_DIR/script/tree/ja-adjust-dep.pl | $TRAVATAR_DIR/script/tree/ja-dep2cfg.pl > OUTFILE", 1);
        }
    } elsif($lang =~ /^(fr|de)$/) {
        my $model;
        if($lang eq "fr") { $model = "frenchFactored"; }
        elsif($lang eq "de") { $model = "germanPCFG"; }
        if($EGRET_MODEL) {
            # Parsing with Stanford Parser
            run_parallel("$PREF/clean", "$PREF/stanford", $lang, "java -mx2000m -cp $STANFORD_JARS edu.stanford.nlp.parser.lexparser.LexicalizedParser -encoding utf-8 -tokenized -sentences newline -outputFormat oneline edu/stanford/nlp/models/lexparser/$model.ser.gz INFILE 2> OUTFILE.log $SPLIT_CMD > OUTFILE");
    
            # Parsing with Egret
            run_parallel("$PREF/clean", "$PREF/egret", $lang, "$EGRET_DIR/egret -lapcfg -i=INFILE -data=$EGRET_MODEL 2> OUTFILE.log > OUTFILE");

            # Combine Stanford and Egret (for now this is not parallel)
            -e "$PREF/tree" or mkdir "$PREF/tree";
            foreach my $i ("", map{".$_"} @suffixes) {
                if(not -e "$PREF/tree/$lang$i") {
                    safesystem("$TRAVATAR_DIR/script/tree/replace-failed-parse.pl $PREF/stanford/$lang$i $PREF/egret/$lang$i $SPLIT_CMD > $PREF/tree/$lang$i") ;
                    die "Combining trees failed on $lang$i" if(file_len("$PREF/stanford/$lang$i") != file_len("$PREF/tree/$lang$i"));
                }
            }
        } else {
            run_parallel("$PREF/clean", "$PREF/tree", $lang, "java -mx2000m -cp $STANFORD_JARS edu.stanford.nlp.parser.lexparser.LexicalizedParser -encoding utf-8 -tokenized -sentences newline -outputFormat oneline edu/stanford/nlp/models/lexparser/$model.ser.gz INFILE 2> OUTFILE.log $SPLIT_CMD > OUTFILE");
        }
    } else { die "Cannot parse $lang"; }

    # Lowercase and print
    run_parallel("$PREF/tree", "$PREF/treelow", $lang, "$TRAVATAR_DIR/script/tree/lowercase.pl < INFILE > OUTFILE");
    run_parallel("$PREF/treelow", "$PREF/low", $lang, "$TRAVATAR_DIR/src/bin/tree-converter -output_format word < INFILE > OUTFILE");
}
run_tree_parsing($SRC, $SPLIT_WORDS_SRC, 1);
run_tree_parsing($TRG, $SPLIT_WORDS_TRG, 0) if $TRG;

###### Forest Parsing ######

sub run_forest_parsing {
    my $lang = shift;
    my $split_words = shift;
    my $is_src = shift;
    my $SPLIT_CMD = "";
    $SPLIT_CMD = "| $TRAVATAR_DIR/src/bin/tree-converter -split \"$split_words\" -input_format egret -output_format egret" if $split_words;

    # Find the model
    my $EGRET_MODEL = ($is_src ? $EGRET_SRC_MODEL : $EGRET_TRG_MODEL);
    $EGRET_MODEL = $EGRET_DEFAULT_MODEL{$lang} if(not $EGRET_MODEL);
    die "No Egret model specified for $lang" if not $EGRET_MODEL;

    # Convert Tree to Forest
    run_parallel("$PREF/tree", "$PREF/treefor", $lang, "$TRAVATAR_DIR/src/bin/tree-converter -input_format penn -output_format egret < INFILE 2> OUTFILE.log > OUTFILE", 1);
    
    if($lang eq "zh") {
        # ZH Parse with Egret
        run_parallel("$PREF/cleangb", "$PREF/egretfor", $lang, "$EGRET_DIR/egret -lapcfg -i=INFILE -printForest $EGRET_FOREST_OPT -data=$EGRET_MODEL 2> OUTFILE.log  | iconv -f GB18030 -t UTF-8 | sed \"s/\\^g//g\" > OUTFILE", 1);
    } elsif($lang eq "ja") {
        # JA Parse with Egret
        run_parallel("$PREF/clean", "$PREF/egretfor", $lang, "cat INFILE | sed \"s/(/-LRB-/g; s/)/-RRB-/g\" | $EGRET_DIR/egret -lapcfg -i=/dev/stdin -printForest $EGRET_FOREST_OPT -data=$EGRET_MODEL 2> OUTFILE.log > OUTFILE", 1);
    } else {
        # Anything else
        run_parallel("$PREF/clean", "$PREF/egretfor", $lang, "$EGRET_DIR/egret -lapcfg -i=INFILE -printForest $EGRET_FOREST_OPT -data=$EGRET_MODEL 2> OUTFILE.log | sed \"s/\\^g//g\" > OUTFILE", 1);
    }
    
    # ZH Combine Stanford and Egret (for now this is not parallel)
    -e "$PREF/for" or mkdir "$PREF/for";
    foreach my $i ("", map{".$_"} @suffixes) {
        safesystem("$TRAVATAR_DIR/script/tree/replace-failed-parse.pl -format egret $PREF/treefor/$lang$i $PREF/egretfor/$lang$i $SPLIT_CMD > $PREF/for/$lang$i") if not -e "$PREF/for/$lang$i";
        die "Combining forests failed on en$i" if(file_len("$PREF/for/$lang$i") == 0);
    }

    # Combine and lowercase the forest
    run_parallel("$PREF/for", "$PREF/forlow", $lang, "$TRAVATAR_DIR/script/tree/lowercase.pl < INFILE > OUTFILE");
}
run_forest_parsing($SRC, $SPLIT_WORDS_SRC, 1) if $FOREST_SRC;
run_forest_parsing($TRG, $SPLIT_WORDS_TRG, 0) if $FOREST_TRG;

##### Truecasing ######

sub run_truecase {
    my $lang = shift;
    my $model = shift;
    run_parallel("$PREF/tree", "$PREF/high", $lang, "$TRAVATAR_DIR/src/bin/tree-converter -output_format word < INFILE > OUTFILE");
    if(not $model) {
        $model = "$PREF/truecaser/$lang.truecaser";
        if(not -e $model) {
            safesystem("mkdir -p $PREF/truecaser") or die;
            safesystem("$TRAVATAR_DIR/script/recaser/train-truecaser.pl --corpus $PREF/high/$lang --model $model");
        }
    }
    run_parallel("$PREF/high", "$PREF/true", $lang, "$TRAVATAR_DIR/script/recaser/truecase.pl --model $model < INFILE > OUTFILE");
}
run_truecase($SRC, $TRUECASE_SRC_MODEL) if $TRUECASE_SRC;
run_truecase($TRG, $TRUECASE_TRG_MODEL) if $TRUECASE_TRG;

###### Alignment ######

# Run the training script
if($ALIGN) {
    $TRG or die "Aligning cannot be performed when target is not specified.";

    # Align using GIZA++
    safesystem("$TRAVATAR_DIR/script/train/train-travatar.pl -last_step lex -work_dir $PREF/train -no_lm true -src_words $PREF/low/$SRC -src_file $PREF/treelow/$SRC -trg_file $PREF/low/$TRG -travatar_dir $TRAVATAR_DIR -bin_dir $GIZA_DIR -threads $THREADS > $PREF/train.log") if not -e "$PREF/train/align";
    safesystem("mkdir $PREF/giza") if not -e "$PREF/giza";
    safesystem("cp $PREF/train/align/align.txt $PREF/giza/$SRC$TRG") if not -e "$PREF/giza/$SRC$TRG";
    safesystem("cat $PREF/giza/$SRC$TRG | sed \"s/\\([0-9][0-9]*\\)-\\([0-9][0-9]*\\)/\\2-\\1/g\" > $PREF/giza/$TRG$SRC") if not -e "$PREF/giza/$TRG$SRC";

    # Run nile if a model is specified
    if($NILE_MODEL) {
        # Check file
        die "Could not find nile at $NILE_DIR/nile.py" if not -e "$NILE_DIR/nile.py";
        die "Could not find nile model $NILE_MODEL" if not -e "$NILE_MODEL";
        # Find the actual directions to use for nile
        my ($NSRC, $NTRG, $NTGS, $NSGT);
        if($NILE_ORDER eq "srctrg") {
            $NSRC = $SRC; $NTRG = $TRG; $NTGS = "trg_given_src"; $NSGT = "src_given_trg";
        } elsif($NILE_ORDER eq "trgsrc") {
            $NSRC = $TRG; $NTRG = $SRC; $NTGS = "src_given_trg"; $NSGT = "trg_given_src";
        } else {
            die "Invalid nile order $NILE_ORDER";
        }
        # Get the splits
        my $CLEANLEN = file_len("$PREF/low/$NTRG");
        my $nilelen = int($CLEANLEN/$NILE_SEGMENTS+1);
        my $nilesuflen = int(log($NILE_SEGMENTS)/log(10)+1);
        my @nilesuffixes = map { sprintf("%0${nilesuflen}d", $_) } (0 .. $NILE_SEGMENTS-1);
        # Binarize the trees on both sides
        run_parallel("$PREF/treelow", "$PREF/treelowbin", $NTRG, "$TRAVATAR_DIR/src/bin/tree-converter -binarize $NILE_BINARIZE < INFILE > OUTFILE");
        run_parallel("$PREF/treelow", "$PREF/treelowbin", $NSRC, "$TRAVATAR_DIR/src/bin/tree-converter -binarize $NILE_BINARIZE < INFILE > OUTFILE");
        # Create and split GIZA++ union alignments
        my $GIZAOUTDIR = ($NILE_GIZATYPE eq "union") ? "gizau" : "gizai";
        (safesystem("mkdir $PREF/$GIZAOUTDIR") or die) if not -e "$PREF/$GIZAOUTDIR";
        (safesystem("$TRAVATAR_DIR/script/train/symmetrize.pl -sym $NILE_GIZATYPE $PREF/train/align/src-trg.giza.A3.final $PREF/train/align/trg-src.giza.A3.final > $PREF/$GIZAOUTDIR/$SRC$TRG") or die) if not -e "$PREF/$GIZAOUTDIR/$SRC$TRG";
        (safesystem("cat $PREF/$GIZAOUTDIR/$SRC$TRG | sed \"s/\\([0-9][0-9]*\\)-\\([0-9][0-9]*\\)/\\2-\\1/g\" > $PREF/$GIZAOUTDIR/$TRG$SRC") or die) if not -e "$PREF/$GIZAOUTDIR/$TRG$SRC";
        # Creating splits
        (safesystem("mkdir $PREF/nilein") or die) if not -e "$PREF/nilein";
        if(not -e "$PREF/nilein/low-$NTRG.$nilesuffixes[0]") {
            split_file($nilelen, $nilesuflen, \@nilesuffixes, "$PREF/low/$NTRG", "$PREF/nilein/low-$NTRG.");
            split_file($nilelen, $nilesuflen, \@nilesuffixes, "$PREF/low/$NSRC", "$PREF/nilein/low-$NSRC.");
            split_file($nilelen, $nilesuflen, \@nilesuffixes, "$PREF/giza/$NSRC$NTRG",        "$PREF/nilein/giza.");
            split_file($nilelen, $nilesuflen, \@nilesuffixes, "$PREF/$GIZAOUTDIR/$NSRC$NTRG", "$PREF/nilein/$GIZAOUTDIR.");
            split_file($nilelen, $nilesuflen, \@nilesuffixes, "$PREF/treelowbin/$NTRG", "$PREF/nilein/tree-$NTRG.");
            split_file($nilelen, $nilesuflen, \@nilesuffixes, "$PREF/treelowbin/$NSRC", "$PREF/nilein/tree-$NSRC.");
            foreach my $f (@nilesuffixes) {
                safesystem("$NILE_DIR/prepare-vocab.py < $PREF/nilein/low-$NTRG.$f > $PREF/nilein/vcb-$NTRG.$f");
                safesystem("$NILE_DIR/prepare-vocab.py < $PREF/nilein/low-$NSRC.$f > $PREF/nilein/vcb-$NSRC.$f");
            }
            safesystem("touch $PREF/nilein/prep.DONE") or die;
        }
        wait_done("$PREF/nilein/prep.DONE");
        # Use Nile to create alignments for each segment
        (safesystem("mkdir $PREF/nile") or die) if not -e "$PREF/nile";
        foreach my $s (@nilesuffixes) {
            # Do nile using trees on the target side
            if(not -e "$PREF/nile/$NSRC$NTRG.$s") {
                safesystem("touch $PREF/nile/$NSRC$NTRG.$s");
                safesystem("mpiexec -n $THREADS python $NILE_DIR/nile.py --f $PREF/nilein/low-$NSRC.$s --e $PREF/nilein/low-$NTRG.$s --etrees $PREF/nilein/tree-$NTRG.$s --ftrees $PREF/nilein/tree-$NSRC.$s --evcb $PREF/nilein/vcb-$NTRG.$s --fvcb $PREF/nilein/vcb-$NSRC.$s --pef $PREF/train/lex/$NTGS.lex --pfe $PREF/train/lex/$NSGT.lex --a1 $PREF/nilein/giza.$s --a2 $PREF/nilein/$GIZAOUTDIR.$s --align --langpair ${NSRC}_${NTRG} --weights $NILE_MODEL --out $PREF/nile/$NSRC$NTRG.$s --k $NILE_BEAM") or die;
                safesystem("touch $PREF/nile/$NSRC$NTRG.$s.DONE");
            }
        }
        wait_done(map { "$PREF/nile/$NSRC$NTRG.$_.DONE" } @nilesuffixes);
        cat_files("$PREF/nile/$NSRC$NTRG", map { "$PREF/nile/$NSRC$NTRG.$_" } @nilesuffixes) if not -e "$PREF/nile/$NSRC$NTRG";
        safesystem("cat $PREF/nile/$NSRC$NTRG | sed \"s/\\([0-9][0-9]*\\)-\\([0-9][0-9]*\\)/\\2-\\1/g\" > $PREF/nile/$NTRG$NSRC") if not -e "$PREF/nile/$NTRG$NSRC";
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
    cat_files("$out_dir/$prefix", map { "$out_dir/$_" } @files); 
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

# Split files 
sub split_file {
    my ($len, $suflen, $sufs, $infile, $outpref) = @_;
    if($SPLIT_TYPE eq "consecutive") {
        safesystem("split -l $len -a $suflen -d $infile $outpref") or die;
    } elsif($SPLIT_TYPE eq "interleave") {
        my $filecnt = @$sufs;
        safesystem("awk '{print >(sprintf(\"$outpref%0${suflen}d\", (NR-1)%$filecnt))}' $infile");
    } else {
        die "Bad split type: $SPLIT_TYPE";
    }
}

# Split files 
sub cat_files {
    my ($out, @in) = @_;
    if($SPLIT_TYPE eq "consecutive") {
        safesystem("cat @in > $out") or die;
    } elsif($SPLIT_TYPE eq "interleave") {
        # Read from each file one line at a time
        my @fh = map { 
            my $fh = IO::File->new("< $_") or die "Couldn't input from $_";
            binmode $fh, ":utf8";
            $fh
        } @in;
        open FILE0, ">:utf8", $out or die "Couldn't open $out";
        my $done = @fh;
        while($done == @fh) {
            $done = 0;
            foreach my $fhin (@fh) {
                last if not defined ($_ = <$fhin>);
                print FILE0;
                $done++;
            }
        }
        close FILE0;
    } else {
        die "Bad split type: $SPLIT_TYPE";
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
