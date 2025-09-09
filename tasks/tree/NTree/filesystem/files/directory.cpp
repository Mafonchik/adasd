#include "directory.hpp"

#include <iostream>

namespace filesystem {

Directory::Directory(Directory* parent) : parent_(parent) {
}

Directory::~Directory() {
    for (auto& [name, dir] : childs_.Values()) {
        delete dir;
    }
}

void Directory::GetName() const {
    if (parent_) {
        for (const auto& [name, dir] : parent_->childs_.Values()) {
            if (dir == this) {
                std::cout << name << std::endl;
                return;
            }
        }
    }
    std::cout << "(root)" << std::endl;
}

}  // namespace filesystem
