#include <string>
#include <fstream>
#include "scanner.h"
using namespace std;
/*
node type list:
int, id, void, const, var, var_declaration,
array_declaration, function_dec,params_list,param,compound,
selection,iteration, if,while,return,assign,operation,array_element,call,args,
unknown nodes
*/
typedef enum {
    Int,Id,Void,Const,Var_dec,Arr_dec,
    Fun,Params,Param,
    Comp,Selec,Iter,Ret,Assign,Op,
    Arry_elem,Call,Args,Unkwn
} Nodekind;
typedef enum {Exp_VOID,Exp_INT} ExpType;

const int MAX_child = 4;
typedef struct treeNode {
    struct treeNode * child[MAX_child];
    struct treeNode * sibling;
    int lineno;
    Nodekind nodekind;
    union {
        TokenType op;
        int val;
        const char * name;
    } attr;
    ExpType type;
} TreeNode;

class Parser {
public:
    Parser(string filename);
    TreeNode * parse(void);
    TreeNode * synTree; //root
private:
    Scanner scanner;
    Token currentToken;
    Token lastToken;
    int tokenIndex;
    bool isError;
    int step;

    Token getToken();
    void  genError(string s);
    void match(TokenType ex);
    TreeNode * newNode(Nodekind k);
    TreeNode *declaration_list(void);
    TreeNode * declaration(void);
    TreeNode * params(void);
    TreeNode * params_list(TreeNode * k);
    TreeNode * param(TreeNode * k);
    TreeNode * compound(void);
    TreeNode * local_declaration(void);
    TreeNode * statement_list(void);
    TreeNode * statement(void);
    TreeNode * expression_stmt(void);
    TreeNode * selection(void);
    TreeNode * iteration(void);
    TreeNode * return_stmt(void);
    TreeNode * expression(void);
    TreeNode * var(void);
    TreeNode * simple_expression(TreeNode * k);
    TreeNode * additive_expression(TreeNode * k);
    TreeNode * term(TreeNode * k);
    TreeNode * factor(TreeNode * k);
    TreeNode * call(TreeNode * k);
    TreeNode * args(void);
    void printSpace(int n);
    void printTree(TreeNode *t);
};