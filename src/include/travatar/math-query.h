#ifndef TRAVATAR_MATH_QUERY__
#define TRAVATAR_MATH_QUERY__

#include <travatar/sentence.h>
#include <travatar/real.h>
#include <vector>
#include <map>
#include <string>

using namespace std;

namespace travatar {

enum TokenType {
    OPERAND=0, BINARY_OPERATOR, UNARY_OPERATOR, OPEN_PARENTHESES, CLOSE_PARENTHESES
};

struct MathToken;

class MathQuery{
    std::vector<MathToken*> tokens_;
public:
    MathQuery(std::string query="", std::map<WordId,Real> vars=std::map<WordId,Real>());
    virtual ~MathQuery();

    static Real Evaluate(const std::map<WordId,Real>& var_map, const std::string& query);
    static Real Evaluate(const MathQuery& mq);
    virtual void Print(std::ostream& oss) const;
};

inline std::ostream &operator<<( std::ostream &out, const MathQuery &L ) {
    L.Print(out);
    return out;
}

struct MathToken {
    int type_;
    MathToken(const int type) { type_ = type; }
    virtual ~MathToken() {}
    virtual void Print(std::ostream& oss) const { };
};

inline std::ostream &operator<<( std::ostream &out, const MathToken &L ) {
    L.Print(out);
    return out;
}

class BinaryOperator : public MathToken {
    int priority_;
    string op_;
public:
    BinaryOperator(int priority, const string & op) : MathToken(BINARY_OPERATOR)
        { priority_ = priority; op_ = op; } 
    virtual Real eval(Real x1, Real x2) = 0;
    const std::string GetOperator() const { return op_; }
    int GetPriority() const { return priority_; }
    virtual void Print(std::ostream& oss) const { oss << op_; }
};

struct Add : public BinaryOperator {
    Add() : BinaryOperator(0,"+") { }
    Real eval(Real x1, Real x2) { return x1 + x2; }
};

struct Subtract : public BinaryOperator {
    Subtract() : BinaryOperator(0,"-") { }
    Real eval(Real x1, Real x2) { return x1 - x2; }
};

struct Multiply : public BinaryOperator {
    Multiply() : BinaryOperator(1,"*") { }
    Real eval(Real x1, Real x2) { return x1 * x2; }
};

struct Divide : public BinaryOperator {
    Divide() : BinaryOperator(1,"/") { }
    Real eval(Real x1, Real x2) { return x1 / x2; }
};

class Operand : public MathToken {
    Real value_;
public: 
    Operand(Real value) : MathToken(OPERAND), value_(value) { };
    Real GetValue() const { return value_; } 
    void Print(std::ostream& oss) const { oss << value_; }
};

struct OpenParentheses : public MathToken {
    OpenParentheses() : MathToken(OPEN_PARENTHESES) { }
    void Print(std::ostream& oss) const { oss << std::string(1,'('); }
};

struct CloseParentheses :public MathToken {
    CloseParentheses() : MathToken(CLOSE_PARENTHESES) { }
    void Print(std::ostream& oss) const { oss << std::string(1,')'); }
};
} // namespace travatar

#endif
