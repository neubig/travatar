include $(top_srcdir)/common.am
AM_CXXFLAGS += -I$(srcdir)/../include $(BOOST_CPPFLAGS)
LDADD=../lib/libtravatar.la ../kenlm/lm/libklm.la ../kenlm/util/libklm_util.la ../kenlm/search/libklm_search.la ../tercpp/libter.la ../marisa/libmarisa.la ../liblbfgs/liblbfgs.la $(BOOST_LDFLAGS) $(BOOST_THREAD_LIB) $(BOOST_REGEX_LIB) $(BOOST_IOSTREAMS_LIB) $(BOOST_SYSTEM_LIB) $(BOOST_LOCALE_LIB) -lz -licui18n -licuuc -licudata -lrt

bin_PROGRAMS = travatar batch-tune forest-extractor hiero-extractor mt-evaluator mt-segmenter rescorer tokenizer train-caser tree-converter

travatar_SOURCES = travatar.cc
travatar_LDADD = $(LDADD)

batch_tune_LDADD = $(LDADD)
batch_tune_SOURCES = batch-tune.cc

forest_extractor_LDADD = $(LDADD)
forest_extractor_SOURCES = forest-extractor.cc

hiero_extractor_LDADD = $(LDADD)
hiero_extractor_SOURCES = hiero-extractor.cc

mt_evaluator_LDADD = $(LDADD)
mt_evaluator_SOURCES = mt-evaluator.cc

mt_segmenter_LDADD = $(LDADD)
mt_segmenter_SOURCES = mt-segmenter.cc

rescorer_LDADD = $(LDADD)
rescorer_SOURCES = rescorer.cc

tokenizer_LDADD = $(LDADD)
tokenizer_SOURCES = tokenizer.cc

train_caser_LDADD = $(LDADD)
train_caser_SOURCES = train-caser.cc

tree_converter_LDADD = $(LDADD)
tree_converter_SOURCES = tree-converter.cc
