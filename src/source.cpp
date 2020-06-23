#include "source.h"
#include "llvm/Support/raw_ostream.h"

void SourceManager::open(const std::string &path) {
  std::ifstream in(path);
  if (!in.is_open())
    serror(Error_e::FileNExists,
           std::string("File ") + path + " doesn't exist.");
  if (path.find(".fs") == path.npos)
    serror(Error_e::MustbeFsFile, "File must have .fs extension!");
  std::string code((std::istreambuf_iterator<char>(in)),
                   std::istreambuf_iterator<char>());
  in.close();
  sources.push_back(FSFile(path, code + "\0\0"));
}

FSFile &FSFile::operator=(const FSFile &other) {
  code = other.code;
  path = other.path;
  return *this;
}
