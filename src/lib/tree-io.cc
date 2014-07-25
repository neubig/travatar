#include <list>
#include <travatar/dict.h>
#include <travatar/tree-io.h>
#include <travatar/io-util.h>
#include <travatar/hyper-graph.h>
#include <travatar/symbol-set.h>
#include <travatar/global-debug.h>
#include <travatar/softmax.h>
#include <boost/regex.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>


using namespace travatar;
using namespace std;
using namespace boost;
using namespace boost::property_tree;

HyperGraph * WordTreeIO::ReadTree(istream & in) {
    string line;
    if(!getline(in,line)) return NULL;
    Sentence words = Dict::ParseWords(line);
    // Create the hypergraph
    HyperGraph * hg = new HyperGraph;
    hg->SetWords(words);
    HyperNode* root = new HyperNode(Dict::WID("X"), -1, make_pair(0,words.size())); hg->AddNode(root);
    HyperEdge * edge = new HyperEdge(root); root->AddEdge(edge); hg->AddEdge(edge);
    for(int i = 0; i < (int)words.size(); i++) {
        HyperNode* child = new HyperNode(Dict::WID("X"), -1, make_pair(i, i+1)); hg->AddNode(child); edge->AddTail(child);
        HyperEdge * edge2 = new HyperEdge(child); child->AddEdge(edge2); hg->AddEdge(edge2);
        HyperNode* child2 = new HyperNode(words[i], -1, make_pair(i, i+1)); hg->AddNode(child2); edge2->AddTail(child2);
    }
    return hg; 
}

void WordTreeIO::WriteTree(const HyperGraph & tree, ostream & out) {
    out << Dict::PrintWords(tree.GetWords());
}

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
        IoUtil::Trim(in, WHITE_SPACE);
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
            string sym = IoUtil::ReadUntil(in, WHITE_SPACE, "()");
            // TODO: Make this die properly
            if(!sym.length()) { getline(in, line); THROW_ERROR("Empty symbol at '("<<line<<"'"); }
            // Create a new node
            HyperNode* node = new HyperNode(Dict::WID(sym), -1, make_pair(pos,-1));
            stack.push_back(node); hg->AddNode(node);
            HyperEdge* edge = new HyperEdge(node);
            node->AddEdge(edge); hg->AddEdge(edge);
            // If this is a terminal, add the string
            IoUtil::Trim(in, WHITE_SPACE);
            if(in.peek() != '(') {
                string val = IoUtil::ReadUntil(in, ")", WHITE_SPACE_OR_OPENPAREN);
                WordId wid = Dict::WID(val);
                hg->GetWords().push_back(wid);
                HyperNode* child = new HyperNode(wid, -1, make_pair(pos,pos+1));
                hg->AddNode(child); edge->AddTail(child);
                ++pos;
            }
        } else {
            THROW_ERROR("Expecting parenthesis but got '("<<(int)next_char<<")"<<next_char<<"'");
        }
    }
    delete hg;
    return NULL;
}

void PennTreeIO::WriteNode(const vector<WordId> & words, 
                           const HyperNode & node, ostream & out) {
    if (node.NumEdges() > 1)
        THROW_ERROR("Cannot write hypergraphs of degree > 1 to Penn format");
    if (node.NumEdges() == 1) {
        out << "(" << Dict::WSym(node.GetSym());
        BOOST_FOREACH(const HyperNode * child, node.GetEdge(0)->GetTails()) {
            out << " ";
            WriteNode(words, *child, out);
        }
        out << ")";
    } else {
        out << Dict::WSym(words[node.GetSpan().first]);
    }
}

void PennTreeIO::WriteTree(const HyperGraph & tree, ostream & out) {
    WriteNode(tree.GetWords(), *tree.GetNode(0), out);
}

HyperGraph * RuleTreeIO::ReadTree(istream & in) {
    // The new hypergraph and stack to read the nodes
    HyperGraph * hg = new HyperGraph;
    vector<HyperNode*> stack;
    string str;
    int pos = 0;
    // Continue until the end of the corpus
    while(in >> str) {
        if(str == ")") {
            if(!stack.size()) { getline(in, str); THROW_ERROR("Unmatched close parenthesis at )" << str); }
            HyperNode * child = *stack.rbegin(); stack.pop_back();
            child->GetSpan().second = pos;
            // If no parent exists, we are at the root. Return.
            if(!stack.size()) return hg;
            // If a parent exists, add the child to its tails
            HyperNode * parent = *stack.rbegin();
            parent->GetEdge(0)->AddTail(child);
        } else if(str.length() > 1 && str[0] == '"' && str[str.length()-1] == '"') {
            WordId wid = Dict::WID(str.substr(1, str.length()-2));
            hg->GetWords().push_back(wid);
            HyperNode* child = new HyperNode(wid, -1, make_pair(pos,pos+1));
            hg->AddNode(child);
            if(!stack.size()) return hg;
            // If a parent exists, add the child to its tails
            HyperNode * parent = *stack.rbegin();
            parent->GetEdge(0)->AddTail(child);
            pos++;
        } else {
            // Create a new node
            HyperNode* node = new HyperNode(Dict::WID(str), -1, make_pair(pos,-1));
            stack.push_back(node); hg->AddNode(node);
            HyperEdge* edge = new HyperEdge(node);
            node->AddEdge(edge); hg->AddEdge(edge);
            if(!(in >> str) || str != "(") 
                THROW_ERROR("Expecting open paren but got: " << str);
        }
    }
    delete hg;
    return NULL;
}

void RuleTreeIO::WriteNode(const vector<WordId> & words, 
                           const HyperNode & node, ostream & out) {
    if (node.NumEdges() > 1)
        THROW_ERROR("Cannot write hypergraphs of degree > 1 to Rule format");
    if (node.NumEdges() == 1) {
        out << Dict::WSym(node.GetSym()) << " ( ";
        BOOST_FOREACH(const HyperNode * child, node.GetEdge(0)->GetTails()) {
            WriteNode(words, *child, out);
            out << ' ';
        }
        out << ')';
    } else {
        out << '"' << Dict::WSym(words[node.GetSpan().first]) << '"';
    }
}

void RuleTreeIO::WriteTree(const HyperGraph & tree, ostream & out) {
    WriteNode(tree.GetWords(), *tree.GetNode(0), out);
}

HyperGraph * JSONTreeIO::ReadTree(istream & in) {
    ptree pt;
    string line; 
    if(!getline(in, line))
        return NULL;
    istringstream line_in(line);
    json_parser::read_json(line_in, pt);
    HyperGraph * ret = new HyperGraph;
    // Get each of the nodes
    BOOST_FOREACH(ptree::value_type &v, pt.get_child("nodes")) {
        HyperNode * node = new HyperNode;
        node->SetId(v.second.get<int>("id"));
        node->SetSym(Dict::WID(v.second.get<string>("sym")));
        ptree::const_iterator it = v.second.get_child("span").begin();
        int l = it->second.get<int>(""); int r = (++it)->second.get<int>("");
        node->SetSpan(make_pair(l, r));
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
            vector<SparsePair> temp;
            BOOST_FOREACH(ptree::value_type &t, v.second.get_child("features"))
                temp.push_back(make_pair(Dict::WID(t.first), t.second.get<double>("")));
            edge->SetFeatures(SparseVector(temp));
            // edge->AddFeature(Dict::WID(t.first), t.second.get<double>(""));
        } catch (ptree_bad_path e) { }
        try {
            BOOST_FOREACH(ptree::value_type &t, v.second.get_child("trg")) {
                CfgData trg_data;
                // Get the words
                try {
                    BOOST_FOREACH(ptree::value_type &x, t.second.get_child("words")) {
                        int val = x.second.get<int>("", INT_MAX);
                        if(val == INT_MAX)
                            val = Dict::WID(x.second.get<string>(""));
                        trg_data.words.push_back(val);
                    }
                } catch (ptree_bad_path e) { }
                // TODO: Get the symbols, head symbol
                edge->GetTrgData().push_back(trg_data);
            }
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
        out << Dict::WSymEscaped(words[i]);
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
                                 -1,
                                 make_pair(atoi(node_match[2].str().c_str()),
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
    string wordstring;
    if(!getline(in,wordstring)) THROW_ERROR("Egret file ended prematurely");
    ret->SetWords(Dict::ParseWords(wordstring));
    // Get the lines one by one and reverse
    vector<string> lines;
    while(getline(in, line) && line != "")
        lines.push_back(line);
    // If we have a failed parse, return the source words
    if(lines.size() == 0) {
        getline(in,line);
        WordTreeIO wtio;
        istringstream iss(wordstring);
        return wtio.ReadTree(iss);
    }
    // Save the parse ID, and also vectors of scores
    WordId parse_id = Dict::WID("parse");
    // For each line
    BOOST_REVERSE_FOREACH(const std::string & line, lines) {
        istringstream iss(line);
        // Get the head
        iss >> buff;
        HyperNode * head = MakeEgretNode(buff, node_map, ret);
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
        edge->SetScore(score);
        if(!normalize_) edge->GetFeatures().Add(parse_id, score);
    }
    // Because Egret outputs posterior probabilities of the edge appearing in
    // the tree, not the relative probability of each edge coming out of a
    // particular node, we need to normalize by nodes
    if(normalize_) {
        BOOST_FOREACH(HyperNode* node, ret->GetNodes()) {
            vector<double> scores; scores.reserve(node->GetEdges().size());
            BOOST_FOREACH(HyperEdge * edge, node->GetEdges())
                scores.push_back(edge->GetScore());
            double denom = AddLogProbs(scores);
            for(int i = 0; i < (int)scores.size(); i++) {
                HyperEdge * edge = node->GetEdge(i);
                double score = scores[i]-denom;
                edge->SetScore(score);
                edge->GetFeatures().Add(parse_id, score);
            }
        }
    }
    return ret;
}

inline void PrintNodeEgret(const HyperNode * node, ostream & out) {
    out << Dict::WSym(node->GetSym());
    if(!node->IsTerminal())
        out << "[" << node->GetSpan().first << "," << node->GetSpan().second-1 << "]";
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
}

HyperGraph * MosesXMLTreeIO::ReadTree(istream & in) {
    THROW_ERROR("Moses XML is not supported as an input format (yet)");
}

void MosesXMLTreeIO::WriteNode(const vector<WordId> & words, 
                           const HyperNode & node, ostream & out) {
    if (node.NumEdges() > 1)
        THROW_ERROR("Cannot write hypergraphs of degree > 1 to Moses XML format");
    if (node.NumEdges() == 1) {
        out << "<tree label=\"" << Dict::EncodeXML(Dict::WSym(node.GetSym())) << "\">";
        BOOST_FOREACH(const HyperNode * child, node.GetEdge(0)->GetTails()) {
            out << " ";
            WriteNode(words, *child, out);
        }
        out << " </tree>";
    } else {
        out << Dict::EncodeXML(Dict::WSym(words[node.GetSpan().first]));
    }
}

void MosesXMLTreeIO::WriteTree(const HyperGraph & tree, ostream & out) {
    WriteNode(tree.GetWords(), *tree.GetNode(0), out);
}
