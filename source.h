#pragma once
#include <iostream>
#include <vector>
#include "compatibility.h"
#include <fstream>
#include <string>


struct FSFile {
    const std::string path;
    const std::string code;
    FSFile(const std::string path,const std::string code) :path(path),code(code){};
    const std::string get_line(int line) const; 
};
class SourceManager {
public:
    std::vector<FSFile> sources;
    SourceManager()=default;
    void open(const std::string& path);
    //void open_(std::experimental::filesystem::path path);
};

