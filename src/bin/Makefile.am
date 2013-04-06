AM_CPPFLAGS = -I$(srcdir)/../include

bin_PROGRAMS = travatar forest-extractor batch-tune tree-converter mt-evaluator

travatar_SOURCES = travatar.cc
travatar_LDADD = ../lib/libtravatar.la ../kenlm/lm/libklm.a ../kenlm/util/libklm_util.a -lboost_thread -lboost_regex -lz -lmarisa -lrt

forest_extractor_SOURCES = forest-extractor.cc
forest_extractor_LDADD = ../lib/libtravatar.la ../kenlm/lm/libklm.a ../kenlm/util/libklm_util.a -lboost_thread -lboost_regex -lz -lmarisa -lrt

tree_converter_SOURCES = tree-converter.cc
tree_converter_LDADD = ../lib/libtravatar.la ../kenlm/lm/libklm.a ../kenlm/util/libklm_util.a -lboost_thread -lboost_regex -lz -lmarisa -lrt

mt_evaluator_SOURCES = mt-evaluator.cc
mt_evaluator_LDADD = ../lib/libtravatar.la ../kenlm/lm/libklm.a ../kenlm/util/libklm_util.a -lboost_thread -lboost_regex -lz -lmarisa -lrt

batch_tune_SOURCES = batch-tune.cc
batch_tune_LDADD = ../lib/libtravatar.la ../kenlm/lm/libklm.a ../kenlm/util/libklm_util.a -lboost_thread -lboost_regex -lz -lmarisa -lrt
