include $(top_srcdir)/common.am
AM_CXXFLAGS += $(BOOST_CPPFLAGS) -I$(srcdir)/../include
LDADD=../lib/libtravatar.la ../kenlm/lm/libklm.la ../kenlm/util/libklm_util.la ../kenlm/search/libklm_search.la ../tercpp/libter.la ../marisa/libmarisa.la ../liblbfgs/liblbfgs.la $(BOOST_SYSTEM_LDFLAGS) $(BOOST_THREAD_LIBS) $(BOOST_REGEX_LIBS) $(BOOST_IOSTREAMS_LIBS) $(BOOST_SYSTEM_LIBS) $(BOOST_LOCALE_LIBS) -lz

bin_PROGRAMS = travatar forest-extractor batch-tune train-caser tree-converter mt-evaluator mt-segmenter rescorer hiero-extractor

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

train_caser_SOURCES = train-caser.cc
train_caser_LDADD = $(LDADD)
