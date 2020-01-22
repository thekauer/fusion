#include "error.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/FormattedStream.h"
#include <string>

void error(Error_e code,const std::string& msg,const SourceLocation& sl) {
    std::string s =sl.file.path +":" +
    std::to_string(sl.line) + "." + 
    std::to_string(sl.col) + ": "+msg + 
    "\n" +sl.file.get_line(sl.line)+"\n";
    llvm::formatted_raw_ostream ro(llvm::outs());
    ro.changeColor(llvm::raw_ostream::RED,true,false);
    ro.write("error in ",9);
    ro.resetColor();
    ro.write(s.c_str(),s.size());
    for(int i=0;i<sl.col;i++) {
        std::cout << " ";
    }
    std::cout << "^\n" <<"\n";
    std::exit(static_cast<int>(code));  
}


void serror(Error_e code,const std::string& msg) {
	llvm::formatted_raw_ostream ro=llvm::formatted_raw_ostream(llvm::outs());
    ro.changeColor(llvm::raw_ostream::RED,true,false);
    ro.write("error: ",7);
    ro.resetColor();
    ro.write(msg.c_str(),msg.size());
    ro.flush();
    std::exit(static_cast<int>(code));
}
