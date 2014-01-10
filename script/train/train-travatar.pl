#!/usr/bin/perl

use strict;
use warnings;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

# Directory options
my $WORK_DIR = "";
my $TRAVATAR_DIR = ""; 
my $BIN_DIR = "";

# Translation Method options
my $TRANSLATION_METHOD = "f2s";

# Parallelization options
my $THREADS = "1";

# Use this option do only part of the training (e.g., only alignment)
my $LAST_STEP = ""; # options: (align,lex)

# Input/output options
my $SRC_FILE = "";
my $SRC_WORDS = "";
my $SRC_FORMAT = "penn";

my $TRG_FILE = "";
my $TRG_FORMAT = "word";
my $TRG_WORDS = "";

# Files containing lexical translation probabilities
my $LEX_SRCTRG = "";
my $LEX_TRGSRC = "";

# Alignment options
my $ALIGN_FILE = "";
my $ALIGN = "giza";
my $SYMMETRIZE = "grow";

# Rule extraction options (T2S)
my $NORMALIZE = "false";
my $BINARIZE = "right";
my $COMPOSE = "4";
my $SRC_LEX_LEN = "-1";
my $ATTACH = "top";
my $ATTACH_LEN = "1";
my $NONTERM_LEN = "2";
my $TERM_LEN = "10";
my $NBEST_RULES = "20";
my $SCORE_OPTIONS = "";
my $SMOOTH = "none";

# Rule extraction options (Hiero)
my $INITIAL_PHRASE_LEN = "10";
my $RULE_MAX_LEN = "5";

# Model files
my $TM_FILE = "";
my $GZIP_TM = "true"; # Whether to gzip the rule table
my $LM_FILE = "";
my $NO_LM = "false";
my $CONFIG_FILE = "";

GetOptions(
    "work_dir=s" => \$WORK_DIR, # The working directory to use
    "travatar_dir=s" => \$TRAVATAR_DIR, # The directory of travatar
    "bin_dir=s" => \$BIN_DIR, # A directory for external bin files (mainly GIZA)
    "threads=s" => \$THREADS, # The number of threads to use
    "last_step=s" => \$LAST_STEP, # The last step to perform
    "src_file=s" => \$SRC_FILE, # The source file you want to train on
    "src_words=s" => \$SRC_WORDS, # A file of plain-text sentences from the source
    "src_format=s" => \$SRC_FORMAT, # The source file format (penn/egret)
    "trg_file=s" => \$TRG_FILE, # The target file you want to train on
    "trg_words=s" => \$TRG_WORDS, # A file of plain-text sentences from the target
    "trg_format=s" => \$TRG_FORMAT, # The target file format (word/penn/egret)
    "lex_srctrg=s" => \$LEX_SRCTRG, # of the source word given the target P(f|e)
    "lex_trgsrc=s" => \$LEX_TRGSRC, # of the target word given the source P(e|f)
    "align_file=s" => \$ALIGN_FILE, # A file containing alignments
    "align=s" => \$ALIGN, # The type of alignment to use (giza)
    "symmetrize=s" => \$SYMMETRIZE, # The type of symmetrization to use (grow)
    "normalize=s" => \$NORMALIZE, # Normalize rule counts to probabilities
    "binarize=s" => \$BINARIZE, # Binarize trees in a certain direction
    "compose=s" => \$COMPOSE, # The number of rules to compose
    "src_lex_len=s" => \$SRC_LEX_LEN, # The length of fully lexicalized rules to use
    "smooth=s" => \$SMOOTH, # The type of smoothing to use on the rule table
    "attach=s" => \$ATTACH, # Where to attach nulls
    "attach_len=s" => \$ATTACH_LEN, # The number of nulls to attach
    "nonterm_len=s" => \$NONTERM_LEN, # The maximum number of non-terminals in a rule
    "term_len=s" => \$TERM_LEN, # The maximum number of terminals in a rule
    "nbest_rules=s" => \$NBEST_RULES, # The maximum number of rules for each source
    "score_options=s" => \$SCORE_OPTIONS, # Any additional options to the score-t2s.pl script (src-trg)
    "tm_file=s" => \$TM_FILE, # An already created TM file
    "lm_file=s" => \$LM_FILE, # An already created LM file
    "config_file=s" => \$CONFIG_FILE, # Where to output the configuration file
    "no_lm=s" => \$NO_LM, # Indicates that no LM will be used
    "method=s" => \$TRANSLATION_METHOD, # The translation method that is used for travatar (f2s or hiero)
    "initial_phrase=s" => \$INITIAL_PHRASE_LEN, # The maximum length of initial phrase in hiero extraction
    "hiero_rule_len=s" => \$RULE_MAX_LEN, # The maximum length of hiero rules that are extracted from hiero extraction
);
if(@ARGV != 0) {
    print STDERR "Usage: $0 --work_dir=work --src_file=src.txt --trg_file=trg.txt\n";
    exit 1;
}

# Sanity check!
((!$WORK_DIR) or (!$SRC_FILE) or (!$TRG_FILE) or (!$TRAVATAR_DIR)) and
    die "Must specify -work_dir ($WORK_DIR) -src_file ($SRC_FILE) -trg_file ($TRG_FILE) -travatar_dir ($TRAVATAR_DIR)";
(!$BIN_DIR) and (!$ALIGN_FILE) and
    die "Must specify -bin_dir ($BIN_DIR) or -align_file ($ALIGN_FILE)";
for($SRC_FILE, $TRG_FILE, $TRAVATAR_DIR, $SRC_WORDS, $TRG_WORDS, $ALIGN_FILE, $LEX_SRCTRG, $LEX_TRGSRC, $LM_FILE) {
    die "File specified but not found: $_" if $_ and not -e $_;
}
((not $LM_FILE) and ($NO_LM ne "true")) and
    die "Must specify an LM file using -lm_file, or choose to not use an LM by setting -no_lm true";
((-e $WORK_DIR) or not safesystem("mkdir $WORK_DIR")) and
    die "Working directory $WORK_DIR already exists or could not be created";

# Steps:
# 1 -> Prepare Data
# 2 -> Create Alignments
# 3 -> Create lexical translation probabilities
# 4 -> Extract and Score Rule Table
# 5 -> Create 

# ******** 1: Prepare Data **********
print STDERR "(1) Preparing data @ ".`date`;

# Convert trees into plain text sentences
$TRG_WORDS = $TRG_FILE if((not $TRG_WORDS) and ($TRG_FORMAT eq "word"));
(safesystem("mkdir $WORK_DIR/data") or die) if ((not $SRC_WORDS) or (not $TRG_WORDS));
if(not $SRC_WORDS) {
    $SRC_WORDS = "$WORK_DIR/data/src.word";
    if ($TRANSLATION_METHOD eq "t2s") {
        safesystem("$TRAVATAR_DIR/src/bin/tree-converter -input_format $SRC_FORMAT -output_format word < $SRC_FILE > $SRC_WORDS") or die;
    } else {
        # Hiero
        safesystem("cp $SRC_FILE $SRC_WORDS");
    }
}
if(not $TRG_WORDS) {
    $TRG_WORDS = "$WORK_DIR/data/trg.word";
    if ($TRANSLATION_METHOD eq "t2s") {
        safesystem("$TRAVATAR_DIR/src/bin/tree-converter -input_format $TRG_FORMAT -output_format word < $TRG_FILE > $TRG_WORDS") or die;
    } else {
        # Hiero
        safesystem("cp $TRG_FILE $TRG_WORDS");
    }
}

# ****** 2: Create Alignments *******
print STDERR "(2) Creating alignments @ ".`date`;

# Alignment with GIZA++
if(not $ALIGN_FILE) {
    safesystem("mkdir $WORK_DIR/align") or die;
    $ALIGN_FILE = "$WORK_DIR/align/align.txt";
    if($ALIGN eq "giza") {
        my $GIZA = "$BIN_DIR/GIZA++";
        my $SNT2COOC = "$BIN_DIR/snt2cooc.out";
        my $PLAIN2SNT = "$BIN_DIR/plain2snt.out";
        my $MKCLS = "$BIN_DIR/mkcls";
        ((not -x $GIZA) or (not -x $SNT2COOC)) and
            die "Could not execute GIZA ($GIZA) or snt2cooc ($SNT2COOC)";
        # Make the classes with mkcls
        my $SRC_CLASSES = "$WORK_DIR/align/src.vcb.classes";
        my $TRG_CLASSES = "$WORK_DIR/align/trg.vcb.classes";
        my $SRC_MKCLS = "$MKCLS -c50 -n2 -p$SRC_WORDS -V$SRC_CLASSES opt";
        my $TRG_MKCLS = "$MKCLS -c50 -n2 -p$TRG_WORDS -V$TRG_CLASSES opt";
        run_two($SRC_MKCLS, $TRG_MKCLS);
        # Create the vcb files and maps
        my $SRC_VCB = "$WORK_DIR/align/src.vcb"; 
        my %src_vcb = create_vcb($SRC_WORDS, $SRC_VCB);
        my $TRG_VCB = "$WORK_DIR/align/trg.vcb";
        my %trg_vcb = create_vcb($TRG_WORDS, $TRG_VCB);
        # Convert GIZA++ into snt format
        my $STPREF = "$WORK_DIR/align/src-trg";
        my $TSPREF = "$WORK_DIR/align/trg-src";
        create_snt($SRC_WORDS, \%src_vcb, $TRG_WORDS, \%trg_vcb, "$STPREF.snt");
        create_snt($TRG_WORDS, \%trg_vcb, $SRC_WORDS, \%src_vcb, "$TSPREF.snt");
        # Prepare the data for GIZA with snt2cooc
        safesystem("$SNT2COOC $SRC_VCB $TRG_VCB $STPREF.snt > $STPREF.cooc") or die;
        safesystem("$SNT2COOC $TRG_VCB $SRC_VCB $TSPREF.snt > $TSPREF.cooc") or die;
        # Run GIZA (in parallel?)
        my $GIZA_SRCTRG_CMD = "$GIZA -CoocurrenceFile $STPREF.cooc -c $STPREF.snt -m1 5 -m2 0 -m3 3 -m4 3 -model1dumpfrequency 1 -model4smoothfactor 0.4 -nodumps 1 -nsmooth 4 -o $STPREF.giza -onlyaldumps 1 -p0 0.999 -s $SRC_VCB -t $TRG_VCB";
        my $GIZA_TRGSRC_CMD = "$GIZA -CoocurrenceFile $TSPREF.cooc -c $TSPREF.snt -m1 5 -m2 0 -m3 3 -m4 3 -model1dumpfrequency 1 -model4smoothfactor 0.4 -nodumps 1 -nsmooth 4 -o $TSPREF.giza -onlyaldumps 1 -p0 0.999 -s $TRG_VCB -t $SRC_VCB";
        run_two($GIZA_SRCTRG_CMD, $GIZA_TRGSRC_CMD);
        # Symmetrize the alignments
        safesystem("$TRAVATAR_DIR/script/train/symmetrize.pl $WORK_DIR/align/src-trg.giza.A3.final $WORK_DIR/align/trg-src.giza.A3.final > $ALIGN_FILE") or die;
    } else {
        die "Unknown alignment type $ALIGN";
    }
}
exit(0) if $LAST_STEP eq "align";

# ****** 3: Create Lexical Translation Probabilities *******
print STDERR "(3) Creating lexical translation probabilities @ ".`date`;

# Create the lexical translation probabilities
if(not ($LEX_SRCTRG and $LEX_TRGSRC)) {
    safesystem("mkdir $WORK_DIR/lex") or die;
    # Write only the non-specified values
    my $WRITE_SRCTRG = ($LEX_SRCTRG ? "/dev/null" : "$WORK_DIR/lex/src_given_trg.lex");
    $LEX_SRCTRG = $WRITE_SRCTRG if not $LEX_SRCTRG;
    my $WRITE_TRGSRC = ($LEX_TRGSRC ? "/dev/null" : "$WORK_DIR/lex/trg_given_src.lex");
    $LEX_TRGSRC = $WRITE_TRGSRC if not $LEX_TRGSRC;
    # Run the program
    safesystem("$TRAVATAR_DIR/script/train/align2lex.pl $SRC_WORDS $TRG_WORDS $ALIGN_FILE $WRITE_SRCTRG $WRITE_TRGSRC") or die;
}
exit(0) if $LAST_STEP eq "lex";

# ****** 4: Create the model file *******
print STDERR "(4) Creating model @ ".`date`;

if(not $TM_FILE) {
    safesystem("mkdir $WORK_DIR/model") or die;
    # First extract the rules
    my $EXTRACT_FILE = "$WORK_DIR/model/extract.gz";
    if ($TRANSLATION_METHOD eq "t2s") {
        my $EXTRACT_OPTIONS = "-input_format $SRC_FORMAT -output_format $TRG_FORMAT -normalize_probs $NORMALIZE -binarize $BINARIZE -compose $COMPOSE -attach $ATTACH -attach_len $ATTACH_LEN -nonterm_len $NONTERM_LEN -term_len $TERM_LEN";
        safesystem("$TRAVATAR_DIR/src/bin/forest-extractor $EXTRACT_OPTIONS $SRC_FILE $TRG_FILE $ALIGN_FILE | gzip -c > $EXTRACT_FILE") or die;
    } else {
        my $EXTRACT_OPTIONS = "-initial_phrase_len $INITIAL_PHRASE_LEN -rule_max_len $RULE_MAX_LEN";
        safesystem("$TRAVATAR_DIR/src/bin/hiero-extractor $EXTRACT_OPTIONS $SRC_FILE $TRG_FILE $ALIGN_FILE | gzip -c > $EXTRACT_FILE") or die;
    }
    die;
    # Then, score the rules (in parallel?)
    my $RT_SRCTRG = "$WORK_DIR/model/rule-table.src-trg.gz"; 
    my $RT_TRGSRC = "$WORK_DIR/model/rule-table.trg-src.gz"; 
    my $RT_SRCTRG_CMD = "zcat $EXTRACT_FILE | LC_ALL=C sort | $TRAVATAR_DIR/script/train/score-t2s.pl $SCORE_OPTIONS --fof-file=$WORK_DIR/model/fof.txt --lex-prob-file=$LEX_TRGSRC | gzip > $RT_SRCTRG";
    my $RT_TRGSRC_CMD = "zcat $EXTRACT_FILE | $TRAVATAR_DIR/script/train/reverse-rt.pl | LC_ALL=C sort | $TRAVATAR_DIR/script/train/score-t2s.pl --lex-prob-file=$LEX_SRCTRG --prefix=fge | $TRAVATAR_DIR/script/train/reverse-rt.pl | LC_ALL=C sort | gzip > $RT_TRGSRC";
    run_two($RT_SRCTRG_CMD, $RT_TRGSRC_CMD);
    # Whether to create the model zipped or not
    my $zip_cmd;
    if($GZIP_TM eq "true") {
        $TM_FILE = "$WORK_DIR/model/rule-table.gz";
        $zip_cmd = "| gzip";
    } else {
        $TM_FILE = "$WORK_DIR/model/rule-table";
    }
    # Finally, combine the table
    safesystem("$TRAVATAR_DIR/script/train/combine-rt.pl --fof-file=$WORK_DIR/model/fof.txt --smooth=$SMOOTH --top-n=$NBEST_RULES $RT_SRCTRG $RT_TRGSRC $zip_cmd > $TM_FILE") or die;
}

# ******* 5: Create a configuration file ********
print STDERR "(5) Creating configuration @ ".`date`;

if(not $CONFIG_FILE) {
    (safesystem("mkdir $WORK_DIR/model") or die) if (not -e "$WORK_DIR/model");
    my $TINI_FILE = "$WORK_DIR/model/travatar.ini";
    open TINI, ">:utf8", $TINI_FILE or die "Couldn't open $TINI_FILE\n";
    print TINI "[tm_file]\n$TM_FILE\n\n";
    print TINI "[lm_file]\n$LM_FILE\n\n" if ($NO_LM ne "true");
    print TINI "[binarize]\n$BINARIZE\n\n";
    # Default values for the weights
    my $weights = "egfp=0.05\negfl=0.05\nfgep=0.05\nfgel=0.05\nlm=0.3\nw=0.3\np=-0.15\nunk=0\nlfreq=0.05\nparse=1\n";
    $weights .= "isx=-0.5\n" if ($TRG_FORMAT ne "word"); # Add syntax augmented weights
    print TINI "[weight_vals]\n$weights\n";
    close TINI;
    print "Finished training! You can find the configuation file in:\n$TINI_FILE\n";
}

# Finish training

################ Functions ##################

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

sub create_vcb {
    my ($words, $vcb) = @_;
    # Read and count the vcb
    open WORDS, "<:utf8", $words or die "Couldn't open $words\n";
    my %vals;
    while(<WORDS>) {
        chomp;
        for(to_words($_)) { $vals{$_}++; }
    }
    close WORDS;
    delete $vals{""};
    # Write the vcb
    open VCB, ">:utf8", $vcb or die "Couldn't open $vcb\n";
    print VCB "1\tUNK\t0\n";
    my $id=2;
    for(sort keys %vals) {
        printf VCB "%d\t%s\t%d\n",$id,$_,$vals{$_};
        $vals{$_} = $id++;
    }
    close VCB;
    return %vals;
}

sub create_snt {
    my ($src_wrd, $src_vcb, $trg_wrd, $trg_vcb, $out_file) = @_;
    open SRC, "<:utf8", $src_wrd or die "Couldn't open $src_wrd\n";
    open TRG, "<:utf8", $trg_wrd or die "Couldn't open $trg_wrd\n";
    open OUT, ">:utf8", $out_file or die "Couldn't open $out_file\n";
    my ($s, $t);
    while(1) {
        $s = <SRC>; $t = <TRG>;
        die "Uneven sentences in input and output" if(defined($s) != defined($t));
        last if not defined($s);
        chomp $s; chomp $t;
        print OUT "1\n".
                  join(" ", map { $src_vcb->{$_} ? $src_vcb->{$_} : 1 } to_words($s))."\n".
                  join(" ", map { $trg_vcb->{$_} ? $trg_vcb->{$_} : 1 } to_words($t))."\n";
    }
    close SRC; close TRG; close OUT;
}

sub run_two {
    @_ == 2 or die "run_two handles two commands, got @_\n";
    my ($CMD1, $CMD2) = @_;
    if($THREADS > 1) {
	    my $pid = fork();
	    die "ERROR: couldn't fork" unless defined $pid;
        if(!$pid) {
            safesystem("$CMD1") or die;
            exit 0;
        } else {
            safesystem("$CMD2") or die;
            waitpid($pid, 0);
        }
    } else {
        safesystem("$CMD1") or die;
        safesystem("$CMD2") or die;
    }
}

sub to_words {
    my $str = shift;
    $str =~ s/^ +//g;
    $str =~ s/ +$//g;
    return split(/ +/, $str);
}
