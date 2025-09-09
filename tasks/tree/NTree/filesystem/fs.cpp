#include "fs.hpp"

#include <iostream>
#include <sstream>

namespace filesystem {

Fs::Fs() {
    root_ = new Directory();
    current_ = root_;
}

Fs::~Fs() {
    delete root_;
}

void Fs::ChangeDir(const std::string& path) {
    std::vector<std::string> components = Split(path, "/");
    Directory* dir = (path[0] == '/') ? root_ : current_;

    size_t start_index = (!components.empty() && components[0] == "/") ? 1 : 0;
    for (size_t i = start_index; i < components.size(); ++i) {
        const std::string& component = components[i];
        if (component == ".") {
            continue;
        }
        if (component == "..") {
            if (dir->parent_) {
                dir = dir->parent_;
            }
            continue;
        }
        if (!dir->childs_.Find(component)) {
            throw exceptions::FileNotFoundException(path);
        }
        dir = dir->childs_[component];
    }
    current_ = dir;
}

void Fs::PWD() const {
    if (current_ == root_) {
        std::cout << "/" << std::endl;
        return;
    }

    std::vector<std::string> components;
    Directory* temp = current_;
    while (temp != root_) {
        for (const auto& [name, dir] : temp->parent_->childs_.Values()) {
            if (dir == temp) {
                components.push_back(name);
                break;
            }
        }
        temp = temp->parent_;
    }

    std::string path = "/";
    for (size_t i = components.size(); i-- > 0;) {
        path += components[i];
        if (i > 0) {
            path += "/";
        }
    }
    std::cout << path << std::endl;
}

void Fs::ListFiles(const std::string& path) {
    Directory* dir = (path == "." || path.empty()) ? current_ : nullptr;
    if (dir == nullptr) {
        try {
            dir = NavigatePath(path);
        } catch (const exceptions::FileNotFoundException&) {
            throw;
        }
    }

    auto dirs = dir->childs_.Values();
    auto files = dir->files_.Values();

    for (const auto& [name, _] : dirs) {
        std::cout << name << "/" << std::endl;
    }
    for (const auto& [name, _] : files) {
        std::cout << name << std::endl;
    }
}

void Fs::MakeDir(const std::string& path, bool is_create_parents) {
    if (path.empty()) {
        throw exceptions::FileNotFoundException("empty path");
    }

    std::vector<std::string> components = Split(path, "/");
    Directory* dir = (path[0] == '/') ? root_ : current_;

    size_t start_index = (!components.empty() && components[0] == "/") ? 1 : 0;

    for (size_t i = start_index; i < components.size(); ++i) {
        const std::string& component = components[i];
        if (component == "." || component == ".." || component.empty()) {
            continue;
        }

        if (!dir->childs_.Find(component)) {
            if (!is_create_parents && i < components.size() - 1) {
                throw exceptions::FileNotFoundException(path);
            }
            dir->childs_[component] = new Directory(dir);
        }
        dir = dir->childs_[component];
    }
}

std::vector<std::string> Fs::Split(const std::string& str, const std::string& splitter) {
    std::vector<std::string> result;
    if (str.empty()) {
        return result;
    }

    size_t start = 0;
    while (start < str.size() && str[start] == '/') {
        start++;
    }

    if (start > 0) {
        result.push_back("/");
    }

    size_t end = str.find(splitter, start);
    while (end != std::string::npos) {
        if (end != start) {
            result.push_back(str.substr(start, end - start));
        }
        start = end + 1;
        end = str.find(splitter, start);
    }

    if (start < str.size()) {
        result.push_back(str.substr(start));
    }

    return result;
}

Directory* Fs::NavigatePath(const std::string& path) {
    if (path.empty()) {
        return current_;
    }

    std::vector<std::string> components = Split(path, "/");
    Directory* dir = (path[0] == '/') ? root_ : current_;

    size_t start_index = 0;
    while (start_index < components.size() && components[start_index].empty()) {
        start_index++;
    }

    for (size_t i = start_index; i < components.size(); ++i) {
        const std::string& component = components[i];
        if (component == "." || component.empty()) {
            continue;
        }
        if (component == "..") {
            if (dir->parent_) {
                dir = dir->parent_;
            }
            continue;
        }
        if (!dir->childs_.Find(component)) {
            throw exceptions::FileNotFoundException(path);
        }
        dir = dir->childs_[component];
    }
    return dir;
}

void Fs::RemoveFile(const std::string& path) {
    if (path.empty()) {
        throw exceptions::FileNotFoundException("empty path");
    }

    if (path == "/") {
        delete root_;
        root_ = new Directory();
        current_ = root_;
        return;
    }

    std::vector<std::string> components = Split(path, "/");
    std::string name = components.back();
    components.pop_back();

    std::string dir_path;
    size_t start_index = (!components.empty() && components[0] == "/") ? 1 : 0;
    for (size_t i = start_index; i < components.size(); ++i) {
        dir_path += "/" + components[i];
    }

    Directory* dir = dir_path.empty() ? current_ : NavigatePath(dir_path);

    if (dir->files_.Find(name)) {
        dir->files_.Erase(name);
    } else if (dir->childs_.Find(name)) {
        delete dir->childs_[name];
        dir->childs_.Erase(name);
    } else {
        throw exceptions::FileNotFoundException(path);
    }
}

void Fs::CreateFile(const std::string& path, bool is_overwrite) {
    if (path.empty() || path == "/") {
        throw exceptions::FileNotFoundException(path);
    }

    std::vector<std::string> components = Split(path, "/");
    std::string filename = components.back();
    components.pop_back();

    std::string dir_path;
    if (!components.empty()) {
        dir_path = components[0] == "/" ? "" : "/";
        for (size_t i = (components[0] == "/" ? 1 : 0); i < components.size(); ++i) {
            dir_path += components[i];
            if (i < components.size() - 1) {
                dir_path += "/";
            }
        }
    }

    Directory* dir = dir_path.empty() ? current_ : NavigatePath(dir_path);

    if (dir->files_.Find(filename)) {
        if (!is_overwrite) {
            throw exceptions::FileAlreadyExistsException(path);
        }
        dir->files_.Erase(filename);
    }
    dir->files_[filename] = File();
}

void Fs::WriteToFile(const std::string& path, bool is_overwrite, std::ostream& output_stream) {
    std::vector<std::string> components = Split(path, "/");
    if (components.empty()) {
        throw exceptions::FileNotFoundException(path);
    }

    std::string filename = components.back();
    components.pop_back();

    std::string dir_path;
    size_t start_index = (!components.empty() && components[0] == "/") ? 1 : 0;
    for (size_t i = start_index; i < components.size(); ++i) {
        dir_path += "/" + components[i];
    }

    Directory* dir = dir_path.empty() ? current_ : NavigatePath(dir_path);

    if (!dir->files_.Find(filename)) {
        if (!is_overwrite) {
            throw exceptions::FileNotFoundException(path);
        }
        dir->files_[filename] = File();
    }

    output_stream << dir->files_[filename].content;
}

void Fs::ShowFileContent(const std::string& path) {
    std::vector<std::string> components = Split(path, "/");
    if (components.empty()) {
        throw exceptions::FileNotFoundException(path);
    }

    std::string filename = components.back();
    components.pop_back();

    std::string dir_path;
    size_t start_index = (!components.empty() && components[0] == "/") ? 1 : 0;
    for (size_t i = start_index; i < components.size(); ++i) {
        dir_path += "/" + components[i];
    }

    Directory* dir = dir_path.empty() ? current_ : NavigatePath(dir_path);

    if (!dir->files_.Find(filename)) {
        throw exceptions::FileNotFoundException(path);
    }

    std::cout << dir->files_[filename].content << std::endl;
}

void Fs::FindFileRecursive(Directory* dir, const std::string& filename, std::vector<std::string>& paths,
                           std::string current_path) {
    if (dir->files_.Find(filename)) {
        paths.push_back(current_path + (current_path == "/" ? "" : "/") + filename);
    }

    for (const auto& [name, child] : dir->childs_.Values()) {
        FindFileRecursive(child, filename, paths, current_path + (current_path == "/" ? "" : "/") + name);
    }
}

void Fs::FindFile(const std::string& filename) {
    std::vector<std::string> paths;
    FindFileRecursive(root_, filename, paths, "/");

    if (paths.empty()) {
        throw exceptions::FileNotFoundException(filename);
    }

    for (const auto& path : paths) {
        std::cout << path << std::endl;
    }
}

}  // namespace filesystem
