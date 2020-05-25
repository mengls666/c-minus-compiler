#include "parser.h"
#include <iostream>

using namespace std;
ofstream fout_Tree("Token_tree.txt");
Parser::Parser(string filename) {
    step = 0;
    tokenIndex = 0;
    isError = false;
    scanner.getSourse(filename);
    scanner.delComments();
    if(scanner.isSuccess) {
        scanner.scan();
        cout << "///begin parsing....///" << endl;
        synTree = parse();
    } if(isError) {
        cout << "Error:Failed to parse because there are exceptions when parsing" << endl;
    } else {
        cout << "///parser runs successful///" << endl;
    }
    printTree(synTree);
    fout_Tree.close();
}
Token Parser::getToken() {
    lastToken = currentToken;
    currentToken = scanner.getToken(tokenIndex++);
    return currentToken;
}
void Parser::genError(string s) {
    cout << s << "--> Syntax error at line " << lastToken.lineNo <<
    "   Token:" << lastToken.tokenString<< "   tokentype:" << lastToken.tokenType << endl;
    isError = true;
}
void Parser::match(TokenType ex) {
    if(currentToken.tokenType==ex) {
        getToken();
    } else {
        genError("An error occured when match tokentype " + ex);
    }
}
TreeNode * Parser::newNode(Nodekind kind) {
    TreeNode * p = (TreeNode * )malloc(sizeof(TreeNode));
    int k;
    if(p==NULL) {
        cout << "can't alloc enough memory!" << endl;
    } else {
        for(k=0;k<MAX_child;k++) {
            p->child[k]=NULL;
        }
        p->sibling = NULL;
        p->nodekind = kind;
        p->lineno = currentToken.lineNo;
        if(kind==Op || kind == Int || kind == Id) {
            p->type = Exp_INT;
        } 
        if(kind==Id) {
            sprintf(p->attr.name,"");
        }
        if(kind==Const) {
            p->attr.val = 0;
        }
    }
    return p;
}
TreeNode * Parser::parse(void) {
    TreeNode * t;
    currentToken = getToken();
    lastToken = currentToken;
    t = declaration_list();
    if(currentToken.tokenType != ENDF) {
        genError("end failed");
    }
    return t;
}
TreeNode * Parser::declaration_list() {
    TreeNode * t = declaration();
    TreeNode *p = t;
    //pass the tokens before void and int
    while(currentToken.tokenType!=INT && currentToken.tokenType != VOID && currentToken.tokenType != ENDF ) {
        genError("10");
        getToken();
        if(currentToken.tokenType==ENDF) {
            break;
        }
    }
    while(currentToken.tokenType==INT || currentToken.tokenType==VOID) {
        TreeNode * q;
        q = declaration();
        if(q != NULL) {
            if(t==NULL) {
                t = p = q;
            } else {
                p->sibling = q;
                p = q;
            }
        }
    }
    match(ENDF);
    return t;
}
TreeNode *Parser::declaration(void) {
    TreeNode * t = NULL;
    TreeNode * p = NULL;
    TreeNode * q = NULL;
    TreeNode * s = NULL;
    if(currentToken.tokenType==INT) {
        p = newNode(Int);
        match(INT);
    } else if(currentToken.tokenType==VOID) {
        p = newNode(Void);
        match(VOID);
    } else {
        genError("tokentype match failed");
    }
    if(p!=NULL && currentToken.tokenType == ID) {
        q = newNode(Id);
        sprintf(q->attr.name,"%s",currentToken.tokenString.c_str());
        match(ID);
        if(currentToken.tokenType==LS) {// function declaration 
            t = newNode(Fun);
            t->child[0]=p; //return type
            t->child[1]=q; // id
            match(LS);
            t->child[2]=params(); // params
            match(RS);
            t->child[3]=compound(); // body
        } else if(currentToken.tokenType==LM) {// array declaration
            t=newNode(Var_dec);
            TreeNode *m = newNode(Arr_dec);
            match(LM);
            match(NUM);
            s = newNode(Const);
            s->attr.val = atoi(lastToken.tokenString.c_str());
            m->child[0]=q;//id
            m->child[1]=s;//size
            t->child[0]=p;//type
            t->child[1]=m;//kind->array
            match(RM);
            match(SEMI);
        } else if(currentToken.tokenType==SEMI) {//normal var
            t = newNode(Var_dec);
            t->child[0]=p; //type
            t->child[1]=q; //id
            match(SEMI);
        } else {
            genError("1");
        }
    } else {
        genError("2");
    }
    return t;
}
TreeNode * Parser::params(void) {
    TreeNode *t = newNode(Params);
    TreeNode *p = NULL;
    if(currentToken.tokenType==VOID) {
        p = newNode(Void);
        match(VOID);
        if(currentToken.tokenType==RS) {// params:void
            if(t!=NULL) {
                t->child[0]=p;
            }
        } else {//params:void id, ...
            t->child[0]=params_list(p);
        }
    } else if(currentToken.tokenType==INT) {
        t->child[0] = params_list(p);
    }
    else {
        genError("");
    }
    return t;
}

TreeNode * Parser::params_list(TreeNode *k) { // k may be a void node or NULL
    TreeNode * t = param(k);
    TreeNode * p = t;
    k = NULL;
    while(currentToken.tokenType==COMMA) {
        TreeNode *q = NULL;
        match(COMMA);
        q = param(k); // just pass the k to the param node
        if(q !=NULL) {
            if(t == NULL) {
                t = p = q;
            } else {
                p -> sibling = q;
                p = q;
            }
        }
    }
    return t; // the first param
}
TreeNode * Parser::param(TreeNode *k) {
    TreeNode *t = newNode(Param);
    TreeNode *p = NULL;
    TreeNode *q = NULL;
    if(k == NULL && currentToken.tokenType == INT) {
        p = newNode(Int);
        match(INT);
    } else if(k != NULL) {
        p = k; // void
    }
    if(p != NULL) {
        t->child[0] = p;
        if(currentToken.tokenType==ID) {
            q = newNode(Id);
            sprintf(q->attr.name,"%s",currentToken.tokenString.c_str());
            t->child[1]=q;
            match(ID);
        } else {
            genError("3");
        }
        if(currentToken.tokenType == LM && t->child[1] != NULL) {
            match(LM);
            t->child[2] = newNode(Var_dec);
            match(RM);
        } else {
            return t;
        }
    } else {
        genError("4");
    }
    return t;   
} 
TreeNode * Parser::compound(void) {
    TreeNode * t = newNode(Comp);
    match(LB);
    t->child[0] = local_declaration();
    t->child[1] = statement_list();
    match(RB);
    return t;
}
TreeNode * Parser::local_declaration(void) {
    TreeNode *t = NULL;
    TreeNode *p = NULL;
    TreeNode *q = NULL;
    while(currentToken.tokenType==INT || currentToken.tokenType==VOID) {
        p = newNode(Var_dec);
        if(currentToken.tokenType==INT) {
            TreeNode * q1 = newNode(Int);
            p->child[0] = q1;
            match(INT);
        } else if(currentToken.tokenType == VOID) {
            TreeNode *q1 = newNode(Void);
            p->child[0] = q1;
            match(VOID);
        }
        if(p != NULL && currentToken.tokenType == ID) {
            TreeNode * q2 = newNode(Id);
            sprintf(q2->attr.name,"%s",currentToken.tokenString.c_str());
            p->child[1] = q2;
            match(ID);
            if(currentToken.tokenType==LM) {// array declaration
                TreeNode *m = newNode(Arr_dec);
                match(LM);
                match(NUM);
                TreeNode *q3 = newNode(Const);
                q3->attr.val = atoi(lastToken.tokenString.c_str());
                m->child[0]=q2;//id
                m->child[1]=q3;//size
                p->child[1]=m;//kind->array
                match(RM);
                match(SEMI);
            } else if(currentToken.tokenType==SEMI) {//normal var
                match(SEMI);
            } else {
                genError("1");
            }
        } else {
            genError("5");
        }
        if(p != NULL) {
            if(t == NULL) {
                t = q = p;
            } else {
                q->sibling = p;
                q = p;
            }
        }
    }
    return t;
}
TreeNode * Parser::statement_list(void) {
    TreeNode *t = statement();
    TreeNode *p = t;
    while(currentToken.tokenType == IF || currentToken.tokenType == LB ||
    currentToken.tokenType == ID || currentToken.tokenType == WHILE ||
    currentToken.tokenType == RETURN || currentToken.tokenType == SEMI ||
    currentToken.tokenType == LS || currentToken.tokenType == NUM) {
        TreeNode *q;
        q = statement();
        if(q != NULL) {
            if(t==NULL) {
                t = p = q;
            } else {
                p -> sibling = q;
                p = q;
            }
        }
    }
    return t;
}
TreeNode * Parser::statement(void) {
    TreeNode *t = NULL;
    switch(currentToken.tokenType) {
        case IF:
            t = selection();
            break;
        case WHILE:
            t = iteration();
            break;
        case RETURN:
            t = return_stmt();
            break;
        case LB:
            t = compound();
            break;
        case ID:
        case SEMI:
        case LS:
        case NUM:
            t = expression_stmt();
            break;
        default:
            genError("6");
            currentToken = getToken();
            break;
    }
    return t;
}
TreeNode * Parser::selection(void) {
    TreeNode * t = newNode(Selec);
    match(IF);
    match(LS);
    if(t != NULL) {
        t -> child[0] = expression();
    }
    match(RS);
    t -> child[1] = statement();
    if(currentToken.tokenType== ELSE) {
        match(ELSE);
        if(t != NULL) {
            t -> child[2] = statement();
        }
    }
    return t;
}
TreeNode * Parser::expression_stmt(void) {
    TreeNode *t = NULL;
    if(currentToken.tokenType==SEMI) {
        match(SEMI);
        return t;
    } else {
        t = expression();
        match(SEMI);
    }
    return t;
}
TreeNode * Parser::iteration() {
    TreeNode *t = newNode(Iter);
    match(WHILE);
    match(LS);
    if(t != NULL) {
        t -> child[0] = expression();
    } else {
        genError("7");
    }
    match(RS);
    if(t != NULL) {
        t -> child[1] = statement();
    } else {
        genError("8");
    }
    return t;
}

TreeNode * Parser::return_stmt(void) {
    TreeNode * t = newNode(Ret);
    match(RETURN);
    if(currentToken.tokenType==SEMI) {
        match(SEMI);
        return t;
    } else {
        if(t != NULL) {
            t -> child[0] = expression();
        }
        match(SEMI);
        return t;
    }
}
TreeNode *Parser::expression(void) {
    TreeNode * t = var();
    if(t == NULL) {
        t = simple_expression(t);
    } else {//assign or simple expression(var/call)
        TreeNode *p = NULL;
        if(currentToken.tokenType==ASSIGN) {
            p = newNode(Assign);
            sprintf(p->attr.name,"%s",currentToken.tokenString.c_str());
            match(ASSIGN);
            p->child[0] = t;
            p->child[1] = expression();
            return p;
        } else {
            t = simple_expression(t);
        }
    }
    return t;
}

TreeNode * Parser::simple_expression(TreeNode * k) {
    TreeNode * t = additive_expression(k);
    k = NULL;
    if(currentToken.tokenType==EQ || currentToken.tokenType==GT ||
    currentToken.tokenType==GEQ || currentToken.tokenType==LT ||
    currentToken.tokenType==LEQ || currentToken.tokenType==NEQ) {
        TreeNode * q = newNode(Op);
        q -> attr.op = currentToken.tokenType;
        q -> child[0] = t;
        t = q;
        match(currentToken.tokenType);
        t->child[1] = additive_expression(k);
        return t;
    }
    return t;
}
TreeNode * Parser::additive_expression(TreeNode * k) {
    TreeNode * t = term(k);
    k = NULL;
    while((currentToken.tokenType==PLUS)||(currentToken.tokenType==MINUS)) {
        TreeNode * q = newNode(Op);
        q -> attr.op = currentToken.tokenType;
        q -> child[0] = t;
        match(currentToken.tokenType);
        q ->child[1] = term(k);
        t = q;
    }
    return t;
}
TreeNode * Parser::term(TreeNode * k) {
    TreeNode * t = factor(k);
    k = NULL;
    while(currentToken.tokenType==MUL || currentToken.tokenType == DIV) {
        TreeNode * q = newNode(Op);
        q ->attr.op = currentToken.tokenType;
        q -> child[0] = t;
        t = q;
        match(currentToken.tokenType);
        q -> child[1] = factor(k);
    }
    return t;
}
TreeNode * Parser::factor(TreeNode * k) {
    TreeNode *t = NULL;
    if(k != NULL) {
        if(currentToken.tokenType==LS && k ->nodekind!=Arry_elem) {
            t = call(k);
        } else {
            t = k;
        }
    } else {
        switch(currentToken.tokenType) {
            case LS:
                match(LS);
                t = expression();
                match(RS);
                break;
            case ID:
                k = var();
                if(currentToken.tokenType==LS && k -> nodekind != Arry_elem) {
                    t = call(k);
                }
                t=k;
                break;
            case NUM:
                t = newNode(Const);
                if(t != NULL && currentToken.tokenType == NUM) {
                    t -> attr.val = atoi(currentToken.tokenString.c_str());
                    
                }
                match(NUM);
                break;
            default:
                genError("9");
                currentToken = getToken();
                break;
        }
    }
    return t;
}
TreeNode *Parser::var(void) {
    TreeNode *t = NULL;
    TreeNode *p = NULL;
    TreeNode *q = NULL;
    if(currentToken.tokenType==ID) {
        p = newNode(Id);
        sprintf(p->attr.name,"%s",currentToken.tokenString.c_str());
        match(ID);

        if(currentToken.tokenType == LM) {
            match(LM);
            q = expression();
            match(RM);
            t = newNode(Arry_elem);
            t -> child[0] = p;
            t->child[1] = q;
        } else {
            t = p;
        }
    }
    return t;
}
TreeNode * Parser::call(TreeNode * k) {
    TreeNode * t = newNode(Call);
    if(k != NULL) {
        t -> child[0] = k;
    }
    match(LS);
    if(currentToken.tokenType==RS) {
        match(RS);
        return t;
    } else if( k != NULL) {
        t -> child[1] = args();
        match(RS);
    }
    return t;
}
TreeNode *Parser::args(void) {
    TreeNode *t = newNode(Args);
    TreeNode *s = NULL;
    TreeNode *p = NULL;
    if(currentToken.tokenType!= RS) {
        s = expression();
        p = s;
        while(currentToken.tokenType==COMMA) {
            TreeNode * q;
            match(COMMA);
            q = expression();
            if(q != NULL) {
                if(s == NULL) {
                    s = p= q;
                } else {
                    p -> sibling = q;
                    p = q;
                }
            }
        }
    }
    if(s != NULL) {
        t -> child[0] = s;
    }
    return t;
}
void Parser::printSpace(int n) {
    for(int i = 0; i < n; i++) {
        fout_Tree<<"    ";
    }
}
void Parser::printTree(TreeNode * t) {
    int i;
    while(t != NULL) {
        printSpace(step);
        if(t->nodekind != Id&& t->nodekind != Op)fout_Tree << t ->nodekind << endl;
        else if(t->nodekind == Id)fout_Tree << t ->nodekind <<":" << t->attr.name<< endl;
        else fout_Tree << t ->nodekind <<":" << t->attr.op<< endl;
        step++;
        for(i=0;i < MAX_child;i++) {
            if(t->child[i] != NULL) {
                printTree(t->child[i]);
            }
        }
        step--;
        t=t->sibling;
    }
}
// int main() {
//     Parser P("test.txt");
//     fout_Tree.close();
// }
