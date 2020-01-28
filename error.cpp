#include "error.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/raw_ostream.h"
#include <sstream>
#include <string>

/*
void error(Error_e code,const std::string& msg,const SourceLocation& sl) {
    std::string s =": "+msg +
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
}*/

void serror(Error_e code, const std::string &msg) {
  llvm::formatted_raw_ostream ro = llvm::formatted_raw_ostream(llvm::outs());
  ro.changeColor(llvm::raw_ostream::RED, true, false);
  ro.write("error: ", 7);
  ro.resetColor();
  ro.write(msg.c_str(), msg.size());
  ro.flush();
  std::exit(static_cast<int>(code));
}

RespawnedCode respawn(const FSFile &file, unsigned int pos) {
  auto file_name = file.path;
  auto it = file.code.begin();
  auto end = file.code.end();
  int line = 1;
  int col = 1;
  while (pos--) {
    if (*it++ == '\n') {
      line++;
      col = 1;
    } else {
      col++;
    }
  }
  auto start = it;
  while (it != end && *it++ != '\n') { /*empty body*/
  }
  auto code = std::string(start, it);
  return RespawnedCode(line, col, code, file_name);
}
[[noreturn]] void error(Error_e code, const FSFile &file,
  const SourceLocation &sl, const std::string &msg) {
  auto rsc = respawn(file, sl.pos);
  std::string s = ": " + msg + "\n" + rsc.code + "\n";
  llvm::formatted_raw_ostream ro(llvm::outs());
  ro.changeColor(llvm::raw_ostream::RED, true, false);
  ro.write("error: ", 7);
  ro.resetColor();
  ro.write(s.c_str(), s.size());
  for (int i = 0; i < rsc.col; i++) {
    std::cout << " ";
  }
  std::cout << "^\n"
            << "\n";
  std::exit(static_cast<int>(code));
}

static void err_impl(Error_e code,const FSFile& file,const SourceLocation& sl, const std::string& msg,llvm::raw_ostream::Color color,const std::string& title) {
  auto rsc = respawn(file, sl.pos);
  std::string s = ": " + msg + "\n" + rsc.code + "\n";
  llvm::formatted_raw_ostream ro(llvm::outs());
  ro.changeColor(color, true, false);
  ro.write(title.c_str(), title.size());
  ro.resetColor();
  ro.write(s.c_str(), s.size());
  for (int i = 0; i < rsc.col; i++) {
    std::cout << " ";
  }
  std::cout << "^\n"
            << "\n";
    
}
void warning(Error_e code, const FSFile &file, const SourceLocation &sl,
             const std::string &msg) {
  auto rsc = respawn(file, sl.pos);
  std::string s = ": " + msg + "\n" + rsc.code + "\n";
  llvm::formatted_raw_ostream ro(llvm::outs());
  ro.changeColor(llvm::raw_ostream::MAGENTA, true, false);
  ro.write("warning: ", 9);
  ro.resetColor();
  ro.write(s.c_str(), s.size());
  for (int i = 0; i < rsc.col; i++) {
    std::cout << " ";
  }
  std::cout << "^\n"
            << "\n";
}
void note(Error_e code, const FSFile &file, const SourceLocation &sl,
          const std::string &msg) {
  auto rsc = respawn(file, sl.pos);
  std::string s = ": " + msg + "\n" + rsc.code + "\n";
  llvm::formatted_raw_ostream ro(llvm::outs());
  ro.changeColor(llvm::raw_ostream::BLACK, true, false);
  ro.write("note: ", 6);
  ro.resetColor();
  ro.write(s.c_str(), s.size());
  for (int i = 0; i < rsc.col; i++) {
    std::cout << " ";
  }
  std::cout << "^\n"
            << "\n";
}

void Error::UnkEsc(const FSFile &file, const SourceLocation &sl,
                   const char ch) {
  std::ostringstream os;
  os << "Unknown escape character \'\\" << ch << "\'.";

  error(Error_e::UnkEsc, file, sl, os.str());
}
