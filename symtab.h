#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include <string>
#include <vector>
#include "parser.h"
using namespace std;

#define HASH_TABLE_SIZE 3533 // size of hash table, just a prime

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
typedef ScopeRec* Scope;

Scope sc_create(string scopeName);
// basic operations on scope stack
Scope sc_top();
void sc_push(Scope scope);
void sc_pop();

/* Procedure st_insert inserts line numbers and
 * memory locations into the symbol table
 * loc = memory location is inserted only the
 * first time, otherwise ignored
 * In C--, pointer to Node AST added
 */
void st_insert(string id, int lineno, int size, TreeNode *node);
void st_insert_nearest(string id, int lineno, int loc, TreeNode *node);

/* Function st_lookup returns the memory
* location of a variable or -1 if not found
*/
int st_lookup(Scope s, string id);
/* _nonest will only search in Scope s whether s is nested in another scope or not*/
int st_lookup_nonest(Scope s, string id);
/* _list will return the BucketList found in nearest symbolTable */
BucketList st_lookup_list(Scope s, string id);

/* Procedure printSymTab prints a formatted
* listing of the symbol table contents
* to the listing file
*/
void printIdentifier(FILE * listing, BucketList *hashTable);
void printSymTab(FILE * listing);


#endif