#pragma once

#include <istream>
#include <sstream>
#include <vector>

#include "./detail/exceptions.hpp"
#include "./files/directory.hpp"

namespace filesystem {

class Directory;

class Fs {
public:
    Fs();
    ~Fs();

    void ChangeDir(const std::string& path);
    void PWD() const;
    void RemoveFile(const std::string& path);
    void ListFiles(const std::string& path = ".");
    void MakeDir(const std::string& path, bool is_create_parents = false);
    void CreateFile(const std::string& path, bool is_overwrite = false);
    void WriteToFile(const std::string& path, bool is_overwrite, std::ostream& stream);
    void ShowFileContent(const std::string& path);
    void FindFile(const std::string& filename);

private:
    std::vector<std::string> Split(const std::string& str, const std::string& splitter);
    Directory* NavigatePath(const std::string& path);
    void FindFileRecursive(Directory* dir, const std::string& filename, std::vector<std::string>& paths,
                           std::string current_path);

private:
    Directory* root_;
    Directory* current_;
};

}  // namespace filesystem
