#include <iostream>
#include "lex.h"


int main() {
    SourceManager sm;
    sm.open("main.fs");
    Lexer lex(sm.sources[0]);
    int max=15;
    while(lex.it!=lex.end) 
    std::cout << lex.next().type << " ";
    
    
    
    return 0;
}