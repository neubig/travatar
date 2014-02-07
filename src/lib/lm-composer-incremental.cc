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

// Calculate a single vertex
search::Vertex* LMComposerIncremental::CalculateVertex(
                    const HyperGraph & parse, vector<search::Vertex*> & verticies,
                    search::Context<lm::ngram::Model> & context, search::SingleBest & best,
                    int id) const {

    // Don't redo ones we've already finished
    if(verticies[id]) return verticies[id];
    // Get the nodes from the parse
    const vector<HyperNode*> & nodes = parse.GetNodes();
    // For the edges coming from this node, add them to the EdgeGenerator
    search::EdgeGenerator edges;
    BOOST_FOREACH(const HyperEdge * edge, nodes[id]->GetEdges()) {
        // Create the words
        std::vector<lm::WordIndex> words;
        std::vector<search::Vertex*> children;
        unsigned long int terminals = 0;
        float below_score = 0.0;
        int unk = 0;
        // Iterate over all output words in the target edge
        BOOST_FOREACH(WordId wid, edge->GetTrgWords()) {
            // Add non-terminal
            if(wid < 0) {
                words.push_back(lm::kMaxWordIndex);
                int tid = -1 - wid;
                children.push_back(CalculateVertex(parse, verticies, context, best, edge->GetTail(tid)->GetId()));
                below_score += children.back()->Bound();
            // Add terminal
            } else {
                lm::WordIndex index = lm_->GetVocabulary().Index(Dict::WSym(wid));
                if(index == 0) unk++;
                words.push_back(index);
                ++terminals;
            }
        }
        
        // Allocate the edge 
        search::PartialEdge pedge(edges.AllocateEdge(children.size()));
        std::vector<search::Vertex*>::const_iterator i = children.begin();
        search::PartialVertex *nt = pedge.NT();
        for (; i != children.end(); ++i, ++nt)
            *nt = (*i)->RootAlternate();

        // Score the rule
        search::ScoreRuleRet score = search::ScoreRule(*lm_, words, pedge.Between());
        pedge.SetScore(below_score + edge->GetScore() + lm_weight_ * score.prob + lm_unk_weight_ * score.oov);

        // Set the note
        search::Note note;
        note.vp = edge;
        pedge.SetNote(note);
        edges.AddEdge(pedge);
    }

    verticies[id] = new search::Vertex;
    search::VertexGenerator<search::SingleBest> vertex_gen(context, *verticies[id], best);
    edges.Search(context, vertex_gen);
    return verticies[id];
}

// Intersect this rule_graph with a language model, using cube pruning to control
// the overall state space.
HyperGraph * LMComposerIncremental::TransformGraph(const HyperGraph & parse) const {

    // Create the search configuration
    search::Config config(lm_weight_, stack_pop_limit_, search::NBestConfig(1));
    search::Context<lm::ngram::Model> context(config, *lm_);
    search::SingleBest best;

    // Create the search graph
    vector<search::Vertex*> verticies(parse.NumNodes());
    for(int i = 0; i < parse.NumNodes(); i++)
        verticies[i] = NULL;
    verticies[0] = CalculateVertex(parse, verticies, context, best, 0);

    // Clear the memory
    BOOST_FOREACH(search::Vertex * vertex, verticies)
        if(vertex)
            delete vertex;
    return NULL;

    // assert(graph.VertexCapacity());
    // for (std::size_t v = 0; v < graph.VertexCapacity() - 1; ++v) {
    //     search::EdgeGenerator edges;
    //     ReadEdges(features, context.LanguageModel(), in, graph, edges);
    //     search::VertexGenerator<Best> vertex_gen(context, *graph.NewVertex(), best);
    //     edges.Search(context, vertex_gen);
    // }
    // // Create the final edge
    // search::EdgeGenerator edges;
    // ReadEdges(features, context.LanguageModel(), in, graph, edges);
    // search::Vertex &root = *graph.NewVertex();
    // search::RootVertexGenerator<Best> vertex_gen(root, best);
    // edges.Search(context, vertex_gen);


    // return root.BestChild();
}
