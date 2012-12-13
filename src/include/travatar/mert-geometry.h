#ifndef _MERT_GEOMETRY_H_
#define _MERT_GEOMETRY_H_

// This code is for the MERT semi-ring as described in:
// "Efficient Minimum Error Rate Training and Minimum Bayes-Risk Decoding for 
// Translation Hypergraphs and Lattices"
// Shankar Kumar, Wolfgang Macherey, Chris Dyer, and Franz Och
// ACL-IJCNLP 2012
//
// It has been adapted from the implementation in the cdec decoder.

#include <vector>
#include <iostream>
#include <cfloat>
#include <boost/shared_ptr.hpp>

#include <travatar/sparse-map.h>
#include <travatar/sentence.h>

namespace travatar {

class HyperEdge;

struct MertLine {
  MertLine() : x(), m(), b(), edge() {}
  MertLine(double _m, double _b) :
    x(-DBL_MAX), m(_m), b(_b), edge() {}
  MertLine(double _x, double _m, double _b, const boost::shared_ptr<MertLine>& p1_, const boost::shared_ptr<MertLine>& p2_) :
    x(_x), m(_m), b(_b), p1(p1_), p2(p2_), edge() {}
  MertLine(double _m, double _b, const HyperEdge& edge) :
    x(-DBL_MAX), m(_m), b(_b), edge(&edge) {}

  double x;                   // x intersection with previous segment in env, or -inf if none
  double m;                   // this line's slope
  double b;                   // intercept with y-axis

  // we keep a lineer to the "parents" of this segment so we can reconstruct
  // the Viterbi translation corresponding to this segment
  boost::shared_ptr<MertLine> p1;
  boost::shared_ptr<MertLine> p2;

  // only MertLines created from an edge using the MertHullWeightFunction
  // have rules
  // TRulePtr rule;
  const HyperEdge* edge;

  // recursively recover the Viterbi translation that will result from setting
  // the weights to origin + axis * x, where x is any value from this->x up
  // until the next largest x in the containing MertHull
  void ConstructTranslation(const std::vector<WordId> & sent, std::vector<WordId>* trans) const;
  void CollectEdgesUsed(std::vector<bool>* edges_used) const;
};

// this is the semiring value type,
// it defines constructors for 0, 1, and the operations + and *
struct MertHull {
  // create semiring zero
  MertHull() : is_sorted(true) {}  // zero
  // for debugging:
  MertHull(const std::vector<boost::shared_ptr<MertLine> >& s) : lines(s) { Sort(); }
  // create semiring 1 or 0
  explicit MertHull(int i);
  MertHull(int n, MertLine* line) : is_sorted(true), lines(n, boost::shared_ptr<MertLine>(line)) {}
  const MertHull& operator+=(const MertHull& other);
  const MertHull& operator*=(const MertHull& other);
  bool IsMultiplicativeIdentity() const {
    return size() == 1 && (lines[0]->b == 0.0 && lines[0]->m == 0.0) && (!lines[0]->edge) && (!lines[0]->p1) && (!lines[0]->p2); }
  const std::vector<boost::shared_ptr<MertLine> >& GetSortedSegs() const {
    if (!is_sorted) Sort();
    return lines;
  }
  size_t size() const { return lines.size(); }
  // Input/Output
  void Print(std::ostream & out) const;
  const std::vector<boost::shared_ptr<MertLine> >& GetLines() { return lines; }
  void Sort() const;

 private:
  bool IsEdgeEnvelope() const {
    return lines.size() == 1 && lines[0]->edge; }
  mutable bool is_sorted;
  mutable std::vector<boost::shared_ptr<MertLine> > lines;
};
inline std::ostream &operator<<( std::ostream &out, const MertHull &L ) {
    L.Print(out);
    return out;
}

struct MertHullWeightFunction {
  MertHullWeightFunction(const SparseMap& ori,
                         const SparseMap& dir) : origin(ori), direction(dir) {}
  const MertHull operator()(const HyperEdge& e) const;
  const SparseMap origin;
  const SparseMap direction;
};

}

#endif
