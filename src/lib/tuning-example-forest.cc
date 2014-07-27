#include <travatar/sparse-map.h>
#include <travatar/tuning-example-forest.h>
#include <travatar/global-debug.h>
#include <travatar/weights.h>
#include <travatar/dict.h>
#include <travatar/hyper-graph.h>
#include <travatar/mert-geometry.h>
#include <travatar/eval-measure.h>
#include <boost/foreach.hpp>
#include <cfloat>
#include <climits>
#include <algorithm>

using namespace std;
using namespace travatar;
using namespace boost;

// ~TuningExampleForest::TuningExampleForest() { }

// Calculate the n-best list giving the current weights
const std::vector<ExamplePair> & 
                   TuningExampleForest::CalculateNbest(const Weights & weights) {
    THROW_ERROR("Cannot yet calculate n-best from forests for tuning");
    return last_nbest_;
}

// Calculate the n-best list giving the current weights
const ExamplePair & 
                   TuningExampleForest::CalculateModelHypothesis(Weights & weights) const {
    THROW_ERROR("Cannot yet calculate model hypothesis from forests for tuning");
    return last_nbest_[0];
}

// This function combines multiple forests into a single one via a shared
// root node. This allows forests from multiple runs to be searched together
void TuningExampleForest::AddHypothesis(const shared_ptr<HyperGraph> & hg) {
    // Make the root node if necessary
    if(forest_.get() == NULL) {
        forest_.reset(new HyperGraph());
        forest_->AddNode(new HyperNode);
        forest_->SetWords(hg->GetWords());
    }
    // Append the forest, and add an edge connecting to the root node
    int id = forest_->Append(*hg);
    HyperNode *root = forest_->GetNode(0), *child = forest_->GetNode(id);
    HyperEdge *edge = new HyperEdge(root);
    edge->AddTail(child); 
    edge->SetTrgData(CfgDataVector(GlobalVars::trg_factors, CfgData(Sentence(1,-1))));
    root->AddEdge(edge); forest_->AddEdge(edge);
}

void TuningExampleForest::FindActiveFeatures() {
    active_.clear();
    BOOST_FOREACH(const HyperEdge * edge, forest_->GetEdges())
        BOOST_FOREACH(const SparsePair & feat, edge->GetFeatures().GetImpl())
            active_.insert(feat.first);
}

void TuningExampleForest::CalculateOracle() {
    try {
        CfgDataVector oracle_sent = measure_->CalculateOracle(*forest_, refs_);
        PRINT_DEBUG("Oracle sentence:" << endl << Dict::PrintWords(oracle_sent) << endl, 1);
        oracle_score_ = measure_->CalculateCachedStats(refs_, oracle_sent, id_)->ConvertToScore();
        PRINT_DEBUG("Oracle score: " << oracle_score_ << endl, 1);
        oracle_score_ *= mult_;
        oracle_score_ = 1;
    } catch (std::runtime_error & e) {
        PRINT_DEBUG("ERROR IN ORACLE CALCULATION " << e.what(), 0);
        oracle_score_ = mult_; // set to 1
    }
}

// Add weights
void TuningExampleForest::CountWeights(set<WordId> & weights) {
    BOOST_FOREACH(const HyperEdge * edge, forest_->GetEdges())
        BOOST_FOREACH(const SparsePair & feat, edge->GetFeatures().GetImpl())
            weights.insert(feat.first);
}

// Calculate the potential gain for a single example given the current weights
SparseMap TuningExampleForest::CalculatePotentialGain(const SparseMap & weights) {
    // Find the best according to the weights
    forest_->ResetViterbiScores();
    Weights wval(weights);
    forest_->ScoreEdges(wval);
    NbestList nbest_list = forest_->GetNbest(1);
    curr_score_ = measure_->CalculateCachedStats(refs_, nbest_list[0]->GetTrgData(), id_)->ConvertToScore() * mult_;
    // Find the potential gain
    oracle_score_ = max(oracle_score_, curr_score_);
    double gain = oracle_score_ - curr_score_;
    // Add this to all existing values
    if(active_.size() == 0)
        FindActiveFeatures();
    SparseMap ret;
    BOOST_FOREACH(WordId id, active_)
        ret.insert(make_pair(id, gain));
    return ret;
}

// Perform the inside step using memoized recursion
const MertHull & TuningExampleForest::CalculateMertHull(
                        const MertHullWeightFunction & func,
                        vector<shared_ptr<MertHull> > & hulls, 
                        int node_id) const {
    if(hulls[node_id].get() == NULL) {
        hulls[node_id].reset(new MertHull);
        BOOST_FOREACH(const HyperEdge * edge, forest_->GetNode(node_id)->GetEdges()) {
            MertHull my_hull = func(*edge);
            BOOST_FOREACH(const HyperNode * node, edge->GetTails())
                my_hull *= CalculateMertHull(func, hulls, node->GetId());
            *hulls[node_id] += my_hull;
        }
    }
    return *hulls[node_id];
}

inline double PosZero(double dub) { return (dub == -0.0 ? 0.0 : dub); }

// Get the convex hull, which consists of scored spans in order of the span location
ConvexHull TuningExampleForest::CalculateConvexHull(
                        const SparseMap & weights,
                        const SparseMap & gradient) const {
    ConvexHull ret;
    // Find if any features in the gradient are not active
    bool active = (active_.size() == 0);
    if(!active) {
        BOOST_FOREACH(const SparseMap::value_type & val, gradient) {
            set<WordId>::const_iterator it = active_.find(val.first);
            if(it != active_.end()) {
                active = true;
                break;
            }
        }
    }
    // Calculate the score of the current best hypothesis
    forest_->ResetViterbiScores();
    Weights wval(weights);
    forest_->ScoreEdges(wval);
    NbestList nbest_list = forest_->GetNbest(1);
    EvalStatsPtr curr_stats = measure_->CalculateCachedStats(refs_, nbest_list[0]->GetTrgData(), id_);
    curr_stats->TimesEquals(mult_);
    // If we are not active, return the simple convex hull
    if(!active) {
        ret.push_back(make_pair(make_pair(-DBL_MAX, DBL_MAX), curr_stats));
    // Otherwise, calculate the convex hull from the forest
    } else {
        vector<shared_ptr<MertHull> > hulls(forest_->NumNodes());
        MertHullWeightFunction func(weights, gradient);
        CalculateMertHull(func, hulls, 0);
        MertHull top_hull = *hulls[0];
        top_hull.Sort();
        PRINT_DEBUG("Hull for: " << Dict::PrintWords(refs_[0]) << endl, 6);
        for(int i = 0; i < (int)top_hull.size(); i++) {
            const MertLine & line = *top_hull.GetLines()[i];
            std::vector<Sentence> sent(GlobalVars::trg_factors);
            line.ConstructTranslation(forest_->GetWords(), &sent);
            EvalStatsPtr stats = measure_->CalculateCachedStats(refs_, sent, id_);
            double next = (i==(int)top_hull.size()-1 ? DBL_MAX : top_hull.GetLines()[i+1]->x);
            // If the score is exactly the same as last, just update the right side
            // if(ret.size()) 
            //     cerr << "DEBUGGING stats: " << *stats << ", " << *ret.rbegin()->second << endl;
            PRINT_DEBUG("Forest: " << *stats << "\t" << Dict::PrintWords(sent[0]) << endl, 6);
            if(ret.size() && (*ret.rbegin()->second == *stats)) {
                ret.rbegin()->first.second = PosZero(next)-DBL_MIN;
                continue;
            }
            // Otherwise, find the score
            stats->TimesEquals(mult_);
            if(PosZero(line.x) == 0) {
                // cerr << "l=" << PosZero(line.x)+DBL_MIN << ",r=" << PosZero(next)-DBL_MIN << ",s=" << curr_stats <<": @curr"<<endl;
                ret.push_back(make_pair(make_pair(-DBL_MIN,+DBL_MIN),curr_stats));
            }
            ret.push_back(make_pair(make_pair(PosZero(line.x)+DBL_MIN,PosZero(next)-DBL_MIN),stats));
            // cerr << "l=" << PosZero(line.x)+DBL_MIN << ",r=" << PosZero(next)-DBL_MIN << ",s=" << score<<": " <<Dict::PrintWords(sent)<<endl;
            // // *********** DEBUG *************
            // if(line.x < 0 && next > 0) {
            //     NbestList nbest_list = forest_->GetNbest(1, forest_->GetWords());
            //     Sentence act_sent = nbest_list[0]->GetWords();
            //     if(act_sent != sent) {
            //         THROW_ERROR("Actual sentence and constructed sentence don't match" << endl << Dict::PrintWords(act_sent) << endl << Dict::PrintWords(sent) << endl);
            //     }
            // }
            // // ************ END **************
        }
    }
    return ret;
}
