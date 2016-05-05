#include "main.hpp"

#include "jit/jit-dump.h"

#include <dlfcn.h>
#include <stdio.h>
#include <iostream>
#include <map>
#include <vector>
#include <string>

extern int yyparse();
extern FILE * yyin;

extern "C" int yywrap() {
    return 1;
}
extern "C" void yyerror(const char * str) {
    std::cerr << "Error parsing: " << str << std::endl;
}

static jit_context_t ctx;

void processDefinition(FunctionAST * f) {
    jit_context_build_start(ctx);
    auto fn = f->Codegen(ctx);
    if (fn) {
        std::cout << "Function defined" << std::endl;
    }
    jit_context_build_end(ctx);
}
void processExtern(ExternAST * f) {
    jit_context_build_start(ctx);
    auto fn = f->Codegen(ctx);
    if (fn) {
        std::cout << "Extern defined" << std::endl;
    }
    jit_context_build_end(ctx);
}
void processTopLevelExpr(FunctionAST * f) {
    jit_context_build_start(ctx);
    auto fn = f->Codegen(ctx);
    jit_context_build_end(ctx);

    if (!fn) return;

    jit_float64 res;
    jit_function_apply(fn, 0, &res);
    std::cout << "Result from apply: " << res << std::endl;
    
    jit_float64 (*closure)();
    closure = (jit_float64 (*)())jit_function_to_closure(fn);
    if (!closure) return;

    std::cout << "Result from closure: " << closure() << std::endl;
}

int main() {
    ctx = jit_context_create();

    yyin = stdin;
    yyparse();
    return 0;
}

static std::map<std::string, jit_value_t> NamedValues;
static std::map<std::string, jit_function_t> NamedFunctions;

jit_value_t NumberExprAST::Codegen(jit_function_t fn) {
    return jit_value_create_float64_constant(fn, jit_type_float64, Val);
}
jit_value_t VariableExprAST::Codegen(jit_function_t fn) {
    auto res = NamedValues[Name];
    if (!res) std::cerr << "Variable not found: " << Name << std::endl;
    return res;
}
jit_value_t BinaryExprAST::Codegen(jit_function_t fn) {
    auto l = LHS->Codegen(fn);
    auto r = RHS->Codegen(fn);
    if (!l || !r) return 0;

    switch (Op) {
        case '+': return jit_insn_add(fn, l, r);
        case '-': return jit_insn_sub(fn, l, r);
        case '*': return jit_insn_mul(fn, l, r);
        case '/': return jit_insn_div(fn, l, r);
        case '<': return jit_insn_lt(fn, l, r);
        case '>': return jit_insn_gt(fn, l, r);
    }
    std::cerr << "Invalid operator: " << Op << std::endl;
    return 0;
}
jit_value_t CallExprAST::Codegen(jit_function_t fn) {
    jit_function_t callee = NamedFunctions[Callee];
    if (!callee) {
        std::cerr << "Function not found: " << Callee << std::endl;
        return 0;
    }
    // TODO: How to validate that?
    //if (Callee.size() != Args.size()) {
    //    std::cerr << "Number of arguments differs: " << Callee.size() << " v. " << Args.size() << std::endl;
    //    return 0;
    //}
    jit_value_t * args = new jit_value_t[Args.size()];
    for (int i = 0; i < Args.size(); i++) {
        args[i] = Args[i]->Codegen(fn);
        if (!args[i]) return 0;
    }
    return jit_insn_call(fn, 0, callee, jit_function_get_signature(callee), args, Args.size(), JIT_CALL_NOTHROW);
}

jit_type_t PrototypeAST::Signature(jit_context_t ctx) {
    if (NamedFunctions[Name]) {
        // Took the easy route here. LLVM's example goes thru a larger route of testing
        // if function body is compatible to allow overriding, etc. Their reasoning for
        // that seems to delegate creation of functions to Prototype, then overriding
        // as necessary.
        std::cerr << "Duplicate function: " << Name << std::endl;
        return 0;
    }
    jit_type_t * params = new jit_type_t[Args.size()];
    for (int i = 0; i < Args.size(); i++) {
        params[i] = jit_type_float64;
    }
    return jit_type_create_signature(jit_abi_cdecl, jit_type_float64, params, Args.size(), 1);
}
jit_value_t * PrototypeAST::GenArgs(jit_function_t fn) {
    // I admit, this needs refactoring
    NamedFunctions[Name] = fn;

    // In LLVM's example, they clear the NamedValues only for Functions. This probably
    // means they are "leaking" variable names from Externs. Since our only scope is
    // inside a function, we always clear the names, even for Externs
    NamedValues.clear();

    jit_value_t * args = new jit_value_t[Args.size()];
    for (int i = 0; i < Args.size(); i++) {
        NamedValues[Args[i]] = args[i] = jit_value_get_param(fn, i);
    }
    return args;
}

jit_function_t ExternAST::Codegen(jit_context_t ctx) {
    jit_type_t sign = Proto->Signature(ctx);
    if (!sign) return 0;

    void * exe = dlopen(0, 0);
    void * native = dlsym(exe, Proto->CName());
    dlclose(exe);

    if (!native) {
        std::cerr << "Invalid extern: " << Proto->CName() << std::endl;
        return 0;
    }

    jit_function_t fn = jit_function_create(ctx, sign);
    jit_value_t * args = Proto->GenArgs(fn);

    // Until I find a way to create an "alias", just like what LLVM example does,
    // we are sticking with a proxy call. Also, the name is just for
    // debugging purposes, in case you are wondering.
    jit_value_t ret = jit_insn_call_native(fn, Proto->CName(), native, sign, args, Proto->NumArgs(), JIT_CALL_NOTHROW);
    jit_insn_return(fn, ret);

    jit_dump_function(stdout, fn, 0); // last parameter is the name
    jit_function_compile(fn);
    return fn;
}

jit_function_t FunctionAST::Codegen(jit_context_t ctx) {
    jit_type_t sign = Proto->Signature(ctx);
    if (!sign) return 0;

    jit_function_t fn = jit_function_create(ctx, sign);

    jit_value_t * args = Proto->GenArgs(fn);

    jit_value_t ret = Body->Codegen(fn);
    if (!ret) return 0;

    jit_insn_return(fn, ret);

    jit_dump_function(stdout, fn, 0); // last parameter is the name
    jit_function_compile(fn);
    return fn;
}

