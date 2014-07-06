#include <travatar/lm-composer.h>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace travatar;

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
    lm_ = new lm::ngram::Model(cols[0].c_str(), lm_config);
    vocab_map_ = lm_save.GetAndFreeVocabMap();    
}

LMComposer::LMComposer(const std::string & str) : lm_data_() {
    std::vector<std::string> params = Tokenize(str, ' ');
    BOOST_FOREACH(const std::string & param, params)
        lm_data_.push_back(new LMData(param));
}
