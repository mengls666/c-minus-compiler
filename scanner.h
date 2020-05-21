#include <string>
#include <vector>
using namespace std;
/* 
tokentype:
else,if,int,return,void,while,
+,-,*,/,<,<=,>,>=,==,!=,=,,,;,(,),
[,],{,},
num,id,error,EOF
*/
typedef enum
{
    ELSE = 1,
    IF,
    INT,
    RETURN,
    VOID,
    WHILE,
    PLUS,
    MINUS,
    MUL,
    DIV,
    LT,
    LEQ,
    GT,
    GEQ,
    EQ,
    NEQ,
    ASSIGN,
    SEMI,
    COMMA,
    LS,
    RS,
    LM,
    RM,
    LB,
    RB,
    LCO,
    RCO,
    NUM,
    ID,ERROR,
    ENDF
} TokenType;
//DFA states
typedef enum
{
    START = 1,
    ST_NUM,
    ST_ID,
    SYMB, // double Symbols
    DONE
} DFAState;
//the string of each type of token
//e.g. tokenTypeString[ELSE]=="ELSE"
struct Token
{
    TokenType tokenType;
    string tokenString;
    int lineNo;
};
class Scanner
{
public:
    bool isSuccess;
    void getSourse(string filename);
    void delComments();
    void scan(void);
    Scanner();
    Token getToken(int index);//get token from tokenList 

private:
    DFAState charType(char);
    char getNext();
    void rollBack();//back to last char
    TokenType getTokenType(string s);
    void printToken(string filename);//output the scanner's result
    string sourseString;
    int charIndex;
    string tokenString;
    bool isComment;
    int lineNumber;
    vector <Token> tokenList;
};