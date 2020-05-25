#include "scanner.h"
#include <cstdlib>
#include <fstream>
#include <string>
#include <vector>
#include <iostream>
using namespace std;
//init function
Scanner::Scanner() {
    isSuccess = true;
    charIndex = 0;
    tokenString = "";
    isComment = false;
    sourseString = "";
    lineNumber = 0;
}
void Scanner::scan(void) {
    cout << "scanner begin" << endl;
    bool doubleSym = false; // <=,>= and so on
    getSourse("delComment.txt");
    int state = START;
    lineNumber = 0;
    char ch;
    while(state <= DONE) {
        //cout << state << endl;
        ch = getNext();
        //got an EOF
        if(ch == '\0') {
            Token t;
            t.lineNo = lineNumber;
            t.tokenType=ENDF;
            tokenList.push_back(t);
            break;
        }
        //DFA state change
        //just refer to the DFA
        if(state == START) {
            state = charType(ch);
            if(state != START) {
                tokenString += ch;
            }
        } else if(state == ST_NUM) { // digit
            state = charType(ch);
            if(state != ST_NUM) {
                state = DONE;
            } else {
                tokenString += ch;
            }
        } else if (state == SYMB) {
            if(ch == '=') {
                tokenString += ch;
                doubleSym = true;
            } else {
                doubleSym = false;
            }
            state = DONE;
        } else if (state == ST_ID) {
            state = charType(ch);
            if(state != ST_ID) {
                state = DONE;
            } else {
                tokenString += ch;
            }
        }
        if (state == DONE) { // got a token
            int tp = 0;
            if(ch == '\n') {
                tp=1;
            } 
            Token t;
            t.lineNo = lineNumber - tp;// if got an '\n', then the linenumber has increased
            t.tokenString = tokenString;
            t.tokenType = getTokenType(tokenString);
            tokenList.push_back(t);
            if(t.tokenType == ERROR) {
                isSuccess = false;
            }
            int lastState = charType(tokenString[tokenString.length()-1]);
            // if doubleSym is true than dont need to rollback
            if(lastState == ST_NUM || lastState == ST_ID || (lastState == SYMB && !doubleSym)) {
                rollBack();
            }
            tokenString = "";
            state = START;
            doubleSym = false;
        }
    }
    if(isSuccess) {
        cout << "scanner finished with no errors" << endl;
        //cout << "the result has saved to " << target << endl;
    } else {
        cout << "scanner failed to scan the code, please check the code" << endl;
    }
    printToken("scanner.txt");
}
Token Scanner::getToken(int index) {
    Token t;
    t.lineNo = lineNumber;
    t.tokenString = "";
    t.tokenType = ENDF;
    if(index < tokenList.size()) {
        t = tokenList.at(index);
    }
    return t;
}
void Scanner::getSourse(string path) {
    ifstream fin(path.c_str());
    string tmp;
    sourseString = "";
    while(getline(fin,tmp)) {
        sourseString += tmp;
        sourseString += '\n';
    }
    fin.close();
    charIndex=0;
} 
void Scanner::delComments() {
    cout << "begin to scan the comments" << endl;
    ofstream fout("delComment.txt");
    int state = 1;
    char ch;
    while(state <= 5) {
        ch = getNext();
        if(ch=='\0') {
            break;
        }
        if(state == 1) {
            if(ch == '/') {
                state = 2;
            } else {
                fout << ch;
            }
        } else if(state == 2) {
            if(ch == '*') {
                state = 3;
                isComment = true;
            } else {
                state = 1;
                fout << "/" << ch; 
            }
        } else if(state == 3) {
            if(ch == '*') {
                state = 4;
            }
        } else if(state == 4) {
            if(ch == '/') {
                state = 5;
            } else if(ch != '*') {
                state = 3;
            }
        }
        if(state == 5) {
            isComment = false;
            state = 1;
        }
    }
    if(isComment) {
        cout << "ERROR:can't find the end symbol of comment" << endl;
        isSuccess = false;
    } else {
        cout << "completely delete the comments" << endl;
    }
}
TokenType Scanner::getTokenType(string s) {
    TokenType t;
    if(s == "else") {
        t = ELSE;
    }
    else if(s == "if") {
        t = IF;
    } else if(s == "int") {
        t = INT;
    } else if(s == "return") {
        t = RETURN;
    } else if(s == "void") {
        t = VOID;
    } else if(s == "while") {
        t = WHILE;
    } else if(s == "+") {
        t = PLUS;
    } else if(s == "-") {
        t = MINUS;
    } else if(s == "*") {
        t = MUL;
    } else if(s == "/") {
        t = DIV;
    } else if(s=="<") {
        t = LT;
    } else if(s=="<=") {
        t = LEQ;
    } else if(s == ">") {
        t = GT;
    } else if(s == ">=") {
        t = GEQ;
    } else if(s == "==") {
        t = EQ;
    } else if (s == "==") {
        t = NEQ;
    } else if(s == "=") {
        t = ASSIGN;
    }else if (s == ";") {
        t = SEMI;
    }else if(s == ",") {
        t = COMMA;
    }else if(s == "(") {
        t = LS;
    } else if(s == ")") {
        t = RS;
    } else if(s == "[") {
        t = LM;
    } else if(s == "]") {
        t = RM;
    } else if(s == "{") {
        t = LB;
    } else if(s == "}") {
        t = RB;
    } else if (charType(s[s.length()-1]) == ST_NUM) {
        t = NUM;
    } else if (charType(s[s.length()-1]) == ST_ID) {
        t = ID;
    }
    return t;
}
DFAState Scanner::charType(char c) {
    if(c == ' ' || c == '\n' || c == '\t' || c == '\r') {
        return START;
    } else if(c>='0' && c <= '9') {
        return ST_NUM;
    } else if((c >='A' && c <= 'Z') ||(c >='a' && c <= 'z')) {
        return ST_ID;
    } else if(c == '<' || c == '>' || c == '=' || c == '!') {
        return SYMB;
    } else {
        return DONE;
    }
} 
char Scanner::getNext() {
    if(charIndex < sourseString.length()) {
        char ch = sourseString[charIndex];
        charIndex++;
        if(ch == '\n') {
            lineNumber++;
        }
        return ch;
    } else {
        return '\0';
    }
}
void Scanner::rollBack() {
    if(charIndex > 0) {
        char ch = sourseString[charIndex - 1];
        charIndex--;
        if(ch == '\n') {
            lineNumber--;
        }
    }
}
void Scanner::printToken(string filename) {
    ofstream fout(filename);
    ifstream fin("tmp.txt");
    string tmp;
    int lineCount = 0;
    int index = 0;
    while(getline(fin,tmp)) {
        fout << lineCount << ":";
        fout << tmp << endl;
        while(index < tokenList.size()) {
            Token t = tokenList.at(index);
            if(lineCount == t.lineNo) {
                fout << "   " << lineCount << ":    [";
                index++;
                fout << t.tokenType << "]" << t.tokenString << endl;
            }
            if(lineCount < t.lineNo) {
                break;
            }
        }
        lineCount++;
    }
    fin.close();
    fout.close();
}