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
    // Load the phrase table
    ifstream tm_in(config.GetString("tm").c_str());
    if(!tm_in)
        THROW_ERROR("Could not find TM: " << config.GetString("tm"));
    shared_ptr<LookupTableHash> tm(LookupTableHash::ReadFromRuleTable(tm_in));
    // Process one at a time
    int nbest_count = config.GetInt("nbest");
    int sent = 0;
    string line;
    PennTreeIO penn;
    while(true) {
        shared_ptr<HyperGraph> tree_graph(penn.ReadTree(std::cin));
        if(tree_graph.get() == NULL) break;
        shared_ptr<HyperGraph> rule_graph(tm->BuildRuleGraph(*tree_graph));
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
