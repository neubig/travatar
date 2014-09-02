include $(top_srcdir)/common.am
#noinst_PROGRAMS = \
#  file_piece_test \
#  joint_sort_test \
#  key_value_packing_test \
#  probing_hash_table_test \
#  sorted_uniform_test

#TESTS = \
#  file_piece_test \
#  joint_sort_test \
#  key_value_packing_test \
#  probing_hash_table_test \
#  sorted_uniform_test

#file_piece_test_SOURCES = file_piece_test.cc
#file_piece_test_LDADD = libklm_util.a

lib_LTLIBRARIES = libklm_util.la

libklm_util_la_SOURCES = bit_packing.cc ersatz_progress.cc exception.cc file.cc file_piece.cc mmap.cc murmur_hash.cc pool.cc read_compressed.cc scoped.cc string_piece.cc usage.cc double-conversion/bignum.cc double-conversion/bignum-dtoa.cc double-conversion/cached-powers.cc double-conversion/diy-fp.cc double-conversion/double-conversion.cc double-conversion/fast-dtoa.cc double-conversion/fixed-dtoa.cc double-conversion/strtod.cc stream/chain.cc stream/io.cc stream/line_input.cc stream/multi_progress.cc

AM_CXXFLAGS += -W -Wall -Wno-sign-compare $(GTEST_CPPFLAGS) $(BOOST_CPPFLAGS) -I$(srcdir)/.. 
