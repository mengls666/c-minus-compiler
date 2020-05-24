#ifndef _SYMTAB_H_
#define _SYMTAB_H_

#include <string>
#include <vector>
#include "parser.h"
using namespace std;




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