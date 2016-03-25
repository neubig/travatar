#include <boost/foreach.hpp>
#include <travatar/lm-func.h>
#include <travatar/global-debug.h>
#include <travatar/hyper-graph.h>
#include <travatar/string-util.h>
#include <travatar/dict.h>

using namespace travatar;
using namespace std;
using namespace lm::ngram;

LMFunc * LMFunc::CreateFromType(lm::ngram::ModelType type) {
    switch(type) {
    case lm::ngram::PROBING:
      return new LMFuncTemplate<lm::ngram::ProbingModel>();
    case lm::ngram::REST_PROBING:
      return new LMFuncTemplate<lm::ngram::RestProbingModel>();
    case lm::ngram::TRIE:
      return new LMFuncTemplate<lm::ngram::TrieModel>();
    case lm::ngram::QUANT_TRIE:
      return new LMFuncTemplate<lm::ngram::QuantTrieModel>();
    case lm::ngram::ARRAY_TRIE:
      return new LMFuncTemplate<lm::ngram::ArrayTrieModel>();
    case lm::ngram::QUANT_ARRAY_TRIE:
      return new LMFuncTemplate<lm::ngram::QuantArrayTrieModel>();
    default:
      THROW_ERROR("Unrecognized kenlm model type " << type);
    }
}

template <class LMType>
pair<Real,int> LMFuncTemplate<LMType>::CalcNontermScore(const LMData* data, const Sentence & syms, const std::vector<HyperNode*> & tails, const std::vector<std::vector<lm::ngram::ChartState> > & states, int lm_id, ChartState & out_state) {
    // Get the rule score for the appropriate model
    RuleScore<LMType> my_rule_score(*static_cast<const LMType*>(data->GetLM()), out_state);
    int unk = 0;
    BOOST_FOREACH(int trg_id, syms) {
        if(trg_id < 0) {
            int curr_id = -1 - trg_id;
            // Add that edge to our non-terminal
            const vector<ChartState> & child_state = states[tails[curr_id]->GetId()];
            // cerr << " Adding node context " << *next_edge->GetTail(curr_id) << " : " << PrintContext(child_state[lm_id].left) << ", " << PrintContext(child_state[lm_id].right) << endl;
            my_rule_score.NonTerminal(child_state[lm_id], 0);
        } else {
            // cerr << " Adding word " << Dict::WSym(trg_id) << endl;
            // Re-index vocabulary
            lm::WordIndex index = data->GetMapping(trg_id);
            if(index == 0) unk++;
            my_rule_score.Terminal(index);
        }
    }
    return make_pair(my_rule_score.Finish(), unk);
}

template <class LMType>
Real LMFuncTemplate<LMType>::CalcFinalScore(const void * lm, const ChartState & prev_state) {
    ChartState my_state;
    RuleScore<LMType> my_rule_score(*static_cast<const LMType*>(lm), my_state);
    my_rule_score.BeginSentence();
    my_rule_score.NonTerminal(prev_state, 0);
    my_rule_score.Terminal(static_cast<const LMType*>(lm)->GetVocabulary().Index("</s>"));
    return my_rule_score.Finish();
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
