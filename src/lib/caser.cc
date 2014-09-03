#include <travatar/caser.h>
#include <travatar/dict.h>
#include <travatar/hyper-graph.h>
#include <travatar/string-util.h>
#include <travatar/global-debug.h>

#include <boost/locale.hpp>
#include <boost/foreach.hpp>

#include <fstream>

using namespace std;

namespace travatar {

Caser::Caser() : type_(NONE) {
    loc_ = boost::locale::generator().generate("en_US.UTF-8");
    BOOST_FOREACH(const std::string & str, Tokenize(". : ! ?"))
        sentence_end_.insert(Dict::WID(str));
    BOOST_FOREACH(const std::string & str, Tokenize("( [ \" ' ` ``"))
        delayed_sentence_start_.insert(Dict::WID(str));

}

Caser::~Caser() { }

std::string Caser::ToLower(const std::string & wid) const {
    return boost::locale::to_lower(wid, loc_);
}
WordId Caser::ToLower(WordId wid) const {
    return Dict::WID(boost::locale::to_lower(Dict::WSym(wid), loc_));
}
void Caser::ToLower(Sentence & sent) const {
    for(int i = 0; i < (int)sent.size(); i++)
        sent[i] = ToLower(sent[i]);
}
void Caser::ToLower(HyperGraph & graph) const {
    ToLower(graph.GetWords());
    BOOST_FOREACH(HyperNode* node, graph.GetNodes()) {
        if(node->IsTerminal()) 
            node->SetSym(ToLower(node->GetSym()));
    }
}

std::string Caser::ToTitle(const std::string & wid) const {
    return boost::locale::to_title(wid, loc_);
}
WordId Caser::ToTitle(WordId wid) const {
    return Dict::WID(boost::locale::to_title(Dict::WSym(wid), loc_));
}
void Caser::ToTitle(Sentence & sent) const {
    vector<bool> first = SentenceFirst(sent);
    for(int i = 0; i < (int)first.size(); i++) {
        if(first[i])
            sent[i] = ToTitle(sent[i]);
    }
}
void Caser::ToTitle(HyperGraph & graph) const {
    ToTitle(graph.GetWords());
    vector<bool> first = SentenceFirst(graph.GetWords());
    BOOST_FOREACH(HyperNode* node, graph.GetNodes()) {
        if(node->IsTerminal() && first[node->GetSpan().first])
            node->SetSym(ToTitle(node->GetSym()));
    }
}

std::string Caser::TrueCase(const std::string & wid) const {
    boost::unordered_map<std::string, std::string>::const_iterator it = truecase_map_.find(ToLower(wid));
    return (it != truecase_map_.end()) ? it->second : wid;
}
WordId Caser::TrueCase(WordId wid) const {
    return Dict::WID(TrueCase(Dict::WSym(wid)));
}
void Caser::TrueCase(Sentence & sent) const {
    vector<bool> first = SentenceFirst(sent);
    for(int i = 0; i < (int)first.size(); i++) {
        if(first[i]) 
            sent[i] = TrueCase(sent[i]);
    }
}
void Caser::TrueCase(HyperGraph & graph) const {
    TrueCase(graph.GetWords());
    vector<bool> first = SentenceFirst(graph.GetWords());
    BOOST_FOREACH(HyperNode* node, graph.GetNodes()) {
        if(node->IsTerminal() && first[node->GetSpan().first])
            node->SetSym(TrueCase(node->GetSym()));
    }
}

std::vector<bool> Caser::SentenceFirst(const Sentence & sent) const {
    vector<bool> first(sent.size(), false);
    if(sent.size())
        first[0] = true;
    bool start = true;
    for(int i = 0; i < (int)sent.size(); i++) {
        if(sentence_end_.find(sent[i]) != sentence_end_.end()) {
            start = true;
        } else if(delayed_sentence_start_.find(sent[i]) == delayed_sentence_start_.end()) {
            first[i] = start;
            start = false;
        }
    }
    return first;
}

void Caser::AddTrueValue(const std::string & str) {
    truecase_map_[ToLower(str)] = str;
}

void Caser::LoadTrueCaseModel(const std::string & str) {
    ifstream in(str.c_str());
    if(!in)
        THROW_ERROR("Could not open truecasing model " << str);
    string line;
    truecase_map_.clear();
    while(getline(in, line))
        AddTrueValue(FirstToken(line));
}

HyperGraph * Caser::TransformGraph(const HyperGraph & graph) const {
    HyperGraph * ret = new HyperGraph(graph);
    switch(type_) {
        case LOW:
            ToLower(*ret);
            break;
        case TITLE:
            ToTitle(*ret);
            break;
        case TRUE:
            TrueCase(*ret);
            break;
        default:
            break;
    }
    return ret;
}

Caser * Caser::CreateCaserFromString(const std::string & str) {
    if(str == "none") 
        return NULL;
    Caser * caser = new Caser();
    if(str == "low") {
        caser->SetType(Caser::LOW);
    } else if(str == "title") {
        caser->SetType(Caser::TITLE);
    } else if(!str.compare(0, 11, "true:model=")) {
        string file = str.substr(11);
        caser->LoadTrueCaseModel(file);
        caser->SetType(Caser::TRUE);
    } else {
        THROW_ERROR("Bad format for caser (must be low/title/true:model=FILE)");
    }
    return caser;
}

} // namespace travatar
