include $(top_srcdir)/common.am
TRAVATARCPP = alignment.cc batch-tune-runner.cc binarizer-cky.cc binarizer-directional.cc config-base.cc dict.cc eval-measure-bleu.cc eval-measure.cc eval-measure-interp.cc eval-measure-ribes.cc eval-measure-ter.cc eval-measure-wer.cc forest-extractor-runner.cc hyper-graph.cc input-file-stream.cc io-util.cc lm-composer-bu.cc lookup-table.cc lookup-table-hash.cc lookup-table-marisa.cc mert-geometry.cc mt-evaluator-runner.cc rule-composer.cc rule-extractor.cc rule-filter.cc sparse-map.cc thread-pool.cc timer.cc translation-rule.cc travatar.cc travatar-runner.cc tree-converter-runner.cc tree-io.cc trimmer.cc trimmer-nbest.cc tune.cc tune-greedy-mert.cc tune-mert.cc tune-online.cc tuning-example-forest.cc tuning-example-nbest.cc unary-flattener.cc util.cc weights.cc weights-pairwise.cc weights-perceptron.cc weights-average-perceptron.cc weights-adagrad.cc word-splitter.cc

AM_CXXFLAGS += -I$(srcdir)/../include -I$(srcdir)/../kenlm -I$(srcdir)/../tercpp -I$(srcdir)/../marisa

lib_LTLIBRARIES = libtravatar.la

libtravatar_la_SOURCES = ${TRAVATARCPP}
libtravatar_la_LDFLAGS = -version-info 0:0:0
