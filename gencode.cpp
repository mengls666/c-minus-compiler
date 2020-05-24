#include "gencode.h"
#include "code.h"
char buffer[1024];
const int ofpFO = 0;
const int retFO = -1;
const int  initFO = -2;
extern Scope global;
extern bool TraceCode;
// this serves as a stack pointer maintained by the compiler
static int frame_offset = initFO;



static int param_offset(TreeNode * param_list) {
    int offset = 0;
    for (TreeNode * node = param_list; node != nullptr; node = node->sibling) {
        offset += 1;
    }

    return offset;
}

static void gen_input(track & track) {
    // copied from gen_fun
    track.add_func("input", emitSkip(0));

    emitRM("ST", ac, retFO, fp, "save return address in fp + retFO");
    emitRO("IN", ac, 0, 0, "read input");
    emitRM("LD", pc, retFO, fp, "return to caller");
}

static void gen_output(track & track) {
    // copied from gen_fun
    track.add_func("output", emitSkip(0));

    emitRM("ST", ac, retFO, fp, "save return address in fp + retFO");
    // save parameter to a register
    emitRM("LD", ac, initFO, fp, "get x for output");
    emitRO("OUT", ac, 0, 0, "output x");
    emitRM("LD", pc, retFO, fp, "return to caller");
}

fun::fun(string fun_name, int index) {
    this->fun_name = fun_name;
    this->index = index;
}

void track::add_func(string id, int index) {
    fun f(id, index);
    funs.push_back(f);
}

int track::find_func(string id) {
    int index = NOT_FOUND;
    for (int i = 0; i < funs.size(); i++) {
        if (funs[i].fun_name == id) {
            index = funs[i].index;
            break;
        }
    }
    return index;
}

/* gentiny is the interface to be used*/
void gentiny(TreeNode * tree) {
    track t;

    emitComment("C-- Compilation to TM code");
    emitComment("Standard prelude:");
    emitRM("LD", mp, 0, ac, "load maxaddress from location 0");
    emitRM("ST", ac, 0, ac, "clear loation 0");
    // skip three command. These are:
    // set fp
    // set return address to halt
    // jump to main
    int saved_loc = emitSkip(3);
    emitComment("End of standard prelude:");
    emitComment("Begin generating built-ins");
    emitComment("Generate built-in input");
    gen_input(t);
    emitComment("Generate built-in output");
    gen_output(t);
    emitComment("End generating built-in");

    // we assume that the global area is the first frame. Each declaration occupies one slot
    int init_fp = 0;
    for (TreeNode * node = tree; node != nullptr; node = node->sibling) {
        gentiny_code(node, t);
        init_fp--;
    }
    emitComment("End of execution");

    // back patch
    int halt_loc = emitSkip(0);
    emitBackup(saved_loc);
    // set frame pointer
    emitRM("LDA", fp, init_fp, mp, "load initial frame pointer");
    // set return address to halt
    emitRM_Abs("LDA", ac, halt_loc, "set return address to halt");
    // perform jump
    emitRM_Abs("JEQ", gp, t.find_func("main"), "jump to main");
    // go back to produce halt
    emitRestore();
    emitRO("HALT", 0, 0, 0, "");
}


void gentiny_code(TreeNode * node, track & track) {
    Nodekind type = node->nodekind;
    switch(type) {
        default:
            gentiny_stmt(node, track);
            break;
        case Fun:
        case Var_dec:
        case Arr_dec:
            gentiny_decl(node, track);
            break;
    }
}

void gentiny_stmt(TreeNode * node, track & track) {
    Nodekind type = node->nodekind;
    switch(type) {
        default:
            gentiny_expr(node, track);
            break;
        case Selec:
            gentiny_stmt_if(node, track);
            break;
        case Iter:
            gentiny_stmt_iter(node, track);
            break;
        case Ret:
            gentiny_stmt_return(node, track);
            break;
        case Comp:
            gentiny_stmt_compound(node, track);
            break;
    }
}


void gentiny_stmt_if(TreeNode * node, track & track) {
    if(TraceCode)
        emitComment("--> If");
    TreeNode * cond = node->child[0];
    gentiny_expr((TreeNode*)cond, track);

    int loc1 = emitSkip(1);
    TreeNode * stmt = node->child[1];
    gentiny_stmt((TreeNode*)stmt, track);

    if(node->child[2] != NULL) {
        int loc2 = emitSkip(1);
        int loc3 = emitSkip(0);
        stmt = node->child[2];
        gentiny_stmt(stmt, track);

        int current_loc = emitSkip(0);
        emitBackup(loc1);
        emitRM_Abs("JEQ", ac, loc3, "If false, jump to else part");
        emitRestore();

        emitBackup(loc2);
        emitRM_Abs("LDA", pc, current_loc, "Jump to the end");
        emitRestore();
    } else {
        int current_loc = emitSkip(0);
        emitBackup(loc1);
        emitRM_Abs("JEQ", ac, current_loc, "If false, jump to the end");
        emitRestore();
    }
    if(TraceCode)
        emitComment("<-- If");
}

void gentiny_stmt_iter(TreeNode * node, track & track) {
    if(TraceCode)
        emitComment("--> While");
    TreeNode * cond = node->child[0];

    int loc1 = emitSkip(0);
    gentiny_expr(cond, track);

    int loc2 = emitSkip(1);
    TreeNode * stmt = node->child[1];
    gentiny_stmt(stmt, track);
    emitRM_Abs("LDA", pc, loc1, "Jump to while condition");

    int current_loc = emitSkip(0);
    emitBackup(loc2);
    emitRM_Abs("JEQ", ac, current_loc, "If false, end loop");
    emitRestore();
    if(TraceCode)
        emitComment("<-- While");
}

void gentiny_stmt_return(TreeNode * node, track & track) {
    if(TraceCode)
        emitComment("--> Return");
    TreeNode * expr = node->child[0];
    if(expr != NULL)    
        gentiny_expr(expr, track);
    if(TraceCode)
        emitComment("<-- Return");
}

void gentiny_stmt_compound(TreeNode * node, track & track) {

    if (node->scope != nullptr) {
        // for function scope, this can be null
        track.current_scope = node->scope;
    }

    for (TreeNode * decl = node->child[0]; decl != nullptr; decl = decl->sibling) {
        gentiny_code(decl, track);
    }

    for (TreeNode * stmt = node->child[1]; stmt != nullptr; stmt = stmt->sibling) {
        gentiny_code(stmt, track);
    }

    if (node->scope != nullptr) {
        // restore scope
        track.current_scope = node->scope->parentScope;
    }
}

void gentiny_expr(TreeNode * node, track & track, bool isAddress) {
    Nodekind type = node->nodekind;
    switch(type) {
        case Assign:
            gentiny_expr_assign(node, track);
            break;
        case Call:
            gentiny_expr_call(node, track);
            break;
        case Op:
            gentiny_expr_op(node, track);
            break;
        case Const:
            gentiny_expr_const(node, track);
            break;
        case Id:
            gentiny_expr_id(node, track, isAddress);
            break;
    }
}

void gentiny_expr_assign(TreeNode * node, track & track) {
    if(TraceCode)
        emitComment("--> Assign");
    TreeNode *id = node->child[0];
    TreeNode *expr = node->child[1];

    // lhs - address, rhs - value
    gentiny_expr(id, track, true);
    emitRM("ST", ac, --frame_offset, fp, "assign: push id(addr) to fp");

    gentiny_expr(expr, track, false);
    emitRM("LD", ac1, frame_offset++, fp, "assign: load id(addr) to ac1");

    // assignment in tiny(ac: expr, ac1: id)
    emitRM("ST", ac, 0, ac1, "assign: assign expr(ac)->id(ac1)");

    if(TraceCode)
        emitComment("<-- Assign");
    return ;
}

void gentiny_expr_call(TreeNode * node, track & track) {
    if (TraceCode) {
        sprintf(buffer, "--> Calling (%s)", node->child[0]->attr.name);
        emitComment(buffer);
    }

    // the slot is for old frame pointer. fp + frame_offset will be the new frame pointer
    frame_offset--;

    // frame offset is the current stack pointer
    // compute arguments
    int param_offset = 0;
    for (TreeNode * param = node->child[1]; param != nullptr; param = param->sibling) {
        // param is a exp node
        gentiny_code(param, track);
        // after this, result will be stored in ac. Store this
        sprintf(buffer, "store parameter %d", param_offset);
        emitRM("ST", ac, frame_offset + initFO - param_offset, fp, buffer);
        param_offset += 1;
    }

    // store old frame pointer
    emitRM("ST", fp, frame_offset + ofpFO, fp, "store old frame pointer");
    // push new frame
    emitRM("LDA", fp, frame_offset, fp, "push new frame");
    // save return in ac
    emitRM("LDA", ac, 1, pc, "save return address in ac");
    // jump. This uses the global address of the function
    emitRM_Abs("LDA", pc, track.find_func(node->child[0]->attr.name), "jump to the function");

    // after call done, pop current frame. At this point, fp is the new fp
    emitRM("LD", fp, ofpFO, fp, "restore frame pointer");

    // because no real space is used, we change this back
    frame_offset++;
    if (TraceCode) {
        sprintf(buffer, "<-- Calling (%s)", node->child[0]->attr.name);
        emitComment(buffer);
    }
}

void gentiny_expr_op(TreeNode * node, track & track) {
    TreeNode *op1, *op2;
    TreeNode *op1_c, *op2_c;
    bool const_folding = false; // code opt
    op1 = node->child[0];
    op2 = node->child[1];
    if(TraceCode)
        emitComment("--> Op");
    if(op1->nodekind == Const && op2->nodekind == Const){
        const_folding = true;
        op1_c = op1;
        op2_c = op2;
    }
    if(!const_folding){
        // push result value op1 to mp stack: ac->temp[mp--]
        gentiny_expr(op1, track, false);
        emitRM("ST", ac, --frame_offset, fp, "op: push operand1 to mp");

        gentiny_expr(op2, track, false);
        emitRM("LD", ac1, frame_offset++, fp, "op: load operand1 from mp");
    }

    // ac: op2 ac1: op1 -> save in ac
    switch(node->attr.op){
        case PLUS:{
            if(const_folding)
                emitRM("LDC", ac, op1_c->attr.val + op2_c->attr.val, 0, "op: op1 + op2");
            else emitRO("ADD", ac, ac1, ac, "op: Plus");
            break;
        }
        case MINUS:{
            if(const_folding)
                emitRM("LDC", ac, op1_c->attr.val - op2_c->attr.val, 0, "op: op1 - op2");
            else emitRO("SUB", ac, ac1, ac, "op: Minus");
            break;
        }
        case MUL:{
            if(const_folding)
                emitRM("LDC", ac, op1_c->attr.val * op2_c->attr.val, 0, "op: op1 * op2");
            else emitRO("MUL", ac, ac1, ac, "op: Times");
            break;
        }
        case DIV:{
            if(const_folding)
                emitRM("LDC", ac, op1_c->attr.val / op2_c->attr.val, 0, "op: op1 / op2");
            else emitRO("DIV", ac, ac1, ac, "op: Divide");
            break;
        }
        case GEQ:{
            if(const_folding)
                emitRM("LDC", ac, op1_c->attr.val >= op2_c->attr.val ? 1 : 0, 0, "op: op1 >= op2");
            else {
                emitRO("SUB", ac, ac1, ac, "op: >=");
                emitRM("JGE", ac, 2, pc, "if >=, pc = pc + 2");
                emitRM("LDC", ac, 0, ac, "if false, ac = 0");
                emitRM("LDA", pc, 1, pc, "if false, skip next");
                emitRM("LDC", ac, 1, ac, "if true, ac = 1");
            }
            break;
        }
        case LEQ:{
            if(const_folding)
                emitRM("LDC", ac, op1_c->attr.val <= op2_c->attr.val ? 1 : 0, 0, "op: op1 <= op2");
            else {
                emitRO("SUB", ac, ac1, ac, "op: <=");
                emitRM("JLE", ac, 2, pc, "if <=, pc = pc + 2");
                emitRM("LDC", ac, 0, ac, "if false, ac = 0");
                emitRM("LDA", pc, 1, pc, "if false, skip next");
                emitRM("LDC", ac, 1, ac, "if true, ac = 1");
            }
            break;
        }
        case GT:{
            if(const_folding)
                emitRM("LDC", ac, op1_c->attr.val > op2_c->attr.val ? 1 : 0, 0, "op: op1 > op2");
            else {
                emitRO("SUB", ac, ac1, ac, "op: >");
                emitRM("JGT", ac, 2, pc, "if >, pc = pc + 2");
                emitRM("LDC", ac, 0, ac, "if false, ac = 0");
                emitRM("LDA", pc, 1, pc, "if false, skip next");
                emitRM("LDC", ac, 1, ac, "if true, ac = 1");
            }
            break;
        }
        case LT:{
            if(const_folding)
                emitRM("LDC", ac, op1_c->attr.val < op2_c->attr.val ? 1 : 0, 0, "op: op1 < op2");
            else {
                emitRO("SUB", ac, ac1, ac, "op: <");
                emitRM("JLT", ac, 2, pc, "if <, pc = pc + 2");
                emitRM("LDC", ac, 0, ac, "if false, ac = 0");
                emitRM("LDA", pc, 1, pc, "if false, skip next");
                emitRM("LDC", ac, 1, ac, "if true, ac = 1");
            }
            break;
        }
        case EQ:{
            if(const_folding)
                emitRM("LDC", ac, op1_c->attr.val == op2_c->attr.val ? 1 : 0, 0, "op: op1 == op2");
            else {
                emitRO("SUB", ac, ac1, ac, "op: ==");
                emitRM("JEQ", ac, 2, pc, "if ==, pc = pc + 2");
                emitRM("LDC", ac, 0, ac, "if false, ac = 0");
                emitRM("LDA", pc, 1, pc, "if false, skip next");
                emitRM("LDC", ac, 1, ac, "if true, ac = 1");
            }
            break;
        }
        case NEQ:{
            if(const_folding)
                emitRM("LDC", ac, op1_c->attr.val != op2_c->attr.val ? 1 : 0, 0, "op: op1 != op2");
            else {
                emitRO("SUB", ac, ac1, ac, "op: !=");
                emitRM("JNE", ac, 2, pc, "if !=, pc = pc + 2");
                emitRM("LDC", ac, 0, ac, "if false, ac = 0");
                emitRM("LDA", pc, 1, pc, "if false, skip next");
                emitRM("LDC", ac, 1, ac, "if true, ac = 1");
            }
            break;
        }
        default:
            emitComment("[gentiny.op.error]: Invalid operator type.");
            break;
    }
    if(TraceCode)
        emitComment("<-- Op");
    return;
}

void gentiny_expr_const(TreeNode * node, track & track) {
    if(TraceCode)
        emitComment("--> Const");

    // load const-value into register ac
    int constVal = node->attr.val;
    emitRM("LDC", ac, constVal, 0, "Const: load val to ac");

    if(TraceCode)
        emitComment("<-- Const");
    return ;
}

/* isAddress decides to calculate address or value(for array-type id) */
void gentiny_expr_id(TreeNode * node, track & track, bool isAddress) {
    if(TraceCode)
        emitComment("--> ExprId");

    Scope s = track.current_scope;
    int mem_loc = st_lookup(s, node->attr.name);
    int idOffset = initFO - mem_loc; // offset in the frame

    emitRM("LDC", ac, idOffset, 0, "ExprId: load id offset to ac");
    emitRO("ADD", ac, fp, ac, "ExprId: fp + offset = base address");


    BucketList list = st_lookup_list(s, node->attr.name);
    TreeNode * var_node = nullptr;
    for (const auto & rec : list) {
        if (rec.id == node->attr.name) {
            var_node = rec.node;
        }
    }
    // this is for the case in which x is a array parameter
    if (var_node->nodekind == Arr_dec || node->nodekind == Arry_elem) {
        // load absolute address
        emitRM("LD", ac, 0, ac, "load absolute address");
    }
    // may need to execute the indexExpr if it's an array
    if(node->nodekind == Arry_elem){
//         if(mem_loc >= 0 && mem_loc < param_offset())

        // a local array variable: ac-base address
        TreeNode *index = node->child[1];
        emitRM("ST", ac, --frame_offset, fp, "ExprId: push base address to mp");
        gentiny_code(index, track);

        emitRM("LD", ac1, frame_offset++, fp, "ExprId: load base address to ac1");
        emitRO("SUB", ac, ac1, ac, "ExprId: base address - index = index address");
    }

    // isAddress - lhs like a[11], need the address of a[11]
    // the weird case is for cases like x[10], output(x)
    if(isAddress or (var_node->nodekind == Arr_dec and not node->nodekind == Arry_elem)){
        emitRM("LDA", ac, 0, ac, "ExprId: load id address");
    }else{
        emitRM("LD", ac, 0, ac, "ExprId: load id value");
    }

    if(TraceCode)
        emitComment("<-- ExprId");
    return ;
}

void gentiny_decl(TreeNode * node, track & track) {
    Nodekind type = node->nodekind;
    switch(type) {
        case Var_dec:
        case Arr_dec:
            gentiny_decl_var(node, track);
            break;
        case Fun:
            gentiny_decl_fun(node, track);
            break;
    }
}


void gentiny_decl_var(TreeNode * node, track & track) {
    if(TraceCode)
        emitComment("--> DeclVar");

    // do nothing more than allocate space in current AR
    // this does not harm if this is a global declaration,
    // though there are more consistent ways to do it
    if (node->child[1]->nodekind==Arr_dec) {
        frame_offset -= node->child[1]->child[1]->attr.val;
    }
    else
        frame_offset -= 1;


    if(TraceCode)
        emitComment("<-- DeclVar");
    return ;
}

void gentiny_decl_fun(TreeNode * node, track & track) {
    /*
     * We now have
     * - parameters saved in fp + initFO
     * - return address in ac (not saved in fp + retFO)
     * - ofp in fp + ofpFO
     */

    if (TraceCode) {
        sprintf(buffer, "--> Function (%s)", node->child[1]->attr.name);
        emitComment(buffer);
    }

    // record function location
    track.add_func(node->child[1]->attr.name, emitSkip(0));

    emitRM("ST", ac, retFO, fp, "save return address in fp + retFO");

    // generate code for the body
    // some setups
    track.current_scope = node->scope;
    // modify stack pointer
    if(node->child[2]->child[0]->nodekind != Void)
        frame_offset = initFO - param_offset(node->child[2]->child[0]);

    if (TraceCode)
        emitComment("-> Begin function body");
    // actually generate the code
    TreeNode * body = node->child[3];
    gentiny_code(body, track);
    if (TraceCode)
        emitComment("<- End function body");


    emitRM("LD", pc, retFO, fp, "return to caller");

    if (TraceCode) {
        sprintf(buffer, "<-- Function (%s)", node->child[1]->attr.name);
        emitComment(buffer);
    }
}