/* semantic analysis process */
#include "analyze.h"
#include <iostream>
using namespace std;
bool TraceAnalyze = 1;
FILE *listing;
extern Scope global;
bool Error = false;
static int location = 0; // allocation of memory location

// declFunc -> true; makes the first compoundStmt in a function not nested
static bool preserveLastScope = false; // maintain nested scope (Comp)

static string analyzingFuncName;

/* Procedure traverse is a generic recursive syntax tree traversal routine:
* it applies preProc in preorder and postProc  in postorder to tree pointed to by t
* reference: Appendix A-ANALYZE.c
*/
static void traverse(TreeNode * t, void(*preProc) (TreeNode *), void(*postProc) (TreeNode *)){
	/* receives preProc and postProc as two functions, e.g.
	* traverse(syntaxTree, insertNode, nullProc); traverse(syntaxTree, nullProc, checkNode);
	*/
	if (t != NULL){
		preProc(t);
		for (int i = 0; i < 4; i++) {
			if((t->nodekind == Var_dec || t->nodekind==Fun) && i==1) continue;
			if(i>=1 &&  t->nodekind == Param) continue;
			traverse(t->child[i], preProc, postProc);
		}
		postProc(t);
		traverse(t->sibling, preProc, postProc); // next equals to sibling in tree structure
	}
}


/* nullProc is a do-nothing procedure to generate preorder-only or 
 * postorder-only traversals from traverse (maybe useless in C--)
 */
static void nullProc(TreeNode * t){
	if (t == NULL) return;
	else return;
}

/* output the error message(lineno, error message...) */
static void typeError(TreeNode *node, string message) {
	fprintf(listing, "Error in Line[%d]: %s, attr.name:%s\n", node->lineno, message.c_str(),node->attr.name);
	Error = true; // static error signal in analysis
}

/* insertNode inserts t's identifier into the symbol table */
static void insertNode(TreeNode * t) {
	switch (t->nodekind)
	{
	// only handles Compound - creating local scopes
	    case Comp:
			if (preserveLastScope) // first level scope of function
				preserveLastScope = false;
			else {
				Scope scope = sc_create(analyzingFuncName);
				sc_push(scope);
				t->scope = scope;
            }
            break;
	// add lineno when node is IdExprNode/CallExprNode
		case Id:
		{
			TreeNode *idNode = t;
			if (st_lookup(sc_top(), idNode->attr.name) < 0) {
				typeError(t, "undeclared variable");
			}
			else {
				/* NULL in loc field means add in lineno field */
				if(st_lookup_nonest(sc_top(), idNode->attr.name) < 0){
					// not in current scope, invoke st_insert_nearest
					st_insert_nearest(idNode->attr.name, idNode->lineno, 0, t);
				}
				else {
					st_insert(idNode->attr.name, idNode->lineno, 0, t);
				}
			}
			break;
		}
		case Call:
		{
			TreeNode * callNode = t;
			if (st_lookup(sc_top(), callNode->child[0]->attr.name) < 0) {
				typeError(t, "undeclared function");
			}
			else {
				st_insert_nearest(callNode->attr.name, callNode->lineno, 0, t);
			}
			break;
		}


		case Var_dec:
		{
			TreeNode * varNode = t;
			if (varNode->child[0]->nodekind == Void) {
				typeError(t, "variable can not have void data type");
				break;
			}
            if(varNode->child[1]->nodekind == Arr_dec) {
                if (st_lookup_nonest(sc_top(), varNode->child[1]->child[0]->attr.name) < 0) {
			        int size = varNode->child[1]->child[1]->attr.val;
				    st_insert(varNode->child[1]->child[0]->attr.name, varNode->lineno, size ,t);
			    }
			    else {
				    typeError(t, "the variable has been declared in the current scope");
			    }
            }
			else {
                if (st_lookup_nonest(sc_top(), varNode->child[1]->attr.name) < 0) {
			        int size = 1;
				    st_insert(varNode->child[1] ->attr.name, varNode->lineno, size ,t);
			    }
			    else {
				    typeError(t, "the variable has been declared in the current scope");
			    }
            }
			break;
		}
		case Param:
		{
			TreeNode * varNode = t;
			if (varNode->child[0]->nodekind == Void) {
				typeError(t, "variable can not have void data type");
				break;
			}
            if(varNode->child[2]!= NULL && varNode->child[2]->nodekind == Var_dec) {
                if (st_lookup_nonest(sc_top(), varNode->child[1]->attr.name) < 0) {
			        int size = 0;
				    st_insert(varNode->child[1]->attr.name, varNode->lineno, size ,t);
			    }
			    else {
				    typeError(t, "the variable has been declared in the current scope");
			    }
            }
			else {
                if (st_lookup_nonest(sc_top(), varNode->child[1]->attr.name) < 0) {
			        int size = 1;
				    st_insert(varNode->child[1] ->attr.name, varNode->lineno, size ,t);
			    }
			    else {
				    typeError(t, "the variable has been declared in the current scope");
			    }
            }
			break;
		}
		case Fun:
		{
			// declaration of function will create a new scope for it
			TreeNode * funNode = t;
			if (st_lookup(sc_top(), funNode->child[1]->attr.name) >= 0) {
				typeError(t, "the function has already been declared");
				break;
			}
			/* funtion record in current symbol table but create a new scope*/
			st_insert(funNode->child[1]->attr.name, funNode->lineno, 0, t);
			sc_push(sc_create(funNode->child[1]->attr.name));
			t->scope = sc_top();
			preserveLastScope = true;
			analyzingFuncName = funNode->child[1]->attr.name;
			break;
		}
	    default:
		    break;
	}
	return;
}


/* initSymTab initialize global symbol table,
 * inserting id input/output funtions into "global" scope table
 * int input(void){...}; void output(int x){...};
 */
static void initSymTab() {
	// create global scope
	global = sc_create("global");
	sc_push(global);

	// input & output
	TreeNode *inputDecl = (TreeNode *)malloc(sizeof(TreeNode));
    inputDecl->nodekind = Fun;
    TreeNode *ret = (TreeNode *)malloc(sizeof(TreeNode));
    ret->nodekind = Int;
    inputDecl->child[0] = ret;
    TreeNode *inputId = (TreeNode *)malloc(sizeof(TreeNode));
    sprintf(inputId->attr.name,"input");
    inputDecl->child[1] = inputId;
    TreeNode *params = (TreeNode *)malloc(sizeof(TreeNode));
    params->nodekind = Params;
    inputDecl->child[2] = params;
    TreeNode *paramVoid = (TreeNode *)malloc(sizeof(TreeNode));
    paramVoid->nodekind = Void;
    params->child[0] = paramVoid;
	TreeNode *outputDecl = (TreeNode *)malloc(sizeof(TreeNode));
    outputDecl->nodekind = Fun;
    TreeNode *ret1 = (TreeNode *)malloc(sizeof(TreeNode));
    ret1->nodekind = Void;
    outputDecl->child[0] = ret1;
    TreeNode *outputId = (TreeNode *)malloc(sizeof(TreeNode));
    sprintf(outputId->attr.name,"output");
    outputDecl->child[1]=outputId;
    TreeNode *params1 = (TreeNode *)malloc(sizeof(TreeNode));
    params1->nodekind = Params;
    outputDecl->child[2]=params1;
    TreeNode * paramX = (TreeNode *)malloc(sizeof(TreeNode));
    paramX->nodekind = Param;
    params1->child[0] = paramX;
    TreeNode * paramX_int = (TreeNode *)malloc(sizeof(TreeNode));
    paramX_int->nodekind = Int;
    params1->child[0] = paramX_int;
    TreeNode * paramX_id = (TreeNode *)malloc(sizeof(TreeNode));
    paramX_id->nodekind = Id;
    sprintf(paramX_id->attr.name,"x");
    params1->child[1] = paramX_id;

	/* insert the input/output as system call into global, lineno 0*/
    st_insert("input", 0, 0, inputDecl);
    st_insert("output", 0, 0, outputDecl);

	return;
}


/* type checking on one node */
static void checkNode(TreeNode *t) {
	
	switch (t->nodekind)
	{
	    case Var_dec:
        case Arr_dec:
        case Fun:
	        break;

	    case Comp:
		    sc_pop();
		    break;
	    case Selec:
	    case Iter:
		{
		/* In if & iter stmt, cond's type cannot be void, which may be a function call return */

	
		    TreeNode *condNode = t->child[0];
		    if (condNode->nodekind == Void) {
		    	typeError(condNode, "invalid type in condition: Void");
		    }
		    break;
		}
		case Ret:
		/* Return type should be the same as function declaration */
		{
			TreeNode *returnExpr = t->child[0];
			BucketList funcRec = st_lookup_list(sc_top(), analyzingFuncName);

			// find the funcDeclNode in funcRec
			TreeNode *idDeclNode = nullptr;
			for (int i = 0; i < funcRec.size(); i++) {
				if (funcRec[i].id == analyzingFuncName) {
					idDeclNode = funcRec[i].node;
				}
			}

			if (idDeclNode == nullptr) { /* not likely to be called if buildTable works correctly */
				cout << "[Analyze.checkNode] Error: Function identifier cannot find in the symbol table" << endl;
				break;
			}
			if (idDeclNode->child[0]->nodekind == Int && (returnExpr == NULL || returnExpr->nodekind == Void)) {
				typeError(t, "Int return value expected");
			}
			else if (idDeclNode->child[0]->nodekind == Void &&(returnExpr->nodekind == Int)) {
				typeError(t, "no return value expected");
			}
			break;
        }
		case Assign:
		{
			TreeNode * idNode = t->child[0];
			TreeNode *exprNode = t->child[1];
			string idName = idNode->attr.name;
			// fix: error(int a; a[11] = ;) error(int a[11]; a = ;)
			// need to look up is_array in the Symbol table
            BucketList idRec = st_lookup_list(sc_top(), idName);
            TreeNode *idDeclNode = nullptr;
            for (int i = 0; i < idRec.size(); i++) {
                if (idRec[i].id == idName) {
                    idDeclNode = idRec[i].node;
                }
            }
            if(idDeclNode == NULL) {
                typeError(t->child[0], "the variable has not been declared");
                break;
            }
			if (exprNode->nodekind == Void) {
				/* some callExpr may return void */
				typeError(t->child[1], "assignment with invalid type: void");
			}
			else if (idNode->nodekind == Id && idDeclNode->nodekind == Arr_dec) {
				 typeError(t->child[0], "assignment to an array variable");
				// has done in Id
			}
			else if(idNode->nodekind == Arry_elem && idDeclNode->nodekind == Var_dec){
			    typeError(t->child[0], "index of a non-array variable is invalid");
			}
			break;
		}

		case Call: // CallExprNode - params(nullptr/expr-expr...)
		{
			TreeNode * callDeclNode = nullptr;
			TreeNode *callNode = t;
			string funcName = callNode->child[0]->attr.name;

			BucketList l = st_lookup_list(sc_top(), funcName);
			for (int i = 0; i < l.size(); i++) {
				if (l[i].id == funcName && l[i].node->nodekind == Fun) {
					callDeclNode = l[i].node;
				}
			}
			if (callDeclNode == nullptr) {
				cout << "[Analyze.checkNode] Error: Function call's identifier cannot find in the symbol table" << endl;
				break;
			}
			//callNode->nodekind = callDeclNode->nodekind;

			/* check argments type */
			TreeNode *args = t->child[1];
			TreeNode *params = (callDeclNode->child[2]->child[0]);
			int paramNum = 0;
			/* calculate the number of parameters in function declaration*/
			while (params != NULL) {
				if (params->child[0]->nodekind == Int) {
					paramNum++;
					params = params->sibling;
				}
				else
					break;
			}
			
			TreeNode *param = callDeclNode->child[2];
			/* check the parameter type */
			for (int i = 0; i < paramNum; i++) {
				if (args == NULL) {
					typeError(t, "the number of parameters is inconsistent with declaration");
				}
				else if (args->child[0]->nodekind == Void) {
					typeError(args, "invalid argument type: void");
				}
				else {
					args = args->sibling;
					param = param->sibling;
				}
			}
			if (args != NULL) {
				typeError(args, "the number of paramters is inconsistent with declaration");
			}

			break;
		}
		case Op:
		{
			/* two operands are ExprNode */
			TreeNode * op1, *op2, *exprT;
			op1 = t->child[0];
			op2 = t->child[1];
			exprT = t;

			if (op1->nodekind == Void || op2->nodekind == Void) {
				typeError(t->child[0], "operand's type is invalid: void");
			}
			else {
				exprT->nodekind = Int;
			}
			break;
		}

		case Id: /* id | id['expr'] */
		{
			TreeNode * idExpr = t;
			BucketList idRec = st_lookup_list(sc_top(), idExpr->attr.name);

			// find the treeNode stored in idRec(DeclNode of the identifier)
			TreeNode *idDeclNode = nullptr;
			for (int i = 0; i < idRec.size(); i++) {
				if (idRec[i].id == idExpr->attr.name) {
					idDeclNode = idRec[i].node;
				}
			}

			if (idDeclNode == nullptr) {
				// cout << t->lineno << endl;
				// cout << "[Analyze.checkNode] Error: Id cannot find in the symbol table" << endl;
				break;
			}
			// if (idDeclNode->is_array) {
			//     if(t->children.size() == 0){
			//         // typeError(t, "assignment to an array variable");
			//         break;
			//     }
			// 	ExprNode *indexNode = dynamic_cast<ExprNode*>(t->children[0]);
			// 	if (indexNode->kind != Int) {
			// 		typeError(indexNode, "the index of array should be Int type");
			// 	}
			// 	else {
			// 		idExpr->kind = Int;
			// 	}
			// }
			// else {
			// 	idExpr->kind = idDeclNode->kind; /* identifier's kind == declartionNode's kind*/
			// }
			break;
		}



	    /* do not need to checkNode DeclNode */
	    default:
		    // not likely happen unless parsing not correctly
		    //cout <<"["<<t->nodekind << "]:[Analyze.checkNode] Error: No explicit type of the node." << endl;
		    break;
	}
}


/* pop out the scope saved in stack in buildTable process */
static void popScope(TreeNode *t){
	if (t->nodekind == Comp) {
		sc_pop();
	}
}


/* preorder scope maintain to checkNode in postorder traverse */
static void pushScope(TreeNode *t) {
	/* save function attr.name in Decl / push scope in Compound scope */
	if (t->nodekind == Fun) {
		TreeNode *funNode = t;
		analyzingFuncName = funNode->child[1]->attr.name;
		sc_push(funNode->scope);
	}
	else if (t->nodekind == Comp && t->scope != NULL) {
		sc_push(t->scope);
	}
}


/* invoked as preorder traverse: traverse(syntaxTree, insertNode, popScope) */
void buildTable(TreeNode *syntaxTree) {
	/* initialize: create global scope & input, output */
	initSymTab();
	syntaxTree->scope = sc_top(); /* program -> scope = global*/
	traverse(syntaxTree, insertNode, popScope);
	sc_pop();

	/* TraceAnalyze = False -=> only output type error 
	   TraceAnalyze = True -=> type error & symbol table */

	if (TraceAnalyze) {
		fprintf(listing, "[TraceAnalyze]: Symbol table created in analysis process: \n");
		printSymTab(listing);
		fclose(listing);
	}
	return;
}


/* invoked as postorder traverse: traverse(syntaxTree, pushScope, checkNode) */
void typeCheck(TreeNode * syntaxTree)
{
	sc_push(global);
	traverse(syntaxTree,pushScope ,checkNode);
	sc_pop();
}
int main() {
	InitCode("code.txt");
	listing = fopen("listing.txt","w");
	Parser P("test.txt");
	buildTable(P.synTree);
	typeCheck(P.synTree);
}