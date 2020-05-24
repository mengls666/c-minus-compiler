#ifndef _ANALYZE_H_
#define _ANALYZE_H_
#include "code.h"
#include "symtab.h"
using namespace std;
/* Function buildSymtab constructs the symbol
* table by preorder traversal of the syntax tree
*/
void buildTable(TreeNode *syntaxTree);

/* Procedure typeCheck performs type checking
* by a postorder syntax tree traversal
*/
void typeCheck(TreeNode *syntaxTree); // calculate type and assign to node + typecheck record to listing file

#endif