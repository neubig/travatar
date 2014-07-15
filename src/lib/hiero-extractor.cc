
#include <travatar/hiero-extractor.h>
#include <travatar/alignment.h>
#include <travatar/util.h>
#include <vector>

using namespace std;
using namespace travatar;

//////////////////////////////////////////
//        HIERO RULE EXTRACTOR          //
//////////////////////////////////////////

bool HieroExtractor::IsRuleValid(HieroRule* rule, int max_terminals) {
    if (!HieroRule::IsRuleBalanced(rule)) {
        THROW_ERROR("The number of NT in src and trg is not balance in rule: " << rule->ToString());
    }
    unsigned int nterm = static_cast<unsigned int>(rule->GetNumberOfNT());
    if (rule->GetSrcSent().size()-nterm > static_cast<unsigned int>(max_terminals) || 
            rule->GetTrgSent().size()-nterm > static_cast<unsigned int>(max_terminals)) 
    {
        return 0;   
    }
     // RULE CONTAINS ALL NON TERMINAL FILTER
    if (rule->GetSrcSent().size() == nterm || rule->GetTrgSent().size() == nterm) {
        return 0;
    }
    if (HieroRule::IsNTSideBySide(rule)) {
        return 0;
    }
    return 1;
}

// Public 
vector< vector<HieroRule*> > HieroExtractor::ExtractHieroRule(const Alignment & align, const Sentence & source, 
        const Sentence & target) const
{
    const SourceAlignment & src_align = align.GetSrcAlignments();
    vector< vector<HieroRule*> > ret;
    HieroExtractor::PhrasePairs filtered_pairs;
    HieroExtractor::PhrasePairs pairs;

    // Doing extraction safely
    try {
        pairs = ExtractPhrase(src_align,source, target);
    } catch (exception& exc) {
        THROW_ERROR("Input or alignment error. \n\tOn Src: " + Dict::PrintWords(source) + 
            "\n\tOn Trg: " + Dict::PrintWords(target));
    }
    // if there are multiple initial phrase pairs containing the same set of alignments, only the 
    // smallest is kept. That is, unaligned words are not allowed at the edges of phrases
    map<pair<int,int>, int> mp = map<pair<int,int>,int>();
    BOOST_FOREACH(HieroExtractor::PhrasePair pp, pairs) {
        pair<int,int> src = pp.first;
        pair<int,int> trg = pp.second;
        pair<int,int> index = make_pair(src.first,src.second);
        int len = trg.second-trg.first;

        if (mp.find(index)==mp.end() || mp[index] > len) {
            mp[index] = len;
        }
    }
    
    BOOST_FOREACH(PhrasePair pp, pairs) {
        pair<int,int> src = pp.first;
        pair<int,int> trg = pp.second;
        pair<int,int> index = make_pair(src.first,src.second);
        int len = trg.second-trg.first;
        if (len == mp[index]) {
            filtered_pairs.push_back(pp);
        }
    }
    pairs = filtered_pairs;
    // Rule extraction algorithm for 1 and 2 Nonterminal Symbols.
    for (int ii=0; ii < (int)pairs.size(); ++ii) {
        vector<HieroRule*> extracted;
        // Phrases
        HieroRule *rule = ParseLexicalTranslationRule(source,target,pairs[ii],src_align);
        if(HieroExtractor::IsRuleValid(rule,max_terminals_)) {
            extracted.push_back(rule);
        } else {
            delete rule;
        }
        for (int jj=0; jj < (int)pairs.size(); ++jj) {
            if (jj == ii || !InPhrase(pairs[jj],pairs[ii])) 
                continue;
            // Unary rule
            rule = ParseUnaryPhraseRule(source,target,pairs[jj],pairs[ii],src_align);
            if(HieroExtractor::IsRuleValid(rule,max_terminals_)) {
                extracted.push_back(rule);
            } else {
                delete rule;
            }
            // Binary rule
            for (int kk=jj+1; kk < (int)pairs.size(); ++kk) {
                // that are in the span of INITIAL phrase, and NOT overlapping each other
                if (kk == jj || !InPhrase(pairs[kk],pairs[ii]) || !InPhrase(pairs[jj],pairs[ii]) || IsPhraseOverlapping(pairs[jj],pairs[kk])) 
                    continue;
                rule = ParseBinaryPhraseRule(source,target,pairs[jj],pairs[kk],pairs[ii],src_align);
                if(HieroExtractor::IsRuleValid(rule,max_terminals_)) {
                    extracted.push_back(rule);
                } else {
                    delete rule;
                }
            }
        }
        if (extracted.size() > static_cast<unsigned int>(0)) {
            ret.push_back(extracted);
        }
    }
    // DEBUG PrintPhrasePairs();
    return ret;
}

// The implementation of phrase extraction algorithm.
// The algorithm to extract all consistent phrase pairs from a word-aligned sentence pair
HieroExtractor::PhrasePairs HieroExtractor::ExtractPhrase(const SourceAlignment & s2t, const Sentence & source, 
        const Sentence & target) const
{
    vector<set<int> > t2s = vector<set<int> >();
    HieroExtractor::PhrasePairs ret = HieroExtractor::PhrasePairs();

    // Remap from source -> target back to target -> source
    for (unsigned t=0; t < target.size(); ++t) {
        t2s.push_back(set<int>());
    }
    for (unsigned s=0; s < s2t.size(); ++s) {
        set<int> ts = s2t[s];
        BOOST_FOREACH(int t, ts)
            t2s[t].insert((int)s);
    }

    // Phrase Extraction Algorithm 
    for (int s_begin=0; s_begin < (int)s2t.size(); ++s_begin) {
        map<int, int> tp;
        int s_last = min((int)s2t.size(), s_begin+max_initial_phrase_);
        for (int s_end=s_begin; s_end < s_last; ++s_end) {
            if (s2t[s_end].size() != 0) {
                BOOST_FOREACH(int _t, s2t[s_end])
                    tp[_t]++;
            }
            int t_begin = MapMinKey(tp);
            int t_end = MapMaxKey(tp);
            if (QuasiConsecutive(t_begin,t_end,tp,t2s)) {
                map<int, int> sp;
                for (int t=t_begin; t<=t_end;++t) 
                    BOOST_FOREACH(int _s, t2s[t])
                        sp[_s]++;
                if (MapMinKey(sp) >= s_begin && MapMaxKey(sp) <= s_end) {
                    int t_first = max(0, t_end-max_initial_phrase_+1);
                    while(t_begin >= t_first) {
                        int t_last = min((int)t2s.size(), t_begin+max_initial_phrase_);
                        for(int jp = t_end; jp < t_last && (jp == t_end || t2s[jp].size() == 0); jp++)
                            ret.push_back(make_pair(make_pair(s_begin,s_end),make_pair(t_begin,jp)));
                        --t_begin;
                        if(t_begin < 0 || t2s[t_begin].size() != 0)
                            break;
                    }
                }
            }
        }
    }
    return ret;
}

// Private Member
string HieroExtractor::AppendString(const Sentence & s, const int begin, const int end) 
{
    string ret = string("");
    for (int i=begin; i<=(int)end; ++i) {
        ret += Dict::WSym(s[i]);
        if (i != end) {
            ret += " ";
        }
    }
    return ret;
}

string HieroExtractor::PrintPhrasePair(const HieroExtractor::PhrasePair & pp, const Sentence & source, 
    const Sentence & target) 
{
    return AppendString(source, pp.first.first, pp.first.second) + string(" ||| ") 
        + AppendString(target, pp.second.first, pp.second.second);
}

void HieroExtractor::PrintPhrasePairs(const HieroExtractor::PhrasePairs & pairs, const Sentence & source, 
    const Sentence & target) 
{
    BOOST_FOREACH(HieroExtractor::PhrasePair pp , pairs) {
        cerr << PrintPhrasePair(pp,source,target) <<endl;
    }
}

int HieroExtractor::MapMaxKey(const map<int,int> & map) {
    int max = 0;
    pair<int,int> item;
    BOOST_FOREACH(item, map) {
        if (item.first > max) max = item.first;
    } 
    return max;
}

int HieroExtractor::MapMinKey(const map<int,int> & map) {
    int min = 0;
    int is_first = 1;
    pair<int,int> item;
    BOOST_FOREACH(item, map) {
        if (is_first || item.first < min) {
            is_first = 0;
            min = item.first;
        }
    } 
    return min;
}

int HieroExtractor::QuasiConsecutive(int small, int large, const map<int,int> & tp, 
        const vector<set<int> > & t2s)
{
    for (int i=small; i <= large; ++i) {
        if (t2s[i].size() != 0 && tp.find(i) == tp.end()) {
            return 0;
        }
    }   
    return 1;
}

int HieroExtractor::IsTerritoryOverlapping(const pair<int,int> & a, const pair<int,int> & b)  
{
    return 
        (a.first >= b.first && a.second <= b.second) || // a in b
        (b.first >= a.first && b.second <= a.second) || // b in a
        (a.first <= b.first && a.second >= b.first)  || // a preceeds b AND they are overlapping
        (b.first <= a.first && b.second >= a.first);    // b preceeds a AND they are overlapping
}

int HieroExtractor::IsPhraseOverlapping(const HieroExtractor::PhrasePair & pair1, const HieroExtractor::PhrasePair & pair2)  
{
    return IsTerritoryOverlapping(pair1.first, pair2.first) || IsTerritoryOverlapping(pair1.second,pair2.second);
}

void HieroExtractor::ParseRuleWith1Nonterminals(
        const Sentence & sentence, 
        const pair<int,int> & my_pair, 
        const pair<int,int> & pair_span, 
        HieroRule* target, 
        const int type,
        const SourceAlignment & align) 
{
    target->SetType(type);
    int x = 0;
    for (int i=pair_span.first; i <= pair_span.second; ++i) {
        if (i >= my_pair.first && i <= my_pair.second) {
            x = 1;
        } else {
            target->SetPenalty(i);
            if (x) target->AddNontermX(--x);
            target->AddWord(make_pair(i,sentence[i]));
            if (type == HIERO_SRC) {
                BOOST_FOREACH(int j, align[i]) {
                    target->AddAlignment(i,j);
                }
            }
        } 
    }
    if (x) target->AddNontermX(0);
}

void HieroExtractor::ParseRuleWith2Nonterminals(
        const Sentence & sentence,
        const pair<int,int> & pair1, 
        const pair<int,int> & pair2, 
        const pair<int,int> & pair_span, 
        HieroRule* target, 
        const int type,
        const SourceAlignment & align) 
{
    target->SetType(type);
    int x0 = 0;
    int x1 = 0;
    for (int i=pair_span.first; i <= pair_span.second; ++i) {
        if (i >= pair1.first && i <= pair1.second) {
            if (x1) target->AddNontermX(x1--);
            x0 = 1;
        } else if (i >= pair2.first && i <= pair2.second) {
            if (x0) target->AddNontermX(--x0);
            x1 = 1;
        } else {
            target->SetPenalty(i);
            if (x0) target->AddNontermX(--x0);
            if (x1) target->AddNontermX(x1--);
            target->AddWord(make_pair(i,sentence[i]));
            if (type == HIERO_SRC) {
                BOOST_FOREACH(int j, align[i]) {
                    target->AddAlignment(i,j);
                }
            }
        } 
    }
    if (x0) target->AddNontermX(0);
    else if (x1) target->AddNontermX(1);
}

HieroRule* HieroExtractor::ParseLexicalTranslationRule(
        const Sentence & source, 
        const Sentence & target, 
        const HieroExtractor::PhrasePair & my_pair,
        const SourceAlignment & align)
{
    HieroRule* rule = new HieroRule;
    rule->SetType(HIERO_SRC);
    for (int i=my_pair.first.first; i <= my_pair.first.second; ++i) {
        rule->AddWord(make_pair(i,source[i]));
        BOOST_FOREACH(int j, align[i]) {
            rule->AddAlignment(i,j);
        }
    }
    rule->SetPenalty(my_pair.first.first);
    rule->SetType(HIERO_TRG);
    for (int i=my_pair.second.first; i <= my_pair.second.second; ++i) 
        rule->AddWord(make_pair(i,target[i]));
    rule->SetPenalty(my_pair.second.first);
    rule->ShiftAlignment();
    return rule;
}

HieroRule* HieroExtractor::ParseUnaryPhraseRule(
        const Sentence & source, 
        const Sentence & target, 
        const HieroExtractor::PhrasePair & my_pair, 
        const HieroExtractor::PhrasePair & pair_span,
        const SourceAlignment & align) 
{
    HieroRule* rule = new HieroRule;
    ParseRuleWith1Nonterminals(source,my_pair.first,pair_span.first,rule,HIERO_SRC,align);
    ParseRuleWith1Nonterminals(target,my_pair.second,pair_span.second,rule,HIERO_TRG,align);
    rule->ShiftAlignment();
    return rule;
}

HieroRule* HieroExtractor::ParseBinaryPhraseRule(
        const Sentence & source, 
        const Sentence & target, 
        const HieroExtractor::PhrasePair & pair1, 
        const HieroExtractor::PhrasePair & pair2, 
        const HieroExtractor::PhrasePair & pair_span,
        const SourceAlignment & align) 
{
    HieroRule* rule = new HieroRule;
    ParseRuleWith2Nonterminals(source,pair1.first,pair2.first,pair_span.first,rule,HIERO_SRC,align);
    ParseRuleWith2Nonterminals(target,pair1.second,pair2.second,pair_span.second,rule,HIERO_TRG,align);
    rule->ShiftAlignment();
    return rule;
}

int HieroExtractor::InPhrase(
        const HieroExtractor::PhrasePair & p1, 
        const HieroExtractor::PhrasePair & p2) {
    return p1.first.first >= p2.first.first && p1.first.second <= p2.first.second && 
        p1.second.first >= p2.second.first && p1.second.second <= p2.second.second;
}

////////////////////////
///// HIERO RULE ///////
////////////////////////
void HieroRule::AddWord(pair<int,WordId> word, int non_term) {
    if (type_ == HIERO_SRC) {
        if (non_term) src_nt_position_.insert(src_words_.size());
        src_words_.push_back(word.second);
        src_index_.push_back(word.first);
    } else if (type_ == HIERO_TRG) {
        if (non_term) trg_nt_position_.insert(trg_words_.size());
        trg_words_.push_back(word.second);
        trg_index_.push_back(word.first);
    }
}

void HieroRule::AddNontermX(int number) {
    AddWord(make_pair(-1,-1-number),1);
}

std::string HieroRule::ToString() const {
    std::ostringstream ss;
    for (int i=0; i < (int)src_words_.size(); ++i) {
        if (i) ss << " ";
        if (src_nt_position_.find(i) == src_nt_position_.end()) {
            ss << "\"" << Dict::WSym(src_words_[i])<< "\"";
        } else {
            ss << "x" << -1-src_words_[i] << ":X";
        }
    }
    ss << " @ X";
    ss << " ||| ";
    for (int i=0; i < (int)trg_words_.size(); ++i) {
        if (i) ss << " ";
        if (trg_nt_position_.find(i) == trg_nt_position_.end()) {
            ss << "\""<<Dict::WSym(trg_words_[i])<<"\"";
        } else {
            ss << "x" << -1-trg_words_[i] << ":X";
        }
    }
    ss << " @ X";
    // ss << " [";
    // for (unsigned i=0; i < alignments_.size(); ++i){
    //     pair<int,int> k = alignments_[i];
    //     if (i) ss << " ";
    //     ss << k.first << "-" << k.second;
    // }
    // ss << "]";
    return ss.str();
}

bool HieroRule::IsNTSideBySide(HieroRule* rule) {
    const Sentence &source_sent = rule->GetSrcSent();
    if (source_sent.size() <= 1) 
        return false;
    for (unsigned int i=1; i < source_sent.size(); ++i) 
        if (source_sent[i] < 0 && source_sent[i-1] < 0)
            return true;
    return false;
}

void HieroRule::SetPenalty(int value) {
    if (type_ == HIERO_SRC && src_penalty_ == -1) 
        src_penalty_ = value;
    else if (type_ == HIERO_TRG && trg_penalty_ == -1)
        trg_penalty_ = value;
}

void HieroRule::ShiftAlignment() {
    map<int,int> src_shift;
    map<int,int> trg_shift;
    // Creating shift map for source
    unsigned position = 0;
    while (position < src_words_.size() && src_words_[position] < 0) 
        ++position;
    int index = 0;
    for (unsigned i=position; i < src_words_.size(); ++i) {
        if (src_words_[i] >= 0) {
            src_shift[src_index_[i]] = index++;
        }
    }
    // Creating shift map for target
    index = 0;
    position = 0;
    while (position < trg_words_.size() && trg_words_[position] < 0)
        ++position;
    for (unsigned i=position; i < trg_words_.size(); ++i) {
        if (trg_words_[i] >= 0) {
            trg_shift[trg_index_[i]] = index++;
        }
    }
    // Shifting alignment
    for (unsigned i=0; i < alignments_.size(); ++i) {
        pair<int,int> align = alignments_[i];
        alignments_[i] = make_pair(src_shift[align.first],trg_shift[align.second]);
    }
}

