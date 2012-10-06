#include <boost/shared_ptr.hpp>
#include <boost/scoped_ptr.hpp>
#include <travatar/dict.h>
#include <travatar/util.h>
#include <travatar/tree-io.h>
#include <travatar/travatar-runner.h>
#include <travatar/lookup-table-hash.h>
#include <travatar/lookup-table-marisa.h>
#include <travatar/lm-composer-bu.h>
#include <travatar/binarizer-directional.h>
#include <travatar/binarizer-cky.h>
#include <lm/model.hh>

using namespace travatar;
using namespace std;
using namespace boost;
using namespace lm::ngram;

void TravatarTrainer::WriteTunedModelFile() {
    THROW_ERROR("Not implemented yet");
}


// Run the model
void TravatarTrainer::Run(const ConfigTravatarTrainer & config) {
    config_ = config;
    string str = SupplyConfig("tuned_model_file");
}
