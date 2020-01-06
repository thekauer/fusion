#include "source.h"





void SourceManager::open(const std::string& path) {
    std::ifstream in(path);
    if(!in.is_open())std::cout<< "error could not open file";
    std::string code((std::istreambuf_iterator<char>(in)),std::istreambuf_iterator<char>());
    in.close();
    sources.push_back(FSFile(path,code+"\n\n"));
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
    int l=0;
    auto it = code.begin();
    for(;it!=code.end() && l<=line;it++) {
        if(*it=='\n')l++;
    }
    auto b =it;
    for(;it!=code.end() || *it!='\n';it++);
    return std::string(b,it); 
    
}