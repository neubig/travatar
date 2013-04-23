bin_PROGRAMS = build_binary lmplz

build_binary_SOURCES = build_binary_main.cc
build_binary_LDADD = libklm.a ../util/libklm_util.a -lz

lmplz_SOURCES = builder/lmplz_main.cc
lmplz_LDADD = libklm.a ../util/libklm_util.a -lz -lboost_program_options -lboost_thread

#noinst_PROGRAMS = \
#  ngram_test
#TESTS = ngram_test
#ngram_test_SOURCES = ngram_test.cc
#ngram_test_LDADD = ../util/libklm_util.a

noinst_LIBRARIES = libklm.a

libklm_a_SOURCES = bhiksha.cc binary_format.cc build_binary_main.cc config.cc fragment_main.cc lm_exception.cc model.cc quantize.cc query_main.cc read_arpa.cc search_hashed.cc search_trie.cc sizes.cc trie.cc trie_sort.cc value_build.cc virtual_interface.cc vocab.cc builder/adjust_counts.cc builder/corpus_count.cc builder/initial_probabilities.cc builder/interpolate.cc builder/pipeline.cc builder/print.cc 

AM_CPPFLAGS = -W -Wall -Wno-sign-compare $(GTEST_CPPFLAGS) -I$(srcdir)/..

