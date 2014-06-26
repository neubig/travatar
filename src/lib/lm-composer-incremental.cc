#include <boost/unordered_set.hpp>
#include <boost/foreach.hpp>
#include <lm/left.hh>
#include <search/context.hh>
#include <search/edge.hh>
#include <search/vertex_generator.hh>
#include <search/rule.hh>
#include <search/edge_generator.hh>
#include <vector>
#include <queue>
#include <map>
#include <travatar/lm-composer-incremental.h>
#include <travatar/generic-string.h>
#include <travatar/hyper-graph.h>
#include <travatar/dict.h>

using namespace travatar;
using namespace std;
using namespace boost;
using namespace lm;
using namespace search;

// Convert all of the edges together into a node
NBestComplete Forest::Complete(std::vector<PartialEdge> &partial) {
    WordId lm_id = Dict::WID("lm");
    WordId lm_unk_id = Dict::WID("lmunk");
    // For each vector, create a node
    HyperNode * node = new HyperNode;
    hg->AddNode(node);
    // For remembering duplicate edges
    map<GenericString<WordId>, HyperEdge*> node_memo;
    // For each edge, add a hyperedge to the graph
    PartialEdge best;
    HyperEdge *old_edge = NULL, *edge = NULL;
    BOOST_FOREACH(const PartialEdge & add, partial) {
        if (!best.Valid() || best.GetScore() < add.GetScore())
            best = add;
        old_edge = (HyperEdge*)add.GetNote().vp;
        double edge_score = add.GetScore();
        // Add the new tails in *source* order 
        vector<HyperNode*> tails;
        Sentence wids;
        GenericString<WordId> node_id;
        if(old_edge) {
            wids = old_edge->GetTrgData()[factor_].words;
            node_id = GenericString<WordId>(old_edge->GetTails().size()+1);
            node_id[old_edge->GetTails().size()] = old_edge->GetId();
        } else{
            wids = Sentence(1,-1);
            node_id = GenericString<WordId>(1);
        }
        BOOST_FOREACH(WordId wid, wids) {
            if(wid < 0) {
                int tid = -1-wid;
                const PartialVertex & part = add.NT()[tid];
                HyperNode* child = (HyperNode*)part.End();
                tails.push_back(child);
                edge_score -= child->GetViterbiScore();
                // Keep track of the node ID
                node_id[tid] = child->GetId();
            }
        }
        // Skip duplicate edges
        if(node_memo.find(node_id) != node_memo.end())
            continue;
            // if(edge_score != node_memo[node_id]->GetScore()) {
            //     THROW_ERROR("Duplicate edges with different scores" << node_id << endl << edge_score << endl << *old_edge << endl << *node_memo[node_id]);
            // }
        // Create the new edge
        int lm_unk = 0;
        if(old_edge) {
            edge = new HyperEdge(*old_edge);
            lm_unk = lm_unks_[old_edge->GetId()];
        } else {
            edge = new HyperEdge;
            edge->SetTrgData(CfgDataVector(GlobalVars::trg_factors, CfgData(Sentence(1,-1))));
        }
        node_memo.insert(make_pair(node_id, edge));
        edge->SetHead(node);
        edge->SetTails(tails);
        hg->AddEdge(edge); node->AddEdge(edge);
        edge->GetFeatures()[lm_id] = (edge_score - lm_unk * lm_unk_weight_ - edge->GetScore())/lm_weight_;
        if(lm_unk)
            edge->GetFeatures()[lm_unk_id] = lm_unk;
        edge->SetScore(edge_score);
    }
    // Set the span for either the internal or final nodes
    if(old_edge) {
        node->SetSpan(old_edge->GetHead()->GetSpan());
        node->SetSym(old_edge->GetHead()->GetSym());
    } else {
        node->SetSpan(edge->GetTail(0)->GetSpan());
        node->SetSym(Dict::WID("LMROOT"));
    }
    node->SetViterbiScore(best.GetScore());
    // Return the n-best
    if (!best.Valid())
        return NBestComplete(NULL, lm::ngram::ChartState(), -INFINITY);
    else
        return NBestComplete(node, best.CompletedState(), best.GetScore());
}

// Calculate a single vertex
search::Vertex* LMComposerIncremental::CalculateVertex(
                    const HyperGraph & parse, vector<search::Vertex*> & vertices,
                    search::Context<lm::ngram::Model> & context, search::Forest & best,
                    int id) const {

    // Don't redo ones we've already finished
    if(vertices[id]) return vertices[id];
    // Get the nodes from the parse
    const vector<HyperNode*> & nodes = parse.GetNodes();
    // For the edges coming from this node, add them to the EdgeGenerator
    search::EdgeGenerator edges;
    int num_edges = 0;
    if(id < 0 || id >= (int)nodes.size() || nodes[id] == NULL)
        THROW_ERROR("Bad id=" << id << " at nodes.size() == " << nodes.size());
    BOOST_FOREACH(const HyperEdge * edge, nodes[id]->GetEdges()) {
        // Create the words
        std::vector<lm::WordIndex> words;
        std::vector<search::Vertex*> children;
        unsigned long int terminals = 0;
        float below_score = 0.0;
        int unk = 0;
        // Iterate over all output words in the target edge
        BOOST_FOREACH(WordId wid, edge->GetTrgData()[factor_].words) {
            // Add non-terminal
            if(wid < 0) {
                words.push_back(lm::kMaxWordIndex);
                int tid = -1 - wid;
                Vertex* vertex = CalculateVertex(parse, vertices, context, best, edge->GetTail(tid)->GetId());
                children.push_back(vertex);
                if(vertex->Empty()) {
                    below_score = -FLT_MAX;
                    break;
                }
                below_score += children.back()->Bound();
            // Add terminal
            } else {
                lm::WordIndex index = GetMapping(wid);
                if(index == 0) unk++;
                words.push_back(index);
                ++terminals;
            }
        }

        if(below_score == -FLT_MAX)
            continue;
        
        // Allocate the edge 
        search::PartialEdge pedge(edges.AllocateEdge(children.size()));
        std::vector<search::Vertex*>::const_iterator i = children.begin();
        search::PartialVertex *nt = pedge.NT();
        for (; i != children.end(); ++i, ++nt)
            *nt = (*i)->RootAlternate();

        // Score the rule
        search::ScoreRuleRet score = search::ScoreRule(*lm_, words, pedge.Between());
        pedge.SetScore(below_score + edge->GetScore() + lm_weight_ * score.prob + lm_unk_weight_ * score.oov);
        best.SetLMUnk(edge->GetId(), score.oov);

        // Set the note
        search::Note note;
        note.vp = edge;
        pedge.SetNote(note);
        edges.AddEdge(pedge);
        num_edges++;
    }

    vertices[id] = new search::Vertex;
    if(!edges.Empty()) {
        search::VertexGenerator<search::Forest> vertex_gen(context, *vertices[id], best);
        edges.Search(context, vertex_gen);
    }
    return vertices[id];
}

// Calculate the root vetex
search::Vertex* LMComposerIncremental::CalculateRootVertex(
                    vector<search::Vertex*> & vertices,
                    search::Context<lm::ngram::Model> & context, search::Forest & best) const {
    assert(vertices[0]);
    // Don't redo ones we've already finished
    int id = vertices.size()-1;
    if(vertices[id]) return vertices[id];
    // For the edges coming from this node, add them to the EdgeGenerator
    search::EdgeGenerator edges;
    std::vector<lm::WordIndex> words(3,0);
    // Calculate the word indexes
    words[0] = lm_->GetVocabulary().Index("<s>");
    words[1] = lm::kMaxWordIndex;
    words[2] = lm_->GetVocabulary().Index("</s>");
    // Allocate the edge
    search::PartialEdge pedge(edges.AllocateEdge(1));
    (*pedge.NT()) = vertices[0]->RootAlternate();
    double below_score = vertices[0]->Bound();
    // Set the note
    search::Note note;
    note.vp = NULL;
    pedge.SetNote(note);
    edges.AddEdge(pedge);
    // Perform scoring and add the edge
    search::ScoreRuleRet score = search::ScoreRule(*lm_, words, pedge.Between());
    pedge.SetScore(below_score + lm_weight_ * score.prob + lm_unk_weight_ * score.oov);
    edges.AddEdge(pedge);
    // Create the vertex
    vertices[id] = new search::Vertex;
    search::VertexGenerator<search::Forest> vertex_gen(context, *vertices[id], best);
    edges.Search(context, vertex_gen);
    return vertices[id];
}

// Intersect this rule_graph with a language model, using cube pruning to control
// the overall state space.
HyperGraph * LMComposerIncremental::TransformGraph(const HyperGraph & parse) const {

    if(parse.NumNodes() == 0) return new HyperGraph;

    // Create the search configuration
    search::NBestConfig nconfig(edge_limit_);
    search::Config config(lm_weight_, stack_pop_limit_, nconfig);
    search::Context<lm::ngram::Model> context(config, *lm_);
    search::Forest best(lm_weight_, lm_unk_weight_, factor_);

    // Create the search graph
    vector<search::Vertex*> vertices(parse.NumNodes() + 1);
    for(int i = 0; i < parse.NumNodes(); i++)
        vertices[i] = NULL;
    vertices[0] = CalculateVertex(parse, vertices, context, best, 0);

    // Create the final vertex (recursively creating the others)
    CalculateRootVertex(vertices, context, best);

    // Clear the memory
    BOOST_FOREACH(search::Vertex * vertex, vertices)
        if(vertex)
            delete vertex;

    // Get the hypergraph and set words
    HyperGraph* ret = best.StealPointer();
    ret->SetWords(parse.GetWords());
    vector<HyperNode*> & nodes = ret->GetNodes();
    // Swap the root into the first position
    nodes[0] = nodes[nodes.size()-1];
    nodes[0]->SetId(0);
    nodes.resize(nodes.size()-1);

    // Return the pointer
    return ret;

}
