include $(top_srcdir)/common.am
AM_CXXFLAGS += -I$(srcdir)/../include
LDADD=../lib/libtravatar.la ../kenlm/lm/libklm.la ../kenlm/util/libklm_util.la ../kenlm/search/libklm_search.la ../tercpp/libter.la ../marisa/libmarisa.la ../liblbfgs/liblbfgs.la -lboost_thread-mt -lboost_regex-mt -lboost_iostreams-mt -lboost_system-mt -lz ${LIBRT}

bin_PROGRAMS = travatar forest-extractor batch-tune tree-converter mt-evaluator mt-segmenter rescorer hiero-extractor

travatar_SOURCES = travatar.cc
travatar_LDADD = $(LDADD)

forest_extractor_SOURCES = forest-extractor.cc
forest_extractor_LDADD = $(LDADD)

tree_converter_SOURCES = tree-converter.cc
tree_converter_LDADD = $(LDADD)

mt_evaluator_SOURCES = mt-evaluator.cc
mt_evaluator_LDADD = $(LDADD)

mt_segmenter_SOURCES = mt-segmenter.cc
mt_segmenter_LDADD = $(LDADD)

batch_tune_SOURCES = batch-tune.cc
batch_tune_LDADD = $(LDADD)

rescorer_SOURCES = rescorer.cc
rescorer_LDADD = $(LDADD)

hiero_extractor_SOURCES = hiero-extractor.cc
hiero_extractor_LDADD = $(LDADD)
