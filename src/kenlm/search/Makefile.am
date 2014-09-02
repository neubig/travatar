include $(top_srcdir)/common.am

lib_LTLIBRARIES = libklm_search.la

libklm_search_la_SOURCES = edge_generator.cc nbest.cc rule.cc vertex.cc

AM_CXXFLAGS += -W -Wall -Wno-sign-compare $(GTEST_CPPFLAGS) $(BOOST_CPPFLAGS) -I$(srcdir)/.. 
