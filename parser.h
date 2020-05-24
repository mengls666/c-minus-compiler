#include <string>
#include <fstream>
#include "scanner.h"
using namespace std;

#define HASH_TABLE_SIZE 3533 // size of hash table, just a prime
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
class ScopeRec;
typedef ScopeRec* Scope;
const int MAX_child = 4;
typedef struct treeNode {
    struct treeNode * child[MAX_child];
    struct treeNode * sibling;
    int lineno;
    Nodekind nodekind;
    union {
        TokenType op;
        int val;
        char name[100];
    } attr;
    ExpType type;
    Scope scope;
} TreeNode;
/* The record in the bucket lists for each identifier,
*
*/
class BucketListRec {
public:
	// data members
	string id;
	vector<int> lines;
	TreeNode *node;
	int memloc;
	BucketListRec *next;

	// methods
	BucketListRec(string _id, TreeNode *_node, int _memloc) {
		id = _id; node = _node; memloc = _memloc;
	};
};
typedef vector<BucketListRec> BucketList;

/* The record of scope, maintaining one symbol table each */
class ScopeRec {
public:
	// data members
	string scopeName;
	int nestedLevel;
	ScopeRec *parentScope;
	/* symbol table of this scope*/
	BucketList hashTable[HASH_TABLE_SIZE];

	// methods
	ScopeRec(string _scopeName) { scopeName = _scopeName; };
};


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