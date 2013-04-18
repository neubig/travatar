TRABATARTH = test-travatar.h test-base.h

AM_CPPFLAGS = -I$(srcdir)/../include -I$(srcdir)/../kenlm -I$(srcdir)/../marisa -DPKGDATADIR='"$(pkgdatadir)"'

noinst_PROGRAMS = test-travatar

nobase_include_HEADERS = test-alignment.h test-base.h test-binarizer.h test-dict.h test-eval-measure.h test-graph-transformer.h test-hyper-graph.h test-io-util.h test-lookup-table.h test-rule-extractor.h test-travatar-runner.h test-tree-io.h test-trimmer.h test-tune.h test-weights.h 

test_travatar_SOURCES = test-travatar.cc ${KYTH}
test_travatar_LDADD = ../lib/libtravatar.la ../kenlm/lm/libklm.a ../kenlm/util/libklm_util.a ../tercpp/libter.a ../marisa/libmarisa.la -lboost_thread -lboost_regex -lboost_iostreams -lz -lrt
