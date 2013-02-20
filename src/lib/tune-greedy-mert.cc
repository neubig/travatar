#include <cfloat>
#include <map>
#include <boost/foreach.hpp>
#include <boost/shared_ptr.hpp>
#include <travatar/tuning-example.h>
#include <travatar/tune-mert.h>
#include <travatar/tune-greedy-mert.h>
#include <travatar/util.h>
#include <travatar/dict.h>
#include <travatar/eval-measure.h>
#include <travatar/sparse-map.h>
#include <travatar/output-collector.h>

using namespace std;
using namespace boost;
using namespace travatar;

#define MARGIN 1

void GreedyMertTask::Run() {
    // Check to make sure that our potential is still higher than the current best
    double best = tgm_->GetBestGain();
    if((potential_ <= best) ||
       (tgm_->GetEarlyTerminate() && best >= tgm_->GetGainThreshold()))
        return;
    // Mak the gradient and find the ranges
    SparseMap gradient;
    gradient[feature_] = 1;
    pair<double,double> gradient_range = tgm_->FindGradientRange(feature_);
    LineSearchResult result = TuneMert::LineSearch(tgm_->GetWeights(), gradient, tgm_->GetExamples(), gradient_range);
    if(result.gain > best) {
        tgm_->UpdateBest(gradient, result);
        best = result.gain;
    }
    ostringstream oss;
    oss << "gain?("<<Dict::WSym(feature_)<<")=" << potential_ << " --> gain@" << result.pos <<"="<< result.gain << ", score="<<result.before->ConvertToString()<<"-->"<<result.after->ConvertToString()<<" (max: " << best << ")" << endl;
    if(collector_)
        collector_->Write(id_, "", oss.str());
    else
        cerr << oss.str();
}

void TuneGreedyMert::UpdateBest(const SparseMap &gradient, const LineSearchResult &result) {
    mutex::scoped_lock lock(result_mutex_);
    if(best_result_.gain < result.gain) {
        best_gradient_ = gradient;
        best_result_ = result;
        cerr << "NEW MAX: " << Dict::WSym(gradient.begin()->first) << "=" << result.gain << endl;
    }
}

pair<double,double> TuneGreedyMert::FindGradientRange(WordId feat) {
    RangeMap::const_iterator it = ranges_.find(feat);
    pair<double,double> range = (it == ranges_.end() ? ranges_[-1] : it->second);
    SparseMap gradient; gradient[feat] = 1;
    return FindGradientRange(weights_, gradient, range);
}

// Current value can be found here
// range(-2,2)
// original(-1)
// change(-1,3)
// gradient -2
// +0.5, -1.5
pair<double,double> TuneGreedyMert::FindGradientRange(
                                const SparseMap & weights,
                                const SparseMap & gradient,
                                pair<double,double> range) {
    pair<double,double> ret(-DBL_MAX, DBL_MAX);
    BOOST_FOREACH(const SparseMap::value_type & grad, gradient) {
        if(grad.second == 0) continue;
        SparseMap::const_iterator it = weights.find(grad.first);
        double w = (it != weights.end()) ? it->second : 0.0;
        double l = (range.first-w)/grad.second;
        double r = (range.second-w)/grad.second;
        if(l > r) { double temp = l; l = r; r = temp; }
        ret.first = max(l, ret.first);
        ret.second = min(r, ret.second);
    }
    return ret; 
}

// Find the best value to tune and tune it
double TuneGreedyMert::TuneOnce() {
    best_result_ = LineSearchResult();
    best_gradient_ = SparseMap();
    PRINT_DEBUG("Calculating potential gains..." << endl, 1);
    SparseMap potential;
    BOOST_FOREACH(const shared_ptr<TuningExample> & examp, examps_) {
        potential += examp->CalculatePotentialGain(weights_);
    }
    // Order the weights in descending order
    typedef pair<double,int> DIPair;
    vector<DIPair> vals;
    BOOST_FOREACH(const SparseMap::value_type val, potential)
        if(val.second >= gain_threshold_)
            vals.push_back(make_pair(val.second, val.first));
    sort(vals.begin(), vals.end());

    // Create the threads
    shared_ptr<ThreadPool> thread_pool;
    shared_ptr<OutputCollector> out_collect;
    int task_id = 0;
    if(threads_) {
        thread_pool.reset(new ThreadPool(threads_, 1000));
        out_collect.reset(new OutputCollector);
    }

    // Dispatch jobs until the best value exceeds the expected value
    BOOST_REVERSE_FOREACH(const DIPair & val, vals) {
        if((val.first < best_result_.gain) || 
           (early_terminate_ && best_result_.gain >= gain_threshold_))
            break;
        GreedyMertTask* task = new GreedyMertTask(task_id++, *this, val.second, val.first, out_collect.get());
        // If the threads are not correct
        if(thread_pool)
            thread_pool->Submit(task);
        else
            task->Run();
    }
    if(thread_pool)
        thread_pool->Stop(true);

    // Update with the best value
    if(best_result_.gain > gain_threshold_) {
        PRINT_DEBUG("Updating: " << Dict::PrintFeatures(best_gradient_) << " * " << best_result_.pos << endl, 0);
        weights_ += best_gradient_ * best_result_.pos;
    }
    PRINT_DEBUG("Features: " << Dict::PrintFeatures(weights_) << endl, 0);
    return best_result_.gain;
}

// Tune new weights using greedy mert until the threshold is exceeded
void TuneGreedyMert::Tune() {
    while (TuneOnce() > gain_threshold_);
}

double TuneGreedyMert::GetBestGain() const {
    return best_result_.gain;
}
