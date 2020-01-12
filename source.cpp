#include "source.h"
#include "llvm/Support/raw_ostream.h"




void SourceManager::open(const std::string& path) {
    std::ifstream in(path);
    if(!in.is_open())std::cout<< "error could not open file";
    std::string code((std::istreambuf_iterator<char>(in)),std::istreambuf_iterator<char>());
    in.close();
    sources.push_back(FSFile(path,code+"\0\0"));
}
/*
void SourceManager::open_(std::experimental::filesystem::path path) {
    using namespace std::experimental::filesystem;
    if(!exists(path)) {
        error(ERR_FILE_NOT_EXISTS,path.concat(" does not exists."));
    }
    if(path.extension()!="fs") {
        error(ERR_ONLY_FS_FILES,"Only files with .fs extension are allowed!");
    }
    std::ifstream in(path);
    std::string code((std::istreambuf_iterator<char>(in)),std::istreambuf_iterator<char>());
    in.close();
    sources.push_back(FSFile(path,code));
}
*/
const std::string FSFile::get_line(int line)const {
    size_t pos=0;
    for(int i=0;i<line-1;i++) {
        pos=code.find('\n',pos);
        if(pos==std::string::npos) {
            //ERROR
            throw "shit";
        }
    }
    auto b =&code[pos];
    auto len = code.find('\n',pos);
    return std::string(b,len); 
    
}

FSFile& FSFile::operator=(const FSFile& other) {
    code=other.code;
    path=other.path;
    return *this;
}