#include <travatar/lm-composer.h>
#include <travatar/dict.h>
#include <travatar/global-debug.h>
#include <travatar/string-util.h>
#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace travatar;

void MapEnumerateVocab::Add(lm::WordIndex index, const StringPiece &str) {
    vocab_map_->insert(std::make_pair(Dict::WID(str.as_string()), index));
}

LMComposer::LMComposer(const std::vector<std::string> & params) : lm_data_(), root_sym_(Dict::WID("LMROOT")) {
    BOOST_FOREACH(const std::string & param, params) {
        lm_data_.push_back(new LMData(param));
    }
}

LMComposer::~LMComposer() {
    BOOST_FOREACH(LMData* data, lm_data_)
        if(data) delete data;
}

void LMComposer::UpdateWeights(const SparseMap & weights) {
    BOOST_FOREACH(LMData* data, lm_data_) {
        SparseMap::const_iterator it1 = weights.find(data->GetFeatureName());
        data->SetWeight(it1 != weights.end() ? it1->second : 0);
        SparseMap::const_iterator it2 = weights.find(data->GetUnkFeatureName());
        data->SetUnkWeight(it2 != weights.end() ? it2->second : 0);
    }
}
