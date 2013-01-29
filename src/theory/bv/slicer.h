/*********************                                                        */
/*! \file slicer.h
 ** \verbatim
 ** Original author: lianah
 ** Major contributors: none
 ** Minor contributors (to current version): none
 ** This file is part of the CVC4 prototype.
 ** Copyright (c) 2009, 2010, 2011  The Analysis of Computer Systems Group (ACSys)
 ** Courant Institute of Mathematical Sciences
 ** New York University
 ** See the file COPYING in the top-level source directory for licensing
 ** information.\endverbatim
 **
 ** \brief Bitvector theory.
 **
 ** Bitvector theory.
 **/

#include "cvc4_private.h"


#include <vector>
#include <list>
#include <ext/hash_map>
#include <math.h>

#include "util/bitvector.h"
#include "util/statistics_registry.h"
#include "util/index.h"
#include "expr/node.h"
#include "theory/bv/theory_bv_utils.h"
#ifndef __CVC4__THEORY__BV__SLICER_BV_H
#define __CVC4__THEORY__BV__SLICER_BV_H


namespace CVC4 {

namespace theory {
namespace bv {



typedef Index TermId;
extern const TermId UndefinedId;


/** 
 * Base
 * 
 */
class Base {
  Index d_size;
  std::vector<uint32_t> d_repr;
public:
  Base(Index size);
  void sliceAt(Index index); 
  void sliceWith(const Base& other);
  bool isCutPoint(Index index) const;
  void diffCutPoints(const Base& other, Base& res) const;
  bool isEmpty() const;
  std::string debugPrint() const;
  Index getBitwidth() const { return d_size; }
  bool operator==(const Base& other) const {
    if (other.getBitwidth() != getBitwidth())
      return false;
    for (unsigned i = 0; i < d_repr.size(); ++i) {
      if (d_repr[i] != other.d_repr[i])
        return false;
    }
    return true; 
  }
}; 

/**
 * UnionFind
 * 
 */
typedef __gnu_cxx::hash_set<TermId> TermSet;
typedef std::vector<TermId> Decomposition; 

struct ExtractTerm {
  TermId id;
  Index high;
  Index low;
  ExtractTerm(TermId i, Index h, Index l)
    : id (i),
      high(h),
      low(l)
  {
    Assert (h >= l && id != UndefinedId); 
  }
  Index getBitwidth() const { return high - low + 1; }
  std::string debugPrint() const; 
};

class UnionFind; 

struct NormalForm {
  Base base;
  Decomposition decomp;

  NormalForm(Index bitwidth)
    : base(bitwidth),
      decomp()
  {}
  /** 
   * Returns the term in the decomposition on which the index i
   * falls in
   * @param i 
   * 
   * @return 
   */
  TermId getTerm(Index i, const UnionFind& uf) const;
  std::string debugPrint(const UnionFind& uf) const; 
};


class UnionFind {
  class Node {
    Index d_bitwidth;
    TermId d_ch1, d_ch2;
    TermId d_repr;    
  public:
    Node(Index b)
  : d_bitwidth(b),
    d_ch1(UndefinedId),
    d_ch2(UndefinedId), 
    d_repr(UndefinedId)
    {}
    
    TermId getRepr() const { return d_repr; }
    Index getBitwidth() const { return d_bitwidth; }
    bool hasChildren() const { return d_ch1 != UndefinedId && d_ch2 != UndefinedId; }

    TermId getChild(Index i) const {
      Assert (i < 2);
      return i == 0? d_ch1 : d_ch2;
    }
    Index getCutPoint(const UnionFind& uf) const {
      Assert (d_ch1 != UndefinedId && d_ch2 != UndefinedId);
      return uf.getNode(d_ch1).getBitwidth(); 
    }
    void setRepr(TermId id) {
      Assert (! hasChildren());
      d_repr = id;
    }

    void setChildren(TermId ch1, TermId ch2) {
      Assert (d_repr == UndefinedId && !hasChildren());
      d_ch1 = ch1;
      d_ch2 = ch2; 
    }
    std::string debugPrint() const; 
  };

  /// map from TermId to the nodes that represent them 
  std::vector<Node> d_nodes;
  /// a term is in this set if it is its own representative
  TermSet d_representatives;
  
  void getDecomposition(const ExtractTerm& term, Decomposition& decomp);
  void handleCommonSlice(const Decomposition& d1, const Decomposition& d2, TermId common);
  
public:
  UnionFind()
    : d_nodes(),
      d_representatives()
  {}

  TermId addTerm(Index bitwidth);
  void unionTerms(const ExtractTerm& t1, const ExtractTerm& t2); 
  void merge(TermId t1, TermId t2);
  TermId find(TermId t1) const ; 
  void split(TermId term, Index i);

  void getNormalForm(const ExtractTerm& term, NormalForm& nf);
  void alignSlicings(const ExtractTerm& term1, const ExtractTerm& term2);
  void ensureSlicing(const ExtractTerm& term);
  
  Node getNode(TermId id) const {
    Assert (id < d_nodes.size());
    return d_nodes[id]; 
  }
  Index getBitwidth(TermId id) const {
    Assert (id < d_nodes.size());
    return d_nodes[id].getBitwidth(); 
  }

};

class Slicer {
  __gnu_cxx::hash_map<TermId, TNode> d_idToNode;
  __gnu_cxx::hash_map<TNode, TermId, TNodeHashFunction> d_nodeToId;
  __gnu_cxx::hash_map<TNode, bool, TNodeHashFunction> d_coreTermCache; 
  UnionFind d_unionFind;
  ExtractTerm registerTerm(TNode node); 
public:
  Slicer()
    : d_idToNode(),
      d_nodeToId(),
      d_coreTermCache(),
      d_unionFind()
  {}
  
  void getBaseDecomposition(TNode node, std::vector<Node>& decomp);
  void processEquality(TNode eq);
  bool isCoreTerm (TNode node);
  static void splitEqualities(TNode node, std::vector<Node>& equalities); 
}; 

}/* CVC4::theory::bv namespace */
}/* CVC4::theory namespace */
}/* CVC4 namespace */

#endif /* __CVC4__THEORY__BV__SLICER_BV_H */
