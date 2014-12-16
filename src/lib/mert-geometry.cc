#include <travatar/mert-geometry.h>
#include <travatar/hyper-graph.h>
#include <travatar/global-debug.h>
#include <travatar/dict.h>
#include <travatar/io-util.h>
#include <boost/foreach.hpp>
#include <cassert>
#include <climits>

using namespace std;
using namespace travatar;

MertHull::MertHull(int i) {
  if (i == 0) {
    // do nothing - <>
  } else if (i == 1) {
    lines.push_back(boost::shared_ptr<MertLine>(new MertLine(0, 0, 0, boost::shared_ptr<MertLine>(), boost::shared_ptr<MertLine>())));
    assert(this->IsMultiplicativeIdentity());
  } else {
    cerr << "Only can create MertHull semiring 0 and 1 with this constructor!\n";
    abort();
  }
}

const MertHull MertHullWeightFunction::operator()(const HyperEdge& e) const {
  const Real m = direction * e.GetFeatures();
  const Real b = origin * e.GetFeatures();
  MertLine* line = new MertLine(m, b, e);
  return MertHull(1, line);
}

void MertHull::Print(ostream& os) const {
  os << '<';
  const vector<boost::shared_ptr<MertLine> >& lines = this->GetSortedSegs();
  for (int i = 0; i < (int)lines.size(); ++i)
    os << (i==0 ? "" : "|") << "x=" << lines[i]->x << ",b=" << lines[i]->b << ",m=" << lines[i]->m << ",p1=" << lines[i]->p1 << ",p2=" << lines[i]->p2;
  os << '>';
}

#define ORIGINAL_MERT_IMPLEMENTATION 1
#ifdef ORIGINAL_MERT_IMPLEMENTATION

struct SlopeCompare {
  bool operator() (const boost::shared_ptr<MertLine>& a, const boost::shared_ptr<MertLine>& b) const {
    return a->m < b->m;
  }
};

const MertHull& MertHull::operator+=(const MertHull& other) {
  if (!other.is_sorted) other.Sort();
  if (lines.empty()) {
    lines = other.lines;
    return *this;
  }
  is_sorted = false;
  int j = lines.size();
  lines.resize(lines.size() + other.lines.size());
  for (int i = 0; i < (int)other.lines.size(); ++i)
    lines[j++] = other.lines[i];
  assert(j == (int)lines.size());
  return *this;
}

void MertHull::Sort() const {
  sort(lines.begin(), lines.end(), SlopeCompare());
  const int k = lines.size();
  int j = 0;
  for (int i = 0; i < k; ++i) {
    MertLine l = *lines[i];
    l.x = -REAL_MAX;
    // cerr << "m=" << l.m << endl;
    if (0 < j) {
      if (lines[j-1]->m == l.m) {   // lines are parallel
        if (l.b <= lines[j-1]->b) continue;
        --j;
      }
      while(0 < j) {
        l.x = (l.b - lines[j-1]->b) / (lines[j-1]->m - l.m);
        if (lines[j-1]->x < l.x) break;
        --j;
      }
      if (0 == j) l.x = -REAL_MAX;
    }
    *lines[j++] = l;
  }
  lines.resize(j);
  is_sorted = true;
}

const MertHull& MertHull::operator*=(const MertHull& other) {
  if (other.IsMultiplicativeIdentity()) { return *this; }
  if (this->IsMultiplicativeIdentity()) { (*this) = other; return *this; }

  if (!is_sorted) Sort();
  if (!other.is_sorted) other.Sort();

  if (this->IsEdgeEnvelope()) {
//    if (other.size() > 1)
//      cerr << *this << " (TIMES) " << other << endl;
    boost::shared_ptr<MertLine> edge_parent = lines[0];
    const Real& edge_b = edge_parent->b;
    const Real& edge_m = edge_parent->m;
    lines.clear();
    for (int i = 0; i < (int)other.lines.size(); ++i) {
      const MertLine& p = *other.lines[i];
      const Real m = p.m + edge_m;
      const Real b = p.b + edge_b;
      const Real& x = p.x;       // x's don't change with *
      lines.push_back(boost::shared_ptr<MertLine>(new MertLine(x, m, b, edge_parent, other.lines[i])));
      assert(lines.back()->p1->edge);
    }
//    if (other.size() > 1)
//      cerr << " = " << *this << endl;
  } else {
    vector<boost::shared_ptr<MertLine> > new_lines;
    int this_i = 0;
    int other_i = 0;
    const int this_size  = lines.size();
    const int other_size = other.lines.size();
    Real cur_x = -REAL_MAX;   // moves from left to right across the
                                     // real numbers, stopping for all inter-
                                     // sections
    Real this_next_val  = (1 < this_size  ? lines[1]->x       : REAL_MAX);
    Real other_next_val = (1 < other_size ? other.lines[1]->x : REAL_MAX);
    while (this_i < this_size && other_i < other_size) {
      const MertLine& this_line = *lines[this_i];
      const MertLine& other_line= *other.lines[other_i];
      const Real m = this_line.m + other_line.m;
      const Real b = this_line.b + other_line.b;
 
      new_lines.push_back(boost::shared_ptr<MertLine>(new MertLine(cur_x, m, b, lines[this_i], other.lines[other_i])));
      int comp = 0;
      if (this_next_val < other_next_val) comp = -1; else
        if (this_next_val > other_next_val) comp = 1;
      if (0 == comp) {  // the next values are equal, advance both indices
        ++this_i;
        ++other_i;
        cur_x = this_next_val;  // could be other_next_val (they're equal!)
        this_next_val  = (this_i+1  < this_size  ? lines[this_i+1]->x        : REAL_MAX);
        other_next_val = (other_i+1 < other_size ? other.lines[other_i+1]->x : REAL_MAX);
      } else {  // advance the i with the lower x, update cur_x
        if (-1 == comp) {
          ++this_i;
          cur_x = this_next_val;
          this_next_val =  (this_i+1  < this_size  ? lines[this_i+1]->x        : REAL_MAX);
        } else {
          ++other_i;
          cur_x = other_next_val;
          other_next_val = (other_i+1 < other_size ? other.lines[other_i+1]->x : REAL_MAX);
        }
      }
    }
    lines.swap(new_lines);
  }
  //cerr << "Multiply: result=" << (*this) << endl;
  return *this;
}

// recursively construct translation
void MertLine::ConstructTranslation(const vector<WordId> & sent, vector<Sentence>* trans) const {
    WordId unk_id = Dict::WID("<unk>");
    const MertLine* cur = this;
    // ant_trans is the translations for the tails in REVERSE order
    vector<vector<Sentence> > ant_trans;
    while(!cur->edge) {
        ant_trans.resize(ant_trans.size() + 1, vector<Sentence>(trans->size()));
        cur->p2->ConstructTranslation(sent, &ant_trans.back());
        cur = cur->p1.get();
    }
    for(int factor = 0; factor < (int)cur->edge->GetTrgData().size(); factor++) {
        size_t ant_size = ant_trans.size();
        assert(ant_size == cur->edge->GetTails().size());
        BOOST_FOREACH(WordId id, cur->edge->GetTrgData()[factor].words) {
            if(id == unk_id) {
                pair<int,int> span = cur->edge->GetHead()->GetSpan();
                if(span.second-span.first != 1) THROW_ERROR("Bad span in unknown rule: " << span);
                (*trans)[factor].push_back(sent[span.first]);
            } else if(id >= 0) {
                (*trans)[factor].push_back(id);
            } else {
                // Because tails are in reverse order, we must access them accordingly
                BOOST_FOREACH(WordId cid, ant_trans[ant_size + id][factor])
                    (*trans)[factor].push_back(cid);
            }
        }
    }
}

void MertLine::CollectEdgesUsed(std::vector<bool>* edges_used) const {
  if (edge) {
    assert(edge->GetId() < (int)edges_used->size());
    (*edges_used)[edge->GetId()] = true;
  }
  if (p1) p1->CollectEdgesUsed(edges_used);
  if (p2) p2->CollectEdgesUsed(edges_used);
}

#else

// THIS IS THE NEW FASTER IMPLEMENTATION OF THE MERT SEMIRING OPERATIONS

#endif

