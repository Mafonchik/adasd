#pragma once

#include <vector>

#include "../map/map.hpp"
#include "file.hpp"

namespace filesystem {

class Directory {
    friend class Fs;

    void GetName() const;

private:
    explicit Directory(Directory* parent = nullptr);
    ~Directory();

    Directory* parent_;
    Map<std::string, Directory*> childs_;
    Map<std::string, File> files_;
};

}  // namespace filesystem
