#include <travatar/tree-io.h>
#include <travatar/io-util.h>

using namespace travatar;
using namespace std;

HyperGraph * PennTreeIO::ReadTree(istream & in) {
    // The new hypergraph and stack to read the nodes
    HyperGraph * hg = new HyperGraph;
    vector<HyperNode*> stack;
    char next_char;
    string line;
    int pos = 0;
    string WHITE_SPACE_OR_OPENPAREN = string(WHITE_SPACE)+"(";
    // Continue until the end of the corpus
    while(in) {
        Trim(in, WHITE_SPACE);
        in >> next_char;
        // If the next character is a close parenthesis, close the noe on the top of the stack
        if(next_char == ')') {
            if(!stack.size()) { getline(in, line); THROW_ERROR("Unmatched close parenthesis at )" << line); }
            HyperNode * child = *stack.rbegin(); stack.pop_back();
            child->GetSpan().second = pos;
            // If no parent exists, we are at the root. Return.
            if(!stack.size()) return hg;
            // If a parent exists, add the child to its tails
            HyperNode * parent = *stack.rbegin();
            parent->GetEdge(0)->AddTail(child);
        // Otherwise, open a new node
        } else if(next_char == '(') {
            // Read the symbol
            string sym = ReadUntil(in, WHITE_SPACE, "()");
            if(!sym.length()) { getline(in, line); THROW_ERROR("Empty symbol at '("<<line<<"'"); }
            // Create a new node
            HyperNode* node = new HyperNode(Dict::WID(sym), MakePair(pos,-1));
            stack.push_back(node); hg->AddNode(node);
            HyperEdge* edge = new HyperEdge(node);
            node->AddEdge(edge); hg->AddEdge(edge);
            // If this is a terminal, add the string
            Trim(in, WHITE_SPACE);
            if(in.peek() != '(') {
                string val = ReadUntil(in, ")", WHITE_SPACE_OR_OPENPAREN);
                WordId wid = Dict::WID(val);
                hg->GetWords().push_back(wid);
                HyperNode* child = new HyperNode(wid, MakePair(pos,pos+1));
                hg->AddNode(child); edge->AddTail(child);
                ++pos;
            }
        } else {
            THROW_ERROR("Expecting parenthesis but got '"<<next_char<<"'");
        }
    }
    return NULL;
}

void PennTreeIO::WriteTree(const HyperGraph & tree, ostream & out) {
    THROW_ERROR("Not implemented yet");
}
