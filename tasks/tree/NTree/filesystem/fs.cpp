#include "fs.hpp"

#include <iostream>

#include "./files/directory.hpp"

namespace filesystem {

void Fs::EnsureInit() const {
    if (!root_) {
        auto* r = new Directory();
        root_ = r;
        current_ = r;
        r->parent_ = r;
        r->name_.clear();
    }
}

Fs::~Fs() {
    if (root_) {
        DeleteSubtree(root_);
        delete root_;
        root_ = nullptr;
        current_ = nullptr;
    }
}

std::vector<std::string> Fs::Split(const std::string& str, const std::string& splitter) {
    std::vector<std::string> parts;
    std::string cur;
    for (char c : str) {
        if (splitter.find(c) != std::string::npos) {
            if (!cur.empty()) {
                parts.push_back(cur);
                cur.clear();
            }
        } else {
            cur.push_back(c);
        }
    }
    if (!cur.empty()) {
        parts.push_back(cur);
    }
    return parts;
}

std::string Fs::JoinPath(const std::vector<std::string>& chunks) {
    if (chunks.empty()) {
        return "/";
    }
    std::string out;
    for (size_t i = 0; i < chunks.size(); ++i) {
        out.push_back('/');
        out += chunks[i];
    }
    return out;
}

Directory* Fs::WalkTo(Directory* start, const std::vector<std::string>& parts, bool allow_create, bool create_parents) {
    Directory* cur = start;
    for (size_t i = 0; i < parts.size(); ++i) {
        const std::string& p = parts[i];
        if (p == ".") {
            continue;
        }
        if (p == "..") {
            if (cur != root_) {
                cur = cur->parent_ ? cur->parent_ : cur;
            }
            continue;
        }
        if (cur->childs_.Find(p)) {
            Directory* nxt = nullptr;
            for (auto& kv : cur->childs_.Values()) {
                if (kv.first == p) {
                    nxt = kv.second;
                    break;
                }
            }
            if (!nxt) {
                throw exceptions::FileNotFoundException(p);
            }
            cur = nxt;
        } else {
            if (allow_create && create_parents) {
                auto* nd = new Directory();
                nd->parent_ = cur;
                nd->name_ = p;
                cur->childs_.Insert({p, nd});
                cur = nd;
            } else if (allow_create && i + 1 == parts.size()) {
                auto* nd = new Directory();
                nd->parent_ = cur;
                nd->name_ = p;
                cur->childs_.Insert({p, nd});
                cur = nd;
            } else {
                throw exceptions::FileNotFoundException(p);
            }
        }
    }
    return cur;
}

Directory* Fs::ResolveDir(const std::string& path, bool allow_create, bool create_parents) {
    EnsureInit();
    if (path.empty() || path == ".") {
        return current_;
    }
    Directory* start = current_;
    if (!path.empty() && path[0] == '/') {
        start = root_;
    }
    auto parts = Split(path, "/\\");
    return WalkTo(start, parts, allow_create, create_parents);
}

Directory* Fs::ResolveDirConst(const std::string& path) const {
    EnsureInit();
    if (path.empty() || path == ".") {
        return current_;
    }
    Directory* curdir = (path[0] == '/') ? root_ : current_;
    auto parts = Split(path, "/\\");
    for (const auto& p : parts) {
        if (p == ".") {
            continue;
        }
        if (p == "..") {
            if (curdir != root_) {
                curdir = curdir->parent_;
            }
            continue;
        }
        if (curdir->childs_.Find(p)) {
            Directory* nxt = nullptr;
            for (auto& kv : curdir->childs_.Values()) {
                if (kv.first == p) {
                    nxt = kv.second;
                    break;
                }
            }
            if (!nxt) {
                throw exceptions::FileNotFoundException(p);
            }
            curdir = nxt;
        } else {
            if (curdir->files_.Find(p)) {
                throw exceptions::FileNotFoundException("Not a directory: " + p);
            }
            throw exceptions::FileNotFoundException(p);
        }
    }
    return curdir;
}

std::string Fs::BuildCwd(const Directory* cur) {
    std::vector<std::string> chunks;
    const Directory* x = cur;
    while (x && x != x->parent_) {
        if (!x->name_.empty()) {
            chunks.push_back(x->name_);
        }
        x = x->parent_;
    }
    std::vector<std::string> rev(chunks.rbegin(), chunks.rend());
    return JoinPath(rev);
}

void Fs::ChangeDir(const std::string& path) {
    EnsureInit();
    if (path == "/") {
        current_ = root_;
        return;
    }
    if (path.empty() || path == ".") {
        return;
    }
    Directory* start = (path[0] == '/') ? root_ : current_;
    auto parts = Split(path, "/\\");
    Directory* curdir = start;
    for (size_t i = 0; i < parts.size(); ++i) {
        const std::string& p = parts[i];
        if (p == ".") {
            continue;
        }
        if (p == "..") {
            if (curdir != root_) {
                curdir = curdir->parent_;
            }
            continue;
        }
        if (curdir->childs_.Find(p)) {
            Directory* nxt = nullptr;
            for (auto& kv : curdir->childs_.Values()) {
                if (kv.first == p) {
                    nxt = kv.second;
                    break;
                }
            }
            if (!nxt) {
                throw exceptions::FileNotFoundException(p);
            }
            curdir = nxt;
        } else {
            if (curdir->files_.Find(p)) {
                throw exceptions::FileNotFoundException("Not a directory: " + p);
            }
            throw exceptions::FileNotFoundException(p);
        }
    }
    current_ = curdir;
}

void Fs::PWD() const {
    EnsureInit();
    std::cout << BuildCwd(current_) << std::endl;
}

void Fs::ListFiles(const std::string& path) const {
    EnsureInit();
    Directory* dir = nullptr;
    if (path == "." || path.empty()) {
        dir = current_;
    } else {
        dir = ResolveDirConst(path);
    }

    auto cd = dir->childs_.Values(true);
    auto cf = dir->files_.Values(true);
    size_t i = 0;
    size_t j = 0;
    while (i < cd.size() || j < cf.size()) {
        bool take_dir = false;
        if (j >= cf.size()) {
            take_dir = true;
        } else if (i < cd.size() && cd[i].first < cf[j].first) {
            take_dir = true;
        }

        if (take_dir) {
            std::cout << cd[i].first << std::endl;
            ++i;
        } else {
            std::cout << cf[j].first << std::endl;
            ++j;
        }
    }
}

void Fs::MakeDir(const std::string& path, bool is_create_parents) {
    EnsureInit();
    if (path == "/" || path.empty() || path == ".") {
        return;
    }
    bool abs = path[0] == '/';
    auto parts = Split(path, "/\\");
    if (parts.empty()) {
        return;
    }
    Directory* base = abs ? root_ : current_;
    if (is_create_parents) {
        WalkTo(base, parts, true, true);
        return;
    }
    if (parts.size() == 1) {
        const std::string& name = parts[0];
        if (!base->childs_.Find(name)) {
            auto* nd = new Directory();
            nd->parent_ = base;
            nd->name_ = name;
            base->childs_.Insert({name, nd});
        }
        return;
    }
    std::vector<std::string> parent(parts.begin(), parts.end() - 1);
    const std::string& last = parts.back();
    Directory* pdir = WalkTo(base, parent, false, false);
    if (!pdir->childs_.Find(last)) {
        auto* nd = new Directory();
        nd->parent_ = pdir;
        nd->name_ = last;
        pdir->childs_.Insert({last, nd});
    }
}

Directory* Fs::FindChildByName(Directory* d, const std::string& name) {
    for (auto& kv : d->childs_.Values()) {
        if (kv.first == name) {
            return kv.second;
        }
    }
    return nullptr;
}

Directory* Fs::ParentForPath(Fs* fs, const std::string& path, std::string& last) {
    bool abs = !path.empty() && path[0] == '/';
    auto parts = Split(path, "/\\");
    if (parts.empty()) {
        return fs->current_;
    }
    last = parts.back();
    parts.pop_back();
    Directory* base = abs ? fs->root_ : fs->current_;
    return fs->WalkTo(base, parts, false, false);
}

void Fs::CreateFile(const std::string& path, bool is_overwrite) {
    EnsureInit();
    std::string last;
    Directory* pdir = ParentForPath(this, path, last);
    if (last.empty()) {
        throw exceptions::FileNotFoundException(path);
    }
    if (pdir->childs_.Find(last)) {
        throw exceptions::FileNotFoundException("Path is directory: " + last);
    }
    if (pdir->files_.Find(last)) {
        if (is_overwrite) {
            for (auto& kv : pdir->files_.Values()) {
                if (kv.first == last) {
                    kv.second.content_.clear();
                    pdir->files_.Insert({last, kv.second});
                    break;
                }
            }
        }
    } else {
        File f;
        if (is_overwrite) {
            f.content_.clear();
        }
        pdir->files_.Insert({last, f});
    }
}

void Fs::WriteToFile(const std::string& path, bool is_overwrite, std::ostringstream& stream) {
    EnsureInit();
    std::string last;
    Directory* pdir = ParentForPath(this, path, last);
    if (!pdir->files_.Find(last)) {
        throw exceptions::FileNotFoundException(last);
    }
    File cur;
    for (auto& kv : pdir->files_.Values()) {
        if (kv.first == last) {
            cur = kv.second;
            break;
        }
    }
    std::string data = stream.str();
    if (is_overwrite) {
        cur.content_ = data;
    } else {
        cur.content_ += data;
    }
    pdir->files_.Insert({last, cur});
}

void Fs::ShowFileContent(const std::string& path) {
    EnsureInit();
    std::string last;
    Directory* pdir = ParentForPath(this, path, last);
    if (!pdir->files_.Find(last)) {
        throw exceptions::FileNotFoundException(last);
    }
    for (auto& kv : pdir->files_.Values()) {
        if (kv.first == last) {
            std::cout << kv.second.content_;
            return;
        }
    }
    throw exceptions::FileNotFoundException(last);
}

void Fs::DeleteSubtree(Directory* dir) {
    auto child_list = dir->childs_.Values();
    for (auto& kv : child_list) {
        DeleteSubtree(kv.second);
        delete kv.second;
    }
    dir->childs_.Clear();
    dir->files_.Clear();
}

void Fs::RemoveFile(const std::string& path) {
    EnsureInit();
    if (path == "/") {
        DeleteSubtree(root_);
        return;
    }
    std::string last;
    Directory* pdir = ParentForPath(this, path, last);
    if (pdir->files_.Find(last)) {
        pdir->files_.Erase(last);
        return;
    }
    if (pdir->childs_.Find(last)) {
        Directory* node = FindChildByName(pdir, last);
        if (!node) {
            throw exceptions::FileNotFoundException(last);
        }
        DeleteSubtree(node);
        pdir->childs_.Erase(last);
        delete node;
        return;
    }
    throw exceptions::FileNotFoundException(last);
}

void Fs::DfsPrintMatches(Directory* dir, const std::string& base, const std::string& name, bool& printed) {
    for (auto& kv : dir->files_.Values(true)) {
        if (kv.first == name) {
            std::string p = base.empty() ? std::string("/") + name : base + "/" + name;
            std::cout << p << std::endl;
            printed = true;
        }
    }
    for (auto& kv : dir->childs_.Values(true)) {
        std::string next_base = base;
        next_base += "/";
        next_base += kv.first;
        DfsPrintMatches(kv.second, next_base, name, printed);
    }
}

void Fs::FindFile(const std::string& filename) {
    EnsureInit();
    bool printed = false;
    DfsPrintMatches(root_, "", filename, printed);
    if (!printed) {
        throw exceptions::FileNotFoundException(filename);
    }
    std::cout << std::endl;
}

}  // namespace filesystem
