#include "error.h"
#include "llvm/Support/FormattedStream.h"
#include "llvm/Support/raw_ostream.h"
#include <sstream>
#include <string>

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

static void err_impl(Error_e code, const FSFile &file, const SourceLocation &sl,
                     const std::string &msg, llvm::raw_ostream::Colors color,
                     const std::string &title) {
  auto rsc = respawn(file, sl.pos);
  std::string s = ": " + msg + "\n" + rsc.code + "\n";
  llvm::formatted_raw_ostream ro(llvm::outs());
  ro.changeColor(color, true, false);
  ro.write(title.c_str(), title.size());
  ro.resetColor();
  ro.write(s.c_str(), s.size());
  for (int i = 0; i < rsc.col; i++) {
    llvm::outs() << " ";
  }
  llvm::outs() << "^\n"
            << "\n";
}
void error(Error_e code, const FSFile &file,
                        const SourceLocation &sl, const std::string &msg) {

  err_impl(code,file,sl,msg,llvm::raw_ostream::RED,"error: ");
  
}
void warning(Error_e code, const FSFile &file, const SourceLocation &sl,
             const std::string &msg) {
  err_impl(code,file,sl,msg,llvm::raw_ostream::MAGENTA,"warning:");
}
void note(Error_e code, const FSFile &file, const SourceLocation &sl,
          const std::string &msg) {
  err_impl(code,file,sl,msg,llvm::raw_ostream::BLACK,"note: ");
}

void Error::UnkEscapeChar(const FSFile &file, const SourceLocation &sl,
                   const char ch) {
  std::ostringstream os;
  os << "Unknown escape character \'\\" << ch << "\'.";

  error(Error_e::UnkEsc, file, sl, os.str());
}

void Error::ExpectedToken(const FSFile& file,const SourceLocation& sl,const std::string& msg) {
  error(Error_e::ExpectedToken,file,sl,msg);
}
