#include <iostream>
#include "parser.h"


int main() {
    SourceManager sm;
    sm.open("main.fs");
    /*
    Lexer lex(sm.sources[0]);
    int max=15;
    while(lex.it!=lex.end) 
    std::cout << lex.next().type << " ";
    */
   
    Parser p(sm.sources[0]);
    auto err = SourceLocation(sm.sources[0]);
    err.line=2;
    err.col=11;
    error(Error_e::ExpectedToken,"works?",err);
    auto m = p.parse_fndecl();
    if(m) std::cout << static_cast<int>(m->type);
    
    
    return 0;
}