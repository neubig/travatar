#include <travatar/train-caser-runner.h>
#include <travatar/config-train-caser-runner.h>
#include <travatar/sentence.h>
#include <travatar/dict.h>
#include <travatar/caser.h>

#include <boost/unordered_map.hpp>
#include <boost/foreach.hpp>

#include <iostream>

using namespace std;
using namespace travatar;

void TrainCaserRunner::Run(const ConfigTrainCaserRunner & config) {
    string line;
    typedef boost::unordered_map<WordId, boost::unordered_map<WordId, int> > DoubleMap;
    typedef pair<WordId, int> WordCount;
    DoubleMap counts;
    Caser caser;
    // Count the words
    while(getline(cin, line)) {
        Sentence sent = Dict::ParseWords(line);
        vector<bool> is_first = caser.SentenceFirst(sent);
        for(int i = 0; i < (int)is_first.size(); i++)
            if(!is_first[i])
                counts[caser.ToLower(sent[i])][sent[i]]++;
    }
    // Find the best for each word
    BOOST_FOREACH(const DoubleMap::value_type & my_map, counts) {
        int max_cnt = -1;
        WordId best = -1;
        BOOST_FOREACH(WordCount cnt, my_map.second) {
            if(cnt.second > max_cnt) {
                max_cnt = cnt.second;
                best = cnt.first;
            }
        }
        cout << Dict::WSym(best) << endl;
    }
}
