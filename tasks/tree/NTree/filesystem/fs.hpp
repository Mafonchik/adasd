#pragma once

#include <sstream>
#include <string>
#include <vector>

#include "./detail/exceptions.hpp"

namespace filesystem {

class Directory;

class Fs {
public:
    void ChangeDir(const std::string& path);
    void PWD() const;
    void RemoveFile(const std::string& path);
    void ListFiles(const std::string& path = ".") const;
    void MakeDir(const std::string& path, bool is_create_parents = false);
    void CreateFile(const std::string& path = ".", bool is_overwrite = false);
    void WriteToFile(const std::string& filename, bool is_overwrite, std::ostringstream& stream);
    void ShowFileContent(const std::string& path);
    void FindFile(const std::string& filename);
    ~Fs();

private:
    static std::vector<std::string> Split(const std::string& str, const std::string& splitter);

    mutable Directory* root_{nullptr};
    mutable Directory* current_{nullptr};

    void EnsureInit() const;
    static std::string JoinPath(const std::vector<std::string>& chunks);
    Directory* ResolveDir(const std::string& path, bool allow_create = false, bool create_parents = false);
    Directory* ResolveDirConst(const std::string& path) const;
    Directory* WalkTo(Directory* start, const std::vector<std::string>& parts, bool allow_create, bool create_parents);
    static void DeleteSubtree(Directory* dir);
    static void CollectFiles(Directory* dir, const std::string& base, const std::string& name,
                             std::vector<std::string>& out);
    static Directory* ParentForPath(Fs* fs, const std::string& path, std::string& last);
    static std::string BuildCwd(const Directory* cur);
    static Directory* FindChildByName(Directory* d, const std::string& name);
    static void DfsPrintMatches(Directory* dir, const std::string& base, const std::string& name, bool& printed);
};

}  // namespace filesystem
