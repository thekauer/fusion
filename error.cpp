#include "error.h"



void error(Error_e code,const std::string& msg,const SourceLocation& sl) {
    std::cout <<sl.file.path << ":" <<
    std::to_string(sl.line) << "." << 
    std::to_string(sl.col) << ": "<<msg << 
    "\n" <<sl.file.get_line(sl.line)<<"\n";
    for(int i=0;i<sl.col;i++) {
        std::cout << " ";
    }
    std::cout << "^\n" <<"\n";
    std::exit(static_cast<int>(code));  
    throw static_cast<int>(code);
}


