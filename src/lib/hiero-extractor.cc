
#include <travatar/hiero-extractor.h>
#include <travatar/hiero-rule-table.h>
#include <travatar/alignment.h>
#include <vector>

using namespace std;
using namespace travatar;

//////////////////////////////////////////
//        HIERO RULE EXTRACTOR          //
//////////////////////////////////////////

string PrintPhrasePairH(PhrasePair pp) {
    ostringstream oss;
    oss << "[(" << pp.first.first << "," << pp.first.second << ")(" << pp.second.first << "," << pp.second.second << ")]";
    return oss.str();
} 

// Public 
vector<vector<HieroRule> > HieroExtractor::ExtractHieroRule(const Alignment & align, const Sentence & source, 
        const Sentence & target) const
{
    vector<vector<HieroRule> >ret = vector<vector<HieroRule> >();
    PhrasePairs filtered_pairs = PhrasePairs();
    PhrasePairs pairs;

    // Doing extraction safely
    try {
        pairs = ExtractPhrase(align,source, target);
    } catch (exception& exc) {
        THROW_ERROR("Input or alignment error. \n\tOn Source: " + Dict::PrintWords(source) + 
            "\n\tOn Target: " + Dict::PrintWords(target));
    }

    int rule_max_len = HieroExtractor::GetMaxRuleLen();

    // if there are multiple initial phrase pairs containing the same set of alignments, only the 
    // smallest is kept. That is, unaligned words are not allowed at the edges of phrases
    map<pair<int,int>, int> mp = map<pair<int,int>,int>();
    BOOST_FOREACH(PhrasePair pp, pairs) {
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
    // Rule extraction algorithm for 1 + 2 Nonterminal Symbol. If we use Higher, algorithm is too complex
    for (int ii=0; (unsigned) ii < pairs.size(); ++ii) {
        // initial phrases are limited to a length of INITIAL_PHRASE_LIMIT (10) words on either side
        if (pairs[ii].first.second - pairs[ii].first.first < HieroExtractor::GetMaxInitialPhrase() || 
                pairs[ii].second.second - pairs[ii].second.first < HieroExtractor::GetMaxInitialPhrase()) 
        {
            vector<HieroRule> _extracted = vector<HieroRule>();
            _extracted.push_back(ParsePhraseTranslationRule(source,target,pairs[ii]));
            // Find pairs of 2 rules
            for (int jj=0; (unsigned) jj < pairs.size(); ++jj) {
                if (jj != ii && InPhrase(pairs[jj],pairs[ii])) {
                    for (int kk=jj+1; (unsigned) kk < pairs.size(); ++kk) {
                        // that are in the span of INITIAL phrase, and NOT overlapping each other
                        if (kk != jj && InPhrase(pairs[kk],pairs[ii]) && InPhrase(pairs[jj],pairs[ii]) && !IsPhraseOverlapping(pairs[jj],pairs[kk])) 
                        {
                            HieroRule _rule = ParseBinaryPhraseRule(source,target,pairs[jj],pairs[kk],pairs[ii]);
                            if ((int)_rule.GetSourceSentence().size() <= rule_max_len || 
                                    (int) _rule.GetTargetSentence().size() <= rule_max_len)
                            {
                                HieroRuleManager::AddRule(_extracted,_rule);
                            }
                        }
                    }
                    // Unary rule
                    HieroRule _rule = ParseUnaryPhraseRule(source,target,pairs[jj],pairs[ii]);
                    if ((int)_rule.GetSourceSentence().size() <= rule_max_len || 
                            (int) _rule.GetTargetSentence().size() <= rule_max_len)
                    {
                        HieroRuleManager::AddRule(_extracted,_rule);
                    }
                }
            }
            if (_extracted.size() > (unsigned) 0) {
                ret.push_back(_extracted);
            }
        }
        // 
    }
    // DEBUG PrintPhrasePairs();
    return ret;
}

// The implementation of phrase extraction algorithm.
// The algorithm to extract all consistent phrase pairs from a word-aligned sentence pair
PhrasePairs HieroExtractor::ExtractPhrase(const Alignment & align, const Sentence & source, 
        const Sentence & target) const
{
    vector<set<int> > s2t = align.GetSrcAlignments();
    vector<set<int> > t2s = vector<set<int> >();
    PhrasePairs ret = PhrasePairs();

    // Remap from source -> target back to target -> source
    for (unsigned t=0; t < target.size(); ++t) {
        t2s.push_back(set<int>());
    }
    for (unsigned s=0; s < s2t.size(); ++s) {
        set<int> ts = s2t[s];
        BOOST_FOREACH(int t, ts) {
            t2s[t].insert((int)s);
        }
    }

    // Phrase Extraction Algorithm 
    // This is very slow (actually), maybe better data structure can improves it.
    for (int s_begin=0; s_begin < (int)s2t.size(); ++s_begin) {
        map<int, int> tp;
        for (int s_end=s_begin; s_end < (int)s2t.size(); ++s_end) {
            if (s2t[s_end].size() != 0) { 
                BOOST_FOREACH(int _t, s2t[s_end]) { tp[_t]++;}
            }
            int t_begin = MapMinKey(tp);
            int t_end = MapMaxKey(tp);
            if (QuasiConsecutive(t_begin,t_end,tp,t2s)) {
                map<int, int> sp;
                for (int t=t_begin; t<=t_end;++t) {
                    if (t2s[t].size() != 0) {
                        BOOST_FOREACH(int _s, t2s[t]) { sp[_s]++; }
                    }
                }
                if (MapMinKey(sp) >= s_begin && MapMaxKey(sp) <= s_end) {
                    while (t_begin >= 0) {
                        int jp = t_end;
                        while (jp <= (int)t2s.size()) {
                            ret.push_back(make_pair(make_pair(s_begin,s_end),make_pair(t_begin,jp)));
                            ++jp;   
                            if (jp == (int)t2s.size() || t2s[jp].size() != 0) {
                                break;
                            }
                        }
                        --t_begin;
                        if(t_begin < 0 || t2s[t_begin].size() != 0) {
                            break;
                        }
                        
                    }
                }
            }
        }
    }
    return ret;
}

// Private Member
string HieroExtractor::AppendString(const Sentence & s, const int begin, const int end) const {
    string ret = string("");
    for (int i=begin; i<=(int)end; ++i) {
        ret += Dict::WSym(s[i]);
        if (i != end) {
            ret += " ";
        }
    }
    return ret;
}

string HieroExtractor::PrintPhrasePair(const PhrasePair & pp, const Sentence & source, 
    const Sentence & target) const
{
    return AppendString(source, pp.first.first, pp.first.second) + string(" -> ") 
        + AppendString(target, pp.second.first, pp.second.second);
}

void HieroExtractor::PrintPhrasePairs(const PhrasePairs & pairs, const Sentence & source, 
    const Sentence & target) 
{
    BOOST_FOREACH(PhrasePair pp , pairs) {
        cerr << PrintPhrasePair(pp,source,target) <<endl;
    }
}

int HieroExtractor::MapMaxKey(const map<int,int> & map) const {
    int max = 0;
    pair<int,int> item;
    BOOST_FOREACH(item, map) {
        if (item.first > max) max = item.first;
    } 
    return max;
}

int HieroExtractor::MapMinKey(const map<int,int> & map) const {
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
        const vector<set<int> > & t2s) const
{
    for (int i=small; i <= large; ++i) {
        if (t2s[i].size() != 0 && tp.find(i) == tp.end()) {
            return 0;
        }
    }   
    return 1;
}

int HieroExtractor::IsTerritoryOverlapping(const pair<int,int> & a, const pair<int,int> & b) const {
    return 
        (a.first >= b.first && a.second <= b.second) || // a in b
        (b.first >= a.first && b.second <= a.second) || // b in a
        (a.first <= b.first && a.second >= b.first)  || // a preceeds b AND they are overlapping
        (b.first <= a.first && b.second >= a.first);    // b preceeds a AND they are overlapping
}

int HieroExtractor::IsPhraseOverlapping(const PhrasePair & pair1, const PhrasePair & pair2) const {
    return IsTerritoryOverlapping(pair1.first, pair2.first) || IsTerritoryOverlapping(pair1.second,pair2.second);
}

void HieroExtractor::ParseRuleWith2Nonterminals(const Sentence & sentence, const pair<int,int> & pair1, 
        const pair<int,int> & pair2, 
        const pair<int,int> & pair_span, 
        HieroRule & target, const int type) const
{
    target.SetType(type);
    int x0 = 0;
    int x1 = 0;
    for (int i=pair_span.first; i <= pair_span.second; ++i) {
        if (i >= pair1.first && i <= pair1.second) {
            if (x1) {
                target.AddNontermX(1);
                x1 = 0;
            }
            x0 = 1;
        } else if (i >= pair2.first && i <= pair2.second) {
            if (x0) {
                target.AddNontermX(0);
                x0 = 0;
            }
            x1 = 1;
        } else {
            if (x0) {
                target.AddNontermX(0);
                x0 = 0;
            } 
            if (x1) {
                target.AddNontermX(1);
                x1 = 0;
            }
            target.AddWord(sentence[i]);
        }
    }
    if (x0) target.AddNontermX(0);
    else if (x1) target.AddNontermX(1);
}

HieroRule HieroExtractor::ParseBinaryPhraseRule(const Sentence & source, const Sentence & target, const PhrasePair & pair1, 
        const PhrasePair & pair2, const PhrasePair & pair_span) const
{
    HieroRule _rule = HieroRule();
    ParseRuleWith2Nonterminals(source,pair1.first,pair2.first,pair_span.first,_rule,HIERO_SOURCE);
    ParseRuleWith2Nonterminals(target,pair1.second,pair2.second,pair_span.second,_rule,HIERO_TARGET);
    return _rule;
}

void HieroExtractor::ParseRuleWith1Nonterminals(const Sentence & sentence, const pair<int,int> & my_pair, 
        const pair<int,int> & pair_span, HieroRule & target, const int type) const
{
    target.SetType(type);
    int x = 0;
    for (int i=pair_span.first; i <= pair_span.second; ++i) {
        if (i >= my_pair.first && i <= my_pair.second) {
            x = 1;
        } else {
            if (x) {
                target.AddNontermX(0);
                x = 0; 
            } 
            target.AddWord(sentence[i]);
        }
    }
    if (x) target.AddNontermX(0);
}

HieroRule HieroExtractor::ParseUnaryPhraseRule(const Sentence & source, const Sentence & target, 
        const PhrasePair & my_pair, const PhrasePair & pair_span) const
{
    HieroRule _rule = HieroRule();
    ParseRuleWith1Nonterminals(source,my_pair.first,pair_span.first,_rule,HIERO_SOURCE);
    ParseRuleWith1Nonterminals(target,my_pair.second,pair_span.second,_rule,HIERO_TARGET);
    return _rule;
}

HieroRule HieroExtractor::ParsePhraseTranslationRule(const Sentence & source, const Sentence & target, 
        const PhrasePair & my_pair) const
{
    HieroRule _rule = HieroRule();
    _rule.SetType(HIERO_SOURCE);
    for (int i=my_pair.first.first; i <= my_pair.first.second; ++i) _rule.AddWord(source[i]);
    _rule.SetType(HIERO_TARGET);
    for (int i=my_pair.second.first; i <= my_pair.second.second; ++i) _rule.AddWord(target[i]);
    return _rule;
}

int HieroExtractor::InPhrase(const PhrasePair & p1, const PhrasePair & p2) const {
    return p1.first.first >= p2.first.first && p1.first.second <= p2.first.second && 
            p1.second.first >= p2.second.first && p1.second.second <= p2.second.second;
}
