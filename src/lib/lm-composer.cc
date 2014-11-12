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

LMData::LMData(void * model, lm::ngram::ModelType type, VocabMap* vocab_map) :
        lm_feat_(Dict::WID("lm")), lm_unk_feat_(Dict::WID("lmunk")), 
        lm_(model), type_(type), vocab_map_(vocab_map), 
        lm_weight_(1), lm_unk_weight_(0), factor_(0) { }

LMData::~LMData() {
    if(lm_) {
        switch(type_) {
        case lm::ngram::PROBING:
            delete static_cast<lm::ngram::ProbingModel*>(lm_);
            break;
        case lm::ngram::REST_PROBING:
            delete static_cast<lm::ngram::RestProbingModel*>(lm_);
            break;
        case lm::ngram::TRIE:
            delete static_cast<lm::ngram::TrieModel*>(lm_);
            break;
        case lm::ngram::QUANT_TRIE:
            delete static_cast<lm::ngram::QuantTrieModel*>(lm_);
            break;
        case lm::ngram::ARRAY_TRIE:
            delete static_cast<lm::ngram::ArrayTrieModel*>(lm_);
            break;
        case lm::ngram::QUANT_ARRAY_TRIE:
            delete static_cast<lm::ngram::QuantArrayTrieModel*>(lm_);
            break;
        default:
            THROW_ERROR("Unrecognized kenlm model type " << type_);
        }
    }
    if(vocab_map_) delete vocab_map_;
}

lm::WordIndex LMData::GetMapping(WordId wid) const {
    VocabMap::const_iterator it = vocab_map_->find(wid);
    return it == vocab_map_->end() ? 0 : it->second;
}

LMData::LMData(const std::string & str) : 
        lm_feat_(Dict::WID("lm")), lm_unk_feat_(Dict::WID("lmunk")), lm_weight_(1), lm_unk_weight_(0), factor_(0) { 
    // Get the LM file name and parameters
    std::vector<std::string> cols = Tokenize(str, '|');
    if(cols.size() > 2)
        THROW_ERROR("Bad LM parameter string with two or more pipes:" << endl << str);
    // Load the parameters
    if(cols.size() > 1) {
        BOOST_FOREACH(std::string param, Tokenize(cols[1], ',')) {
            std::vector<std::string> kv = Tokenize(param, '=');
            if(kv.size() != 2)
                THROW_ERROR("Bad parameter \""<<param<<"\" in " << endl << str);
            if(kv[0] == "factor") {
                factor_ = boost::lexical_cast<int>(kv[1]);
                if(factor_ >= GlobalVars::trg_factors)
                    THROW_ERROR("LM factor ID " << factor_ << " is too large for number of target factors ("<<GlobalVars::trg_factors<<", with the first index at 0)");
            } else if(kv[0] == "lm_feat") {
                lm_feat_ = Dict::WID(kv[1]);
            } else if(kv[0] == "lm_unk_feat") {
                lm_unk_feat_ = Dict::WID(kv[1]);
            } else {
                THROW_ERROR("Bad parameter name " << kv[0] << " in " << endl << str);
            }
        }
    }
    // Create the LM, and an index mapping from travatar IDs to lm ids
    MapEnumerateVocab lm_save;
    lm::ngram::Config lm_config;
    lm_config.enumerate_vocab = &lm_save;
    if (!lm::ngram::RecognizeBinary(cols[0].c_str(), type_))
        type_ = lm::ngram::PROBING;
    switch(type_) {
    case lm::ngram::PROBING:
        lm_ = new lm::ngram::ProbingModel(cols[0].c_str(), lm_config);
        break;
    case lm::ngram::REST_PROBING:
        lm_ = new lm::ngram::RestProbingModel(cols[0].c_str(), lm_config);
        break;
    case lm::ngram::TRIE:
        lm_ = new lm::ngram::TrieModel(cols[0].c_str(), lm_config);
        break;
    case lm::ngram::QUANT_TRIE:
        lm_ = new lm::ngram::QuantTrieModel(cols[0].c_str(), lm_config);
        break;
    case lm::ngram::ARRAY_TRIE:
        lm_ = new lm::ngram::ArrayTrieModel(cols[0].c_str(), lm_config);
        break;
    case lm::ngram::QUANT_ARRAY_TRIE:
        lm_ = new lm::ngram::QuantArrayTrieModel(cols[0].c_str(), lm_config);
        break;
    default:
        THROW_ERROR("Unrecognized kenlm model type " << type_);
    }
    vocab_map_ = lm_save.GetAndFreeVocabMap();    
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
