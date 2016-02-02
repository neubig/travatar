include $(top_srcdir)/common.am
bin_PROGRAMS = query build_binary lmplz

LDLIBS = -lz $(BOOST_LDFLAGS) $(BOOST_PROGRAM_OPTIONS_LIB) $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB) -lpthread ${LIBRT}

query_SOURCES = query_main.cc
query_LDADD = libklm.la ../util/libklm_util.la ${LDLIBS}

build_binary_SOURCES = build_binary_main.cc
build_binary_LDADD = libklm.la ../util/libklm_util.la ${LDLIBS}

lmplz_SOURCES = builder/lmplz_main.cc
lmplz_LDADD = libklm.la ../util/libklm_util.la ${LDLIBS}

#noinst_PROGRAMS = \
#  ngram_test
#TESTS = ngram_test
#ngram_test_SOURCES = ngram_test.cc
#ngram_test_LDADD = ../util/libklm_util.la

lib_LTLIBRARIES = libklm.la

libklm_la_SOURCES = bhiksha.cc binary_format.cc builder/adjust_counts.cc builder/corpus_count.cc builder/initial_probabilities.cc builder/interpolate.cc builder/output.cc builder/pipeline.cc builder/print.cc config.cc filter/arpa_io.cc filter/phrase.cc filter/vocab.cc lm_exception.cc model.cc quantize.cc read_arpa.cc search_hashed.cc search_trie.cc sizes.cc trie.cc trie_sort.cc value_build.cc virtual_interface.cc vocab.cc

AM_CXXFLAGS += -W -Wall -Wno-sign-compare $(GTEST_CPPFLAGS) -I$(srcdir)/.. $(BOOST_CPPFLAGS)
