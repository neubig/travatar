#include <list>
#include <travatar/tree-io.h>
#include <travatar/io-util.h>
#include <boost/regex.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

using namespace travatar;
using namespace std;
using namespace boost;
using namespace boost::property_tree;

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
        if(!in) break;
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
            THROW_ERROR("Expecting parenthesis but got '("<<(int)next_char<<")"<<next_char<<"'");
        }
    }
    return NULL;
}

void PennTreeIO::WriteTree(const HyperGraph & tree, ostream & out) {
    THROW_ERROR("Not implemented yet");
}


HyperGraph * JSONTreeIO::ReadTree(istream & in) {
    ptree pt;
    string line; getline(in, line); istringstream line_in(line);
    json_parser::read_json(line_in, pt);
    HyperGraph * ret = new HyperGraph;
    // Get each of the nodes
    BOOST_FOREACH(ptree::value_type &v, pt.get_child("nodes")) {
        HyperNode * node = new HyperNode;
        node->SetId(v.second.get<int>("id"));
        node->SetSym(Dict::WID(v.second.get<string>("sym")));
        ptree::const_iterator it = v.second.get_child("span").begin();
        int l = it->second.get<int>(""); int r = (++it)->second.get<int>("");
        node->SetSpan(MakePair(l, r));
        try {
            BOOST_FOREACH(ptree::value_type &t, v.second.get_child("trg_span"))
                node->GetTrgSpan().insert(t.second.get<int>(""));
        } catch (ptree_bad_path e) { }
        node->SetFrontier((HyperNode::FrontierType)v.second.get<char>("frontier", HyperNode::UNSET_FRONTIER));
        ret->AddNode(node);
    }
    BOOST_FOREACH(ptree::value_type &v, pt.get_child("edges")) {
        HyperEdge * edge = new HyperEdge;
        edge->SetId(v.second.get<int>("id"));
        edge->SetHead(ret->GetNode(v.second.get<int>("head")));
        edge->GetHead()->AddEdge(edge);
        try {
            BOOST_FOREACH(ptree::value_type &t, v.second.get_child("tails"))
                edge->AddTail(ret->GetNode(t.second.get<int>("")));
        } catch (ptree_bad_path e) { }
        try {
            BOOST_FOREACH(ptree::value_type &t, v.second.get_child("features"))
                edge->AddFeature(Dict::WID(t.first), t.second.get<double>(""));
        } catch (ptree_bad_path e) { }
        edge->SetScore(v.second.get<double>("score", 0.0));
        ret->AddEdge(edge);
    }
    BOOST_FOREACH(ptree::value_type &v, pt.get_child("words"))
        ret->AddWord(Dict::WID(v.second.get<string>("")));
    return ret;
}

void JSONTreeIO::WriteTree(const HyperGraph & tree, ostream & out) {
    out << "{\"nodes\": [";
    BOOST_FOREACH(HyperNode * node, tree.GetNodes()) {
        if(node->GetId() != 0) out << ", ";
        out << *node;
    }
    out << "], \"edges\": [";
    BOOST_FOREACH(HyperEdge * edge, tree.GetEdges()) {
        if(edge->GetId() != 0) out << ", ";
        out << *edge;
    }
    out << "], \"words\": [";
    const vector<int> & words = tree.GetWords();
    for(int i = 0; i < (int)words.size(); i++) {
        if(i != 0) out << ", ";
        out << "\"" << Dict::WSymEscaped(words[i]) << "\"";
    }
    out << "]}";
}

HyperNode * EgretTreeIO::MakeEgretNode(const string & str_id, SymbolSet<int> & node_map, HyperGraph * graph) {
    // Try to find the node
    int id = node_map.GetId(str_id, true);
    if(id < graph->NumNodes()) return graph->GetNode(id);
    else if (id != graph->NumNodes()) THROW_ERROR("Mismatched ID numbers in MakeEgretTreeNode()");
    // Make the node if it doesn't exist
    regex node_regex("(.*)\\[(\\d+),(\\d+)\\]");
    smatch node_match;
    HyperNode * new_node;
    if(!regex_match(str_id, node_match, node_regex)) {
        new_node = new HyperNode(Dict::WID(str_id));
    } else {
        new_node = new HyperNode(Dict::WID(node_match[1]), 
                                 MakePair(atoi(node_match[2].str().c_str()),
                                          atoi(node_match[3].str().c_str())+1));
    }
    graph->AddNode(new_node);
    return new_node;
}

HyperGraph * EgretTreeIO::ReadTree(istream & in) {
    HyperGraph * ret = new HyperGraph;
    string line, buff;
    double score;
    SymbolSet<int> node_map;
    // Make various regexes
    cmatch edge_match, sent_match;
    if(!getline(in,line)) return NULL;
    if(line.substr(0,8) != "sentence") THROW_ERROR("Missing sentence line: " << endl);
    // Create the sentence and root node
    if(!getline(in,line)) THROW_ERROR("partial egret output");
    ret->SetWords(Dict::ParseWords(line));
    // Get the lines one by one
    HyperNode * head = NULL;
    while(getline(in, line)) {
        // If we've finished, swap the root to the first position and return
        if(line == "") {
            if(head == NULL) THROW_ERROR("partial egret output");
            int head_pos = head->GetId();
            vector<HyperNode*> & nodes = ret->GetNodes();
            nodes[head_pos] = nodes[0]; nodes[head_pos]->SetId(head_pos);
            nodes[0] = head; head->SetId(0);
            return ret;
        }
        istringstream iss(line);
        // Get the head
        iss >> buff;
        head = MakeEgretNode(buff, node_map, ret);
        HyperEdge * edge = new HyperEdge(head); ret->AddEdge(edge); head->AddEdge(edge);
        // The next string should always be "=>"
        iss >> buff;
        if(buff != "=>") THROW_ERROR("=> not found in Egret node string: " << line);
        // Read the tail nodes
        while(iss >> buff && buff != "|||") {
            HyperNode * tail = MakeEgretNode(buff, node_map, ret);
            // Set terminal node's spans to those of their parent
            if(tail->GetSpan().first == -1)
                tail->SetSpan(head->GetSpan());
            edge->AddTail(tail);
        }
        if(buff != "|||") THROW_ERROR("||| not found in Egret node string: " << line);
        // Finally, read the score and add it to a parse
        iss >> score;
        edge->SetScore(score); edge->AddFeature(Dict::WID("parse"), score);
    }
    return NULL;
}

inline void PrintNodeEgret(const HyperNode * node, ostream & out) {
    out << Dict::WSym(node->GetSym());
    if(!node->IsTerminal())
        out << "[" << node->GetSpan().first << "," << node->GetSpan().second << "]";
}

void EgretTreeIO::WriteTree(const HyperGraph & tree, ostream & out) {
    out << "sentence :" << endl << Dict::PrintWords(tree.GetWords()) << endl;
    BOOST_REVERSE_FOREACH(const HyperEdge * edge, tree.GetEdges()) {
        PrintNodeEgret(edge->GetHead(), out);
        out << " =>";
        BOOST_FOREACH(const HyperNode * tail, edge->GetTails()) {
            out << " ";
            PrintNodeEgret(tail, out);
        }
        out << " ||| " << edge->GetScore() << endl;
    }
    out << endl;
}
