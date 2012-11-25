AM_CPPFLAGS = -I$(srcdir)/../include

bin_PROGRAMS = travatar forest-extractor greedy-mert

travatar_SOURCES = travatar.cc
travatar_LDADD = ../lib/libtravatar.la -lboost_thread -lboost_regex -lz -lmarisa

forest_extractor_SOURCES = forest-extractor.cc
forest_extractor_LDADD = ../lib/libtravatar.la -lboost_thread -lboost_regex -lz -lmarisa

greedy_mert_SOURCES = greedy-mert.cc
greedy_mert_LDADD = ../lib/libtravatar.la -lboost_thread -lboost_regex
