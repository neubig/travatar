#!/usr/bin/perl

use strict;
use warnings;
use utf8;
use Getopt::Long;
use List::Util qw(sum min max shuffle);
binmode STDIN, ":utf8";
binmode STDOUT, ":utf8";
binmode STDERR, ":utf8";

my $WORK_DIR = ""; # The working directory to use
my $TRAVATAR_DIR = ""; # The directory of travatar
my $BIN_DIR = ""; # A directory for external bin files (mainly GIZA)

my $SRC_FILE = ""; # The source file you want to train on
my $SRC_WORDS = ""; # A file of plain-text sentences from the source
my $SRC_FORMAT = "penn"; # The source file format (penn/egret)
my $SRC_CLASSES = ""; # Classes for source words in mkcls format

my $TRG_FILE = ""; # The target file you want to train on
my $TRG_FORMAT = "word"; # The target file format (word/penn/egret)
my $TRG_WORDS = ""; # A file of plain-text sentences from the target
my $TRG_CLASSES = ""; # Classes for source words in mkcls format

my $ALIGN_FILE = ""; # A file containing alignments
my $ALIGN = "giza"; # The type of alignment to use (giza)
my $SYMMETRIZE = "grow"; # The type of symmetrization to use (grow)
GetOptions(
    "work_dir=s" => \$WORK_DIR,
    "travatar_dir=s" => \$TRAVATAR_DIR,
    "bin_dir=s" => \$BIN_DIR,
    "src_file=s" => \$SRC_FILE,
    "src_words=s" => \$SRC_WORDS,
    "src_format=s" => \$SRC_FORMAT,
    "trg_file=s" => \$TRG_FILE,
    "trg_words=s" => \$TRG_WORDS,
    "trg_format=s" => \$TRG_FORMAT,
    "align_file=s" => \$ALIGN_FILE,
);
if(@ARGV != 0) {
    print STDERR "Usage: $0 --work-dir=work --src-file=src.txt --trg-file=trg.txt\n";
    exit 1;
}

# Sanity check!
((!$WORK_DIR) or (!$SRC_FILE) or (!$TRG_FILE) or (!$TRAVATAR_DIR) or (!$BIN_DIR)) and
    die "Must specify -work_dir ($WORK_DIR) -src_file ($SRC_FILE) -trg_file ($TRG_FILE) -travatar_dir ($TRAVATAR_DIR) -bin_dir ($BIN_DIR)";
((not -e $SRC_FILE) or (not -e $TRG_FILE) or (not -e $TRAVATAR_DIR)) and
    die "Could not find -src_file ($SRC_FILE) -trg_file ($TRG_FILE) or -travatar_dir ($TRAVATAR_DIR)";
((-e $WORK_DIR) or not safesystem("mkdir $WORK_DIR")) and
    die "Working directory $WORK_DIR already exists or could not be created";

# Steps:
# 1 -> Prepare Data
# 2 -> Create Alignments
# 3 -> Extract and Score Rule Table
# 4 -> Score Phrase Table

# ******** 1: Prepare Data **********
print STDERR "(1) Preparing data @ ".`date`;

# Convert trees into plain text sentences
safesystem("mkdir $WORK_DIR/data");
if(not $SRC_WORDS) {
    $SRC_WORDS = "$WORK_DIR/data/src.word";
    safesystem("$TRAVATAR_DIR/src/bin/tree-converter -input_format $SRC_FORMAT -output_format word < $SRC_FILE > $SRC_WORDS");
}
if(not $TRG_WORDS) {
    if($TRG_FORMAT eq "word") {
        $TRG_WORDS = $TRG_FILE;
    } else {
        $TRG_WORDS = "$WORK_DIR/data/trg.word";
        safesystem("$TRAVATAR_DIR/src/bin/tree-converter -input_format $TRG_FORMAT -output_format word < $TRG_FILE > $TRG_WORDS");
    }
}

# ****** 2: Create Alignments *******
print STDERR "(2) Creating alignments @ ".`date`;

# Alignment with GIZA++
if(not $ALIGN_FILE) {
    safesystem("mkdir $WORK_DIR/align");
    if($ALIGN eq "giza") {
        my $GIZA = "$BIN_DIR/GIZA++";
        my $SNT2COOC = "$BIN_DIR/snt2cooc.out";
        my $PLAIN2SNT = "$BIN_DIR/plain2snt.out";
        my $MKCLS = "$BIN_DIR/mkcls";
        ((not -x $GIZA) or (not -x $SNT2COOC)) and
            die "Could not execute GIZA ($GIZA) or snt2cooc ($SNT2COOC)";
        # Make the classes with mkcls
        if(!$SRC_CLASSES) {
            $SRC_CLASSES = "$WORK_DIR/align/src.cls";
            safesystem("$MKCLS -c50 -n2 -p$SRC_WORDS -V$SRC_CLASSES opt");
        }
        if(!$TRG_CLASSES) {
            $TRG_CLASSES = "$WORK_DIR/align/src.cls";
            safesystem("$MKCLS -c50 -n2 -p$TRG_WORDS -V$TRG_CLASSES opt");
        }
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
        safesystem("$SNT2COOC $SRC_VCB $TRG_VCB $STPREF.snt > $STPREF.cooc");
        safesystem("$SNT2COOC $TRG_VCB $SRC_VCB $TSPREF.snt > $TSPREF.cooc");
        # Run GIZA
        safesystem("$GIZA -CoocurrenceFile $STPREF.cooc -c $STPREF.snt -m1 5 -m2 0 -m3 3 -m4 3 -model1dumpfrequency 1 -model4smoothfactor 0.4 -nodumps 1 -nsmooth 4 -o $STPREF.giza -onlyaldumps 1 -p0 0.999 -s $SRC_VCB -t $TRG_VCB");
        safesystem("$GIZA -CoocurrenceFile $TSPREF.cooc -c $TSPREF.snt -m1 5 -m2 0 -m3 3 -m4 3 -model1dumpfrequency 1 -model4smoothfactor 0.4 -nodumps 1 -nsmooth 4 -o $TSPREF.giza -onlyaldumps 1 -p0 0.999 -s $TRG_VCB -t $SRC_VCB");
        # Symmetrize the alignments
        safesystem("$TRAVATAR_DIR/script/train/symmetrize.pl HERE HERE");
        
    } else {
        die "Unknown alignment type $ALIGN";
    }
}

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
        for(split(/ +/)) { $vals{$_}++; }
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
                  join(" ", map { $src_vcb->{$_} ? $src_vcb->{$_} : 1 } split(/ /, $s))."\n".
                  join(" ", map { $trg_vcb->{$_} ? $trg_vcb->{$_} : 1 } split(/ /, $t))."\n";
    }
    close SRC; close TRG; close OUT;
}
