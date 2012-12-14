AM_CPPFLAGS = -I$(srcdir)/../include

bin_PROGRAMS = travatar forest-extractor batch-tune

travatar_SOURCES = travatar.cc
travatar_LDADD = ../lib/libtravatar.la -lboost_thread -lboost_regex -lz -lmarisa

forest_extractor_SOURCES = forest-extractor.cc
forest_extractor_LDADD = ../lib/libtravatar.la -lboost_thread -lboost_regex -lz -lmarisa

batch_tune_SOURCES = batch-tune.cc
batch_tune_LDADD = ../lib/libtravatar.la -lboost_thread -lboost_regex -lz -lmarisa
