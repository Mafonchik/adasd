#pragma once
#include <iostream>
#include <string>

namespace filesystem {

class File {
public:
    std::string content;

    void Read(size_t bytes = 0) const;
    void Write();
    void Append(const std::string& data);
};

}  // namespace filesystem
