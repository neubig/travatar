AM_CPPFLAGS = -I$(srcdir)/../include
LDADD=../lib/libtravatar.la ../kenlm/lm/libklm.a ../kenlm/util/libklm_util.a ../tercpp/libter.a ../marisa/libmarisa.la -lboost_thread -lboost_regex -lboost_iostreams -lz -lrt

bin_PROGRAMS = travatar forest-extractor batch-tune tree-converter mt-evaluator

travatar_SOURCES = travatar.cc
travatar_LDADD = $(LDADD)

forest_extractor_SOURCES = forest-extractor.cc
forest_extractor_LDADD = $(LDADD)

tree_converter_SOURCES = tree-converter.cc
tree_converter_LDADD = $(LDADD)

mt_evaluator_SOURCES = mt-evaluator.cc
mt_evaluator_LDADD = $(LDADD)

batch_tune_SOURCES = batch-tune.cc
batch_tune_LDADD = $(LDADD)
