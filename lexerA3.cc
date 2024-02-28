#include <iostream>
#include <istream>
#include <vector>
#include <string>
#include <cctype>
#include <stdlib.h>
#include <string.h>
#include <map>
#include <stack>

#include "lexer.h"
#include "inputbuf.h"

using namespace std;

string reserved[] = { "END_OF_FILE", "INT", "REAL", "BOO", "TR", "FA", "IF", "WHILE", "SWITCH", "CASE", "PUBLIC", "PRIVATE", "NUM", "REALNUM", "NOT", "PLUS", "MINUS", "MULT", "DIV", "GTEQ", "GREATER", "LTEQ", "NOTEQUAL", "LESS", "LPAREN", "RPAREN", "EQUAL", "COLON", "COMMA", "SEMICOLON", "LBRACE", "RBRACE", "ID", "ERROR" // TODO: Add labels for new token types here (as string)
};

#define KEYWORDS_COUNT 11
string keyword[] = { "int", "real", "bool", "true", "false", "if", "while", "switch", "case", "public", "private" };

LexicalAnalyzer lexer;
Token token;
TokenType tempTokenType;

std::map<string, string> typeMap;
std::map<string, vector<string> > unknownGroups;
struct Node
{
    string val;
    TokenType type;
};

std::stack<Node> operators;
std::stack<Node> variables;
bool errorFound = false;

void add_to_map(const std::string& str, string type) {
    typeMap[str] = type;
}

void add_vars_to_map(std::vector<string> vars, string type) {
    for(string s : vars) {
        add_to_map(s, type);
    }
}

void display() {
    for (const auto& pair : typeMap) {
        std::cout << pair.first;
        if(pair.second == "int" || pair.second == "real" || pair.second == "bool")
            std::cout << ": "<< pair.second << " #"  << std::endl;
    } 
}

LexicalAnalyzer::LexicalAnalyzer()
{
    this->line_no = 1;
    tmp.lexeme = "";
    tmp.line_no = 1;
    tmp.token_type = ERROR;
}

bool LexicalAnalyzer::SkipComments()
{
    bool comments = false;
    char c;
    if(input.EndOfInput() ){
        input.UngetChar(c);
        return comments;
    }
    input.GetChar(c);   
    if(c == '/'){
        input.GetChar(c);
        if(c == '/'){
            comments = true;
            while(c != '\n'){
                comments = true;
                input.GetChar(c);
            }
            line_no++;
            SkipComments();
        }else{
            comments = false;
            cout << "Syntax Error\n";
            exit(0);
        }
    }else{
           input.UngetChar(c);          
           return comments;
    }   
}

bool LexicalAnalyzer::SkipSpace()
{
    char c;
    bool space_encountered = false;

    input.GetChar(c);
    line_no += (c == '\n');

    while (!input.EndOfInput() && isspace(c)) {
        space_encountered = true;
        input.GetChar(c);
        line_no += (c == '\n');
    }

    if (!input.EndOfInput()) {
        input.UngetChar(c);
    }
    return space_encountered;
}

bool LexicalAnalyzer::IsKeyword(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return true;
        }
    }
    return false;
}

TokenType LexicalAnalyzer::FindKeywordIndex(string s)
{
    for (int i = 0; i < KEYWORDS_COUNT; i++) {
        if (s == keyword[i]) {
            return (TokenType) (i + 1);
        }
    }
    return ERROR;
}

Token LexicalAnalyzer::ScanNumber()
{
    char c;
    bool realNUM = false;
    input.GetChar(c);
    if (isdigit(c)) {
        if (c == '0') {
            tmp.lexeme = "0";
            input.GetChar(c);
            if(c == '.'){
                //cout << "\n I am here too " << c << " \n";
                input.GetChar(c);
                if(!isdigit(c)){
                    input.UngetChar(c);
                }else{
                    while (!input.EndOfInput() && isdigit(c)) {
                        tmp.lexeme += c;
                        input.GetChar(c);
                        realNUM = true;    
                    }   
                    input.UngetChar(c);
                }
            }else{
                input.UngetChar(c);
            }
        } else {
            tmp.lexeme = "";
            while (!input.EndOfInput() && isdigit(c)) {
                tmp.lexeme += c;
                input.GetChar(c);
            }
            if(c == '.'){ 
                //cout << "\n I am here too " << c << " \n";
                input.GetChar(c);
                if(!isdigit(c)){
                    input.UngetChar(c);
                }else{
                    while (!input.EndOfInput() && isdigit(c)) {
                        tmp.lexeme += c;
                        input.GetChar(c);
                        realNUM = true;
                    }   
                }
            }
            if (!input.EndOfInput()) {
                input.UngetChar(c);
            }
        }
        if(realNUM){
            tmp.token_type = REAL;
        }else{
            tmp.token_type = INT;
        }
        tmp.line_no = line_no;
        return tmp;
    } else { 
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
        tmp.line_no = line_no;
        return tmp;
    }
}

Token LexicalAnalyzer::ScanIdOrKeyword()
{
    char c;
    input.GetChar(c);
    
    if (isalpha(c)) {
        tmp.lexeme = "";
        while (!input.EndOfInput() && isalnum(c)) {
            tmp.lexeme += c;
            input.GetChar(c);
        }
        
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.line_no = line_no;
        
        if (IsKeyword(tmp.lexeme))
            tmp.token_type = FindKeywordIndex(tmp.lexeme);
        else
            tmp.token_type = ID;
    } else {
        if (!input.EndOfInput()) {
            input.UngetChar(c);
        }
        tmp.lexeme = "";
        tmp.token_type = ERROR;
    }
    return tmp;
}

TokenType LexicalAnalyzer::UngetToken(Token tok)
{
    tokens.push_back(tok);;
    return tok.token_type;
}

Token LexicalAnalyzer::GetToken()
{
    char c;

    // if there are tokens that were previously
    // stored due to UngetToken(), pop a token and
    // return it without reading from input
    if (!tokens.empty()) {
        tmp = tokens.back();
        tokens.pop_back();
        return tmp;
    }

    SkipSpace();
    SkipComments();
    SkipSpace();
    tmp.lexeme = "";
    tmp.line_no = line_no;
    input.GetChar(c);
    //cout << "\n Char obtained " << c << "\n";
    switch (c) {
        case '!':
            tmp.token_type = NOT;
            return tmp;    
        case '+':
            tmp.token_type = PLUS;
            return tmp;    
        case '-':
            tmp.token_type = MINUS;
            return tmp; 
        case '*':
            tmp.token_type = MULT;
            return tmp;       
        case '/':
            tmp.token_type = DIV;
            return tmp;    
        case '>':
            input.GetChar(c);
            if(c == '='){
                tmp.token_type = GTEQ;   
            }else{
                input.UngetChar(c);
                tmp.token_type = GREATER;
            }
            return tmp;    
        case '<':
            input.GetChar(c);
            if(c == '='){
                tmp.token_type = LTEQ;   
            }else if (c == '>'){
                tmp.token_type = NOTEQUAL;    
            }else{
                input.UngetChar(c);
                tmp.token_type = LESS;
            }
            return tmp;            
        case '(':
            //cout << "\n I am here" << c << " \n";
            tmp.token_type = LPAREN;
            return tmp;    
        case ')':
            tmp.token_type = RPAREN;
            return tmp;    
        case '=':
            tmp.token_type = EQUAL;
            return tmp;
        case ':':
            tmp.token_type = COLON;
            return tmp;
        case ',':
            tmp.token_type = COMMA;
            return tmp;
        case ';':
            tmp.token_type = SEMICOLON;
            return tmp;
        case '{':
            tmp.token_type = LBRACE;
            return tmp;
        case '}':
            tmp.token_type = RBRACE;
            return tmp;
        default:
            if (isdigit(c)) {
                input.UngetChar(c);
                return ScanNumber();
            } else if (isalpha(c)) {
                input.UngetChar(c);
                //cout << "\n ID scan " << c << " \n"; 
                return ScanIdOrKeyword();
            } else if (input.EndOfInput())
                tmp.token_type = END_OF_FILE;
            else
                tmp.token_type = ERROR;

            return tmp;
    }
}

string parse_typename(void){
    token = lexer.GetToken();
    if(token.token_type == INT){
        return "int"; 
    } else if(token.token_type == REAL){
        return "real";
    } else {
        return "bool";
    }  
}

int parse_vardecllist(void){
    string type;
    vector<string> vars;
    while(token.token_type != SEMICOLON){
        // add variable to vector
        vars.push_back(token.lexeme);
        token = lexer.GetToken();
        if(token.token_type == COMMA){
            // get next variable
            // cout<< "got comma\n";
            token = lexer.GetToken();
        } if(token.token_type == COLON){
            // get type of var and add the list to map
            // cout << "got type\n";
            type = parse_typename();
            add_vars_to_map(vars, type);
            // getting the semicolon
            token = lexer.GetToken();
        }
    }
    return(0);
}

void parse_globalVars(void){
    // got the first id
    token = lexer.GetToken();
    // read until body starts
    while (token.token_type != LBRACE){
        // cout << "parsing vars line by line\n";
        parse_vardecllist();  
        token = lexer.GetToken();     
    }
    // ungetting LBR
    tempTokenType = lexer.UngetToken(token);
}

void evaluate(Node op, Node var1, Node var2){
    if(op.type == NOT){
        if(var1.type == ID){
            if (typeMap.find(var1.val) != typeMap.end()){
                if(typeMap[var1.val] != "bool"){
                    cout << "TYPE MISMATCH "<< token.line_no <<" C3\n";
                    exit(1);
                }
            } else {
                typeMap[var1.val] = "bool";
            }
        }
    }
}

bool parse_expression(string expected){
    token = lexer.GetToken();
    int vars_needed = 0;
    while(token.token_type != SEMICOLON || token.token_type != RPAREN){
        Node curr_node = {token.lexeme, token.token_type};
        Node op;
        Node var;
        if(token.token_type == PLUS || token.token_type == MINUS || token.token_type == MULT || token.token_type == DIV){
            // return value is same as input, both inputs have same type
            vars_needed = 2;
            operators.push(curr_node);
        } else if(token.token_type == GREATER || token.token_type == LESS || token.token_type == GTEQ || token.token_type == LTEQ || token.token_type == NOTEQUAL || token.token_type == EQUAL){
            // return value is bool, both inputs are same type
            vars_needed = 2;
            operators.push(curr_node);
        } else if(token.token_type = NOT){
            // return value is bool input is bool
            vars_needed = 1;
            operators.push(curr_node);
        } else if(token.token_type == ID){
            // id 
            if(vars_needed == 1){
                op = operators.top();
                operators.pop();
                if(op.type != NOT){
                    var = variables.top();
                    variables.pop();
                } else {
                    if(curr_node.type == ID){
                        if (typeMap.find(curr_node.val) != typeMap.end()){
                            if(typeMap[curr_node.val] != "bool"){
                                cout << "TYPE MISMATCH "<< token.line_no <<" C3\n";
                                errorFound = true;
                                return false;
                            }
                        } else {
                            typeMap[curr_node.val] = "bool";
                        }
                    }
                }
            } else {
                variables.push(curr_node);
                vars_needed--;
            }
        } else if(token.token_type == INT || token.token_type == REAL || token.token_type == BOOL){
            // static values
            if(vars_needed == 1){
                op = operators.top();
                var = variables.top();
                operators.pop();
                variables.pop();
            } else {
                variables.push(curr_node);
                vars_needed--;
            }
        }
        token = lexer.GetToken();
    } // statement is parsed till RPAREN
    return "";
}

// int parse_ifstmt(void){
//     int tempI;
//     token = lexer.GetToken();
//     if(token.token_type == IF){
//         token = lexer.GetToken();
//         if(token.token_type == LPAREN){
//             parse_expression("bool");
//             token = lexer.GetToken();
//             if(token.token_type == RPAREN){
//                 cout << "\n Rule parsed: ifstmt -> IF LPAREN expression RPAREN body \n";
//                 // parse_body();
                
//             }else{
//                 cout << "\n Syntax Error \n";
//             }
//         }else{
//             cout << "\n Syntax Error \n";
//         }    
//     }else{
//         cout << "\n Syntax Error \n";
//     }
//     return(0);
// }

void parse_stmt(void){
    int tempI;
    token = lexer.GetToken();
    string expected;
    if(token.token_type == ID){
        // cout << "\n Rule parsed: stmt -> assignment_stmt \n";
        // tempI = parse_assstmt();        
        if(typeMap.find(token.lexeme) != typeMap.end()){
            expected = typeMap[token.lexeme];
        } else {
            typeMap[token.lexeme] = "int";
        } 
        token = lexer.GetToken();
        if(parse_expression(expected)){
            cout << "TYPE MISMATCH "<< token.line_no <<" C1\n";
            errorFound = true;
        }
    // }else if(token.token_type == IF){
    //     tempTokenType = lexer.UngetToken(token);
    //     // cout << "\n Rule parsed: stmt -> if_stmt";
    //     tempI = parse_ifstmt();
    // }else if(token.token_type == WHILE){
    //     tempTokenType = lexer.UngetToken(token);
    //     // cout << "\n Rule parsed: stmt -> while_stmt";
    //     // tempI = parse_whilestmt();
    // }else if(token.token_type == SWITCH){
    //     tempTokenType = lexer.UngetToken(token);
    //     // cout << "\n Rule parsed: stmt -> switch_stmt";
    //     // tempI = parse_switchstmt();
    // }else{
    //     cout << "\n Syntax Error \n";
    }
}

void parse_body(void){
    token = lexer.GetToken();
    while(token.token_type != RBRACE){
        parse_stmt();
        token = lexer.GetToken();
    }
}

int parse_program(void){
    
    token = lexer.GetToken();
    int tempI;
    while (token.token_type != END_OF_FILE)
    {
        // code starts with id -> global variables
        if(token.token_type == ID){
            lexer.UngetToken(token);
            // parse the global variables and add to the map
            // cout << "Parsing global\n";
            parse_globalVars();

        } else if(token.token_type == LBRACE){
            tempTokenType = lexer.UngetToken(token);               
            parse_body();
            return 0;
        }else if(token.token_type == END_OF_FILE){
            return(0);
        }else{
            return(0);
        }
        token = lexer.GetToken();
    }
}

int main()
{
    int i;
    // cout << "Start \n";
    i = parse_program();
    // cout << "\n End of Program \n";
    if(!errorFound)
        display();
}