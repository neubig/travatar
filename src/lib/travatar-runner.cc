#include <boost/shared_ptr.hpp>
#include <travatar/dict.h>
#include <travatar/util.h>
#include <travatar/tree-io.h>
#include <travatar/travatar-runner.h>
#include <travatar/lookup-table-hash.h>

using namespace travatar;
using namespace std;
using namespace boost;

// Run the model
void TravatarRunner::Run(const ConfigTravatarRunner & config) {
    // Load the rule table
    ifstream tm_in(config.GetString("tm_file").c_str());
    if(!tm_in)
        THROW_ERROR("Could not find TM: " << config.GetString("tm_file"));
    shared_ptr<LookupTableHash> tm(LookupTableHash::ReadFromRuleTable(tm_in));
    tm_in.close();
    // Load the weight file
    ifstream weight_in(config.GetString("weight_file").c_str());
    if(!weight_in)
        THROW_ERROR("Could not find weights: " << config.GetString("weight_file"));
    SparseMap weights = Dict::ParseFeatures(weight_in);
    weight_in.close();
    // Process one at a time
    int nbest_count = config.GetInt("nbest");
    int sent = 0;
    string line;
    PennTreeIO penn;
    while(getline(std::cin, line)) {
        istringstream in(line);
        shared_ptr<HyperGraph> tree_graph(penn.ReadTree(in));
        if(tree_graph.get() == NULL) break;
        shared_ptr<HyperGraph> rule_graph(tm->BuildRuleGraph(*tree_graph));
        rule_graph->ScoreEdges(weights);
        rule_graph->ResetViterbiScores();
        vector<shared_ptr<HyperPath> > nbest_list = rule_graph->GetNbest(nbest_count);
        if(nbest_count == 1) {
            cout << Dict::PrintWords(nbest_list[0]->CalcTranslation()) << endl;
        } else {
            BOOST_FOREACH(const shared_ptr<HyperPath> & path, nbest_list) {
                cout 
                    << sent
                    << " ||| " << Dict::PrintWords(path->CalcTranslation())
                    << " ||| " << path->GetScore()
                    << " ||| " << Dict::PrintFeatures(path->CalcFeatures()) << endl;
            }
        }
        sent++;
    }
}
