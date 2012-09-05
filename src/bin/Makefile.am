AM_CPPFLAGS = -I$(srcdir)/../include

bin_PROGRAMS = travatar forest-extractor

travatar_SOURCES = travatar.cc
travatar_LDADD = ../lib/libtravatar.la -lboost_regex -lz -lmarisa

forest_extractor_SOURCES = forest-extractor.cc
forest_extractor_LDADD = ../lib/libtravatar.la -lboost_regex -lz -lmarisa
