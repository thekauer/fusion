#include <iostream>
#include "compiler.h"


int main(int argc,char** argv) {
    Compiler compiler;
    compiler.compile(argc,argv);
    return 0;
}
