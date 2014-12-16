#include <travatar/global-debug.h>
#include <travatar/dict.h>
#include <travatar/sentence.h>
#include <travatar/math-query.h>
#include <boost/foreach.hpp>
#include <stack> 
using namespace travatar;
using namespace std;
using namespace boost;

namespace travatar {

MathQuery::MathQuery(const std::string query, const std::map<WordId,Real> var_map) {
    // Parse the query into data structure
    int read = -1;
    for (size_t i=0; i < query.size(); ++i) {
        char c = query[i];
        
        // Case of reading number
        if (read == -1 && c >= '0' && c <= '9') {
            read = i;
        } 
        if (read == -1 || !((c >= '0' && c <= '9') || c == '.')) {
            // We are reading number before and we are done reading number!
            if (read != -1) {
                tokens_.push_back(new Operand(std::atof(query.substr(read,i-read).c_str())));
                read = -1;
            }
            
            if (c >= 'A' && c <= 'Z') {
               std::map<WordId,Real>::const_iterator it = var_map.find(Dict::WID(string(1,c)));
               if (it != var_map.end()) 
                    tokens_.push_back(new Operand(it->second));
               else 
                    THROW_ERROR("Could not find variable " << c << " in evaluation measure. Did you define this variable in the -eval properly?");    
            } else if (c == '(') {
                tokens_.push_back(new OpenParentheses());
            } else if (c == ')') {
                tokens_.push_back(new CloseParentheses());
            } else if (c == '+') {
                tokens_.push_back(new Add());
            } else if (c == '-') {
                tokens_.push_back(new Subtract());
            } else if (c == '*') {
                tokens_.push_back(new Multiply());
            } else if (c == '/') {
                tokens_.push_back(new Divide());
            } else {
                if (c == ' ') { /* ignore */ }
                else THROW_ERROR("Unsupported token in query: '" << c << "'");
            }
        }
    }
    // In case the last token is a number
    if (read != -1) 
        tokens_.push_back(new Operand(std::atof(query.substr(read,query.length()-read).c_str())));
}

MathQuery::~MathQuery() {
    BOOST_FOREACH(MathToken* tok, tokens_)
        delete tok;
}

Real MathQuery::Evaluate(const std::map<WordId,Real> & var_map, const std::string & query_str) {
    MathQuery query(query_str, var_map);
    return Evaluate(query);
}

Real MathQuery::Evaluate(const MathQuery& query) {
    stack<MathToken*> temp;
    vector<MathToken*> postfix;
    
    // Convert to postfix notation
    BOOST_FOREACH(MathToken* tok, query.tokens_) {
        int c = tok->type_;
        if (c == OPERAND) {
            postfix.push_back(tok);
        } else if (c == OPEN_PARENTHESES) {
            temp.push(tok);
        } else if (c == BINARY_OPERATOR) {
            BinaryOperator* op = dynamic_cast<BinaryOperator*>(tok);
            while(!temp.empty()) {
                BinaryOperator* top = dynamic_cast<BinaryOperator*>(temp.top());
                if (/*dynamic_cast<OpenParentheses*>(top) != 0 || (*/top && top->GetPriority() >= op->GetPriority()) {
                    top = NULL;
                    postfix.push_back(temp.top());
                    temp.pop();
                } else { 
                    top = NULL;
                    break;
                }
            }
            op = NULL;
            temp.push(tok);
        } else if (c == CLOSE_PARENTHESES) {
            while(!temp.empty() && temp.top()->type_ != OPEN_PARENTHESES) {
                postfix.push_back(temp.top());
                temp.pop();
            }
            if ((int) temp.size() == 0) {
                THROW_ERROR("Too many closing bracket in query: " << query);
            } else {
                temp.pop();
            }
        } else {
            THROW_ERROR("Undefined token type: " << c);
        }
    }
    while (!temp.empty()) {
        if (temp.top()->type_ == OPEN_PARENTHESES) {
            THROW_ERROR("Too many opening bracket in query: " << query);
        } else {
            postfix.push_back(temp.top());
            temp.pop();
        }
    }
    
    // DEBUG:
    // cerr << "POSTFIX: ";
    // for (size_t i=0; i < postfix.size(); ++i) { 
    //    if (i) cerr << " ";
    //     cerr << *(postfix[i]);
    // }
    // cerr << endl;
    
    // Process all the postfix token
    stack<Real> tmp;
    BOOST_FOREACH (MathToken* tok, postfix) {
        int c = tok->type_;
        if (c == OPERAND) {
            // DEBUG: cerr << dynamic_cast<Operand*>(tok)->GetValue() << endl;
            tmp.push(dynamic_cast<Operand*>(tok)->GetValue()); 
        } else if (c == BINARY_OPERATOR) {
            Real c1, c2;
            if (tmp.size() == 0) 
                THROW_ERROR("Not enough operand on query: " << query);
            c2 = tmp.top();
            tmp.pop();
            if (tmp.size() == 0)
                THROW_ERROR("Not enough operand on query: " << query);
            c1 = tmp.top();
            tmp.pop();
            BinaryOperator* op = dynamic_cast<BinaryOperator*>(tok);
            // DEBUG: cerr << c1 << op->GetOperator() << c2 << endl;
            Real result = op->eval(c1,c2);
            tmp.push(result);
            op = NULL;
        } else {
            THROW_ERROR("Unknown error");
        }
        tok = NULL;
    }
    Real ret;
    if (tmp.size() == 0) { 
        ret= 0;
    } else {
        ret = tmp.top();
        tmp.pop();
        if (tmp.size() != 0) 
            THROW_ERROR("Not enough operator on query: " << query);
    }
    return ret;
}

void MathQuery::Print(ostream & oss) const {
    BOOST_FOREACH(MathToken* tok, tokens_) 
        oss << *tok;
}
} // namespace travatar

