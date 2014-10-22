#ifndef TRAVATAR_RUNNER_H__ 
#define TRAVATAR_RUNNER_H__

#include <travatar/task.h>
#include <travatar/output-collector.h>
#include <travatar/sparse-map.h>
#include <boost/shared_ptr.hpp>
#include <vector>

namespace travatar {

class ConfigTravatarRunner;
class HyperGraph;
class GraphTransformer;
class LookupTable;
class Weights;
class TravatarRunner;
class EvalMeasure;
typedef std::vector<int> Sentence;

class TravatarRunnerTask : public Task {
public:
    TravatarRunnerTask(int sent,
                       const boost::shared_ptr<HyperGraph> & tree_graph,
                       TravatarRunner * runner,
                       std::vector<Sentence> refs,
                       OutputCollector * collector,
                       OutputCollector * nbest_collector,
                       OutputCollector * trace_collector,
                       OutputCollector * forest_collector
                       )
        : sent_(sent), tree_graph_(tree_graph), refs_(refs), runner_(runner),
          collector_(collector), nbest_collector_(nbest_collector), 
          trace_collector_(trace_collector), forest_collector_(forest_collector) { }
    void Run();
private:
    int sent_; // ID of this sentence
    boost::shared_ptr<HyperGraph> tree_graph_; // The input
    std::vector<Sentence> refs_; // References
    TravatarRunner * runner_; // The runner holding all the information
    OutputCollector * collector_; // The output collector
    OutputCollector * nbest_collector_; // The output collector
    OutputCollector * trace_collector_; // The output collector
    OutputCollector * forest_collector_; // The output collector
};

class TravatarRunner {
public:

    TravatarRunner() { }
    ~TravatarRunner() { }
    
    // Run the model
    void Run(const ConfigTravatarRunner & config);
    
    // Getters/setters
    bool HasBinarizer() const { return binarizer_.get() != NULL; }
    const GraphTransformer & GetBinarizer() const { return *binarizer_; }
    bool HasTM() const { return tm_.get() != NULL; }
    const GraphTransformer & GetTM() const { return *tm_; }
    bool HasLM() const { return lms_.size() > 0; }
    const std::vector<boost::shared_ptr<GraphTransformer> > & GetLMs() const { return lms_; }
    bool HasTrimmer() const { return trimmer_.get() != NULL; }
    const GraphTransformer & GetTrimmer() const { return *trimmer_; }
    bool HasWeights() const { return weights_.get() != NULL; }
    const Weights & GetWeights() const { return *weights_; }
    Weights & GetWeights() { return *weights_; }
    bool HasEvalMeasure() const { return tune_eval_measure_.get() != NULL; }
    const EvalMeasure & GetEvalMeasure() const { return *tune_eval_measure_; }
    int GetNbestCount() const { return nbest_count_; }
    int GetThreads() const { return threads_; }
    bool GetDoTuning() const { return do_tuning_; } 

private:

    boost::shared_ptr<GraphTransformer> CreateLMComposer(
        const ConfigTravatarRunner & config,
        const std::vector<std::string> & lm_files,
        int pop_limit,
        const SparseMap & weights);

    boost::shared_ptr<GraphTransformer> binarizer_;
    boost::shared_ptr<GraphTransformer> tm_;
    std::vector<boost::shared_ptr<GraphTransformer> > lms_;
    boost::shared_ptr<GraphTransformer> trimmer_;
    boost::shared_ptr<Weights> weights_;
    boost::shared_ptr<EvalMeasure> tune_eval_measure_;
    int nbest_count_;
    int threads_;
    bool do_tuning_;


};

}

#endif

