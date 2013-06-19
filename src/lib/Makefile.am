include $(top_srcdir)/common.am
TRAVATARCPP = travatar.cc travatar-runner.cc dict.cc tree-io.cc hyper-graph.cc alignment.cc rule-extractor.cc forest-extractor-runner.cc lookup-table.cc lookup-table-hash.cc lookup-table-marisa.cc sparse-map.cc lm-composer-bu.cc binarizer-directional.cc binarizer-cky.cc rule-composer.cc thread-pool.cc weights.cc weights-pairwise.cc weights-perceptron.cc eval-measure-bleu.cc eval-measure-interp.cc eval-measure-ribes.cc eval-measure-ter.cc eval-measure-wer.cc util.cc io-util.cc translation-rule.cc rule-filter.cc config-base.cc tune.cc tune-mert.cc tune-greedy-mert.cc batch-tune-runner.cc tuning-example-nbest.cc tuning-example-forest.cc mert-geometry.cc eval-measure.cc trimmer.cc trimmer-nbest.cc tree-converter-runner.cc unary-flattener.cc mt-evaluator-runner.cc timer.cc input-file-stream.cc

AM_CPPFLAGS += -I$(srcdir)/../include -I$(srcdir)/../kenlm -I$(srcdir)/../tercpp -I$(srcdir)/../marisa

lib_LTLIBRARIES = libtravatar.la

libtravatar_la_SOURCES = ${TRAVATARCPP}
libtravatar_la_LDFLAGS = -version-info 0:0:0
