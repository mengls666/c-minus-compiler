#ifndef C_MINUS_COMPILER_GENTINY_H
#define C_MINUS_COMPILER_GENTINY_H
#include "analyze.h"
#include <string>
using namespace std;


#define NOT_FOUND -1

class fun {
public:
    string fun_name;
    int index;
    fun(string fun_name, int index);
};

class track {
public:
    Scope current_scope;
    vector<fun> funs;
    void add_func(string id, int index);
    int find_func(string id);
};

void gentiny(TreeNode * tree);
void gentiny_code(TreeNode * tree, track & track);

// DeclType : Var, Fun
void gentiny_decl(TreeNode * tree, track & track);
void gentiny_decl_var(TreeNode * node, track & track);
void gentiny_decl_fun(TreeNode * node, track & track);

// ExprType : Assign, Call, Op, Const, Id
void gentiny_expr(TreeNode * tree, track & track, bool isAddress=false);
void gentiny_expr_assign(TreeNode * node, track & track);
void gentiny_expr_call(TreeNode * node, track & track);
void gentiny_expr_op(TreeNode * node, track & track);
void gentiny_expr_const(TreeNode * node, track & track);
void gentiny_expr_id(TreeNode * node, track & track, bool isAddress);

// StmtType : ExprStmt, If, Iter, Return, Compound
void gentiny_stmt(TreeNode * tree, track & track);
void gentiny_stmt_expr(TreeNode * node, track & track);
void gentiny_stmt_if(TreeNode * node, track & track);
void gentiny_stmt_iter(TreeNode * node, track & track);
void gentiny_stmt_return(TreeNode * node, track & track);
void gentiny_stmt_compound(TreeNode * node, track & track);

#endif //C_MINUS_COMPILER_GENTINY_H