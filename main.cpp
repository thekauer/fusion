#include <iostream>
#include "parser.h"

#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Verifier.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/Support/raw_ostream.h"



int main() {
    SourceManager sm;
    sm.open("main.fs");
    /*
    Lexer lex(sm.sources[0]);
    int max=15;
    while(lex.it!=lex.end) 
    std::cout << lex.next().type << " ";
    */
   
    Lexer l(sm.sources[0]);
    l.lex();
    Parser p(l.tokens);
    auto err = SourceLocation(sm.sources[0]);
    err.line=2;
    err.col=11;
    //auto m = p.parse_fndecl();
    auto m = p.parse_fndecl();
    if(m) {
        std::cout <<"yeey\n";
        m->print_name();
    }
    else std::cout << "ayy";

    //error(Error_e::ExpectedToken,"works?",err);
    //auto m = p.parse_fndecl();
    //llvm::Module* mod =  new llvm::Module("tests",ctx);
    //m->print_name();
    //m->proto->print_name();
    //m->body->print_name();
    

    return 0;
}