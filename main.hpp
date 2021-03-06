#include <jit/jit.h>

#include <string>
#include <vector>

/// ExprAST - Base class for all expression nodes.
class ExprAST {
public:
    virtual ~ExprAST() {}
    virtual jit_value_t Codegen(jit_function_t) = 0;
};

/// NumberExprAST - Expression class for numeric literals like "1.0".
class NumberExprAST : public ExprAST {
    double Val;
public:
    NumberExprAST(double val) : Val(val) {}
    virtual jit_value_t Codegen(jit_function_t);
};

/// VariableExprAST - Expression class for referencing a variable, like "a".
class VariableExprAST : public ExprAST {
    std::string Name;
public:
    VariableExprAST(const std::string &name) : Name(name) {}
    virtual jit_value_t Codegen(jit_function_t);
};

/// BinaryExprAST - Expression class for a binary operator.
class BinaryExprAST : public ExprAST {
    char Op;
    ExprAST *LHS, *RHS;
public:
    BinaryExprAST(char op, ExprAST *lhs, ExprAST *rhs) : Op(op), LHS(lhs), RHS(rhs) {}
    virtual jit_value_t Codegen(jit_function_t);
};

/// CallExprAST - Expression class for function calls.
class CallExprAST : public ExprAST {
    std::string Callee;
    std::vector<ExprAST*> Args;
public:
    CallExprAST(const std::string &callee, std::vector<ExprAST*> &args) : Callee(callee), Args(args) {}
    virtual jit_value_t Codegen(jit_function_t);
};

/// PrototypeAST - This class represents the "prototype" for a function,
/// which captures its name, and its argument names (thus implicitly the number
/// of arguments the function takes).
class PrototypeAST {
    std::string Name;
    std::vector<std::string> Args;
public:
    PrototypeAST(const std::string &name, const std::vector<std::string> &args) : Name(name), Args(args) {}
    jit_type_t Signature(jit_context_t);
    jit_value_t * GenArgs(jit_function_t);

    const char * CName() { return Name.c_str(); }
    int NumArgs() { return Args.size(); }
};

class ExternAST {
    PrototypeAST *Proto;
public:
    ExternAST(PrototypeAST *proto) : Proto(proto) {}
    jit_function_t Codegen(jit_context_t ctx);
};

/// FunctionAST - This class represents a function definition itself.
class FunctionAST {
    PrototypeAST *Proto;
    ExprAST *Body;
public:
    FunctionAST(PrototypeAST *proto, ExprAST *body) : Proto(proto), Body(body) {}
    jit_function_t Codegen(jit_context_t ctx);
};

typedef std::vector<std::string> StringList;
typedef std::vector<ExprAST *> ExprList;

void processDefinition(FunctionAST *);
void processExtern(ExternAST *);
void processTopLevelExpr(FunctionAST *);

