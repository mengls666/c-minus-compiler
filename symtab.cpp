/* symbol table in semantic analysis */

#include <iostream>
#include <cstdio>
#include <vector>
#include <string>
#include "symtab.h"

using namespace std;

/* SHIFT is the power of two used as multiplier
in hash function  */
#define SHIFT 4

#define MAX_SCOPE_NUM 1024

#define NOT_FOUND -1

/* maintaining global scope stack */
static Scope scopes[MAX_SCOPE_NUM];
static int nScope = 0;
static Scope scopeStack[MAX_SCOPE_NUM];
static int nScopeStack = 0;
static int locAlloc[MAX_SCOPE_NUM];

Scope global; // global scope: derive out other scopes

/* the hash function:
* h = (a^(n-1)c1 + ... + cn) mod size
*/
static int hash_func(string key) {
	int h = 0;

	for (int i = 0; i < key.length(); i++) {
		h = ((h << SHIFT) + key[i]) % HASH_TABLE_SIZE; // SHIFT is much efficient
	}

	return h;
}

Scope sc_create(string scopeName)
{
	Scope s = new ScopeRec(scopeName);

	s->nestedLevel = nScopeStack;
	s->parentScope = sc_top();

	scopes[nScope++] = s;

	return s;
}

Scope sc_top()
{
	return scopeStack[nScopeStack - 1];
}

void sc_push(Scope scope)
{
	scopeStack[nScopeStack++] = scope;
	/* fix: mem allocation considering codegen */
	if(scope->parentScope == NULL || scope->parentScope->scopeName == "global")
	    locAlloc[nScopeStack-1] = 0;
	else
	    locAlloc[nScopeStack - 1] = locAlloc[nScopeStack - 2];
}

void sc_pop()
{
	nScopeStack--;
}

void st_insert(string id, int lineno, int size, TreeNode *node) {
	int hashVal = hash_func(id);
	Scope top = sc_top();
	BucketList *listPointer = &top->hashTable[hashVal];

	int idx = -1;
	for (int i = 0; i < listPointer->size(); i++) {
		if (id == listPointer->at(i).id) {
			idx = i;
			break;
		}
	}

	if (idx == -1) { /* variable not yet in table */
		BucketListRec r = BucketListRec(id, node, locAlloc[nScopeStack-1]);
		locAlloc[nScopeStack - 1] += size;
		r.lines.push_back(lineno);
		listPointer->push_back(r);
	}
	else { /* found in table, so just add line number*/
		listPointer->at(idx).lines.push_back(lineno);
	}
}

/* Invoked in buildTable -> analyzing IdExprNode */
void st_insert_nearest(string id, int lineno, int loc, TreeNode *node) {
	int hashVal = hash_func(id);
	Scope top = sc_top();
	BucketList *listPointer = &top->hashTable[hashVal];

	while (top) {
		listPointer = &top->hashTable[hashVal];
		for (int i = 0; i < listPointer->size(); i++) {
			// nearest principle of scope
			if (id == listPointer->at(i).id) {
				listPointer->at(i).lines.push_back(lineno);
				return;
			}
		}
		top = top->parentScope;
	}
}

/* Function st_lookup returns the memory
* location of a variable or -1 if not found
*/
int st_lookup(Scope s, string id)
{
	int hashVal = hash_func(id); //O(1) search time
	
	while (s) {
		BucketList list = s->hashTable[hashVal];
		for (int i = 0; i < list.size(); i++) {
			// nearest principle of scope
			if (id == list[i].id) {
				return list[i].memloc;
			}
		}
		s = s->parentScope;
	}
	return NOT_FOUND; // -1 as error code
}

// just lookup in the current scope
int st_lookup_nonest(Scope s, string id)
{
	int hashVal = hash_func(id); //O(1) search time
	BucketList list = s->hashTable[hashVal];

	for (int i = 0; i < list.size(); i++) {
		if (id == list[i].id) {
			return list[i].memloc;
		}
	}
	return NOT_FOUND; // -1 as error code
}

/* _list will return the BucketList found in nearest symbolTable */
BucketList st_lookup_list(Scope s, string id)
{
	int hashVal = hash_func(id);
	BucketList list;
	while (s) {
		list = s->hashTable[hashVal];
		for (int i = 0; i < list.size(); i++) {
			if (id == list[i].id)
				return list;
		}
		s = s->parentScope; // nearest principle of scope
	}
	return list;
}

/* invoked in printSymTab, presenting the information of one identifier */
// format: [identifier]  [DeclType]  [TypeKind] [memloc] [Lineno(s)]
// e.g.		   main		  Function	   void	        0         8
//              a           Var        Int          1      7, 11, 20
void printIdentifier(FILE * listing, BucketList *hTable)
{
	TreeNode *varNode;
	TreeNode *funNode;
	TreeNode *node;
	ExpType dataType;
	BucketList l;

	for (int idx = 0; idx < HASH_TABLE_SIZE; idx++) {
		if (hTable[idx].size() == 0)
			continue;
		l = hTable[idx];
		for (int i = 0; i < l.size(); i++) {
			node = l[i].node;
			fprintf(listing, "%-12s", l[i].id.c_str());
			if (node->nodekind == Var_dec) {
				varNode = node;
				if (varNode->child[1]->nodekind == Arr_dec) {
					fprintf(listing, "Array       ");
					if(varNode->child[0]->nodekind == Int) {
                        dataType = Exp_INT;
                    } else if(varNode->child[0]->nodekind == Void) {
                        dataType = Exp_VOID;
                    }
				}
				else {
					fprintf(listing, "Variable    ");
					if(varNode->child[0]->nodekind == Int) {
                        dataType = Exp_INT;
                    } else if(varNode->child[0]->nodekind == Void) {
                        dataType = Exp_VOID;
                    }
				}
			}
			else if (node->nodekind == Fun) {
				funNode = node;
				fprintf(listing, "Function    ");
				if(varNode->child[0]->nodekind == Int) {
                    dataType = Exp_INT;
                } else if(varNode->child[0]->nodekind == Void) {
                    dataType = Exp_VOID;
                }
			}

			switch (dataType)
			{
			case Void:
				fprintf(listing, "void    ");
				break;
			case Int:
				fprintf(listing, "int     ");
			default:
				break;
			}

			fprintf(listing, "%4d      ", l[i].memloc);

			for (auto no : l[i].lines) {
				fprintf(listing, "%4d ", no);
			}
			fprintf(listing, "\n");
		}
	}
	return;
}

/* Procedure printSymTab prints a formatted
* listing of the symbol table contents
* to the listing file
*/
void printSymTab(FILE * listing) {
	/* print out all scopes sequentially */
	for (int i = 0; i < nScope; i++) {
		Scope scope = scopes[i];
		BucketList *hashTable = scope->hashTable;

		// print scope name: global/function name
		fprintf(listing, "Scope Name: %s ", scope->scopeName.c_str());

		// print nested level(global:0, main:1)
		fprintf(listing, "<nested depth: %d>\n", scope->nestedLevel);
		
		fprintf(listing, "Name       | Type   | dataType  |  memloc  | LineNOs \n");
		fprintf(listing, "=====================================================\n");
		
		printIdentifier(listing, hashTable);
		fprintf(listing, "\n");

	}
	return; 
}
int main() {
    Parser P("test.txt");
    FILE * symtab = fopen("symtab.txt","rw");
    printSymTab(symtab);
}