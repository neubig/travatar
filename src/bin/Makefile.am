AM_CPPFLAGS = -I$(srcdir)/../include

bin_PROGRAMS = travatar forest-extractor batch-tune tree-converter mt-evaluator

travatar_SOURCES = travatar.cc
travatar_LDADD = ../lib/libtravatar.la -lboost_thread -lboost_regex -lz -lmarisa

forest_extractor_SOURCES = forest-extractor.cc
forest_extractor_LDADD = ../lib/libtravatar.la -lboost_thread -lboost_regex -lz -lmarisa

tree_converter_SOURCES = tree-converter.cc
tree_converter_LDADD = ../lib/libtravatar.la -lboost_thread -lboost_regex -lz -lmarisa

mt_evaluator_SOURCES = mt-evaluator.cc
mt_evaluator_LDADD = ../lib/libtravatar.la -lboost_thread -lboost_regex -lz -lmarisa

batch_tune_SOURCES = batch-tune.cc
batch_tune_LDADD = ../lib/libtravatar.la -lboost_thread -lboost_regex -lz -lmarisa
