include $(top_srcdir)/common.am

noinst_LIBRARIES = libklm_search.a

libklm_search_a_SOURCES = edge_generator.cc nbest.cc rule.cc vertex.cc

AM_CXXFLAGS += -W -Wall -Wno-sign-compare $(GTEST_CPPFLAGS) -I$(srcdir)/.. 
