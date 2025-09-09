#pragma once

#include <fmt/core.h>

#include <exception>
#include <string>

namespace filesystem::exceptions {

class FileNotFoundException : public std::exception {
public:
    explicit FileNotFoundException(const std::string& msg) : msg_("FileSystemError: " + msg) {
    }

    const char* what() const noexcept override {
        return msg_.c_str();
    }

private:
    std::string msg_;
};

class FileAlreadyExistsException : public std::exception {
public:
    explicit FileAlreadyExistsException(const std::string& msg) : msg_("FileSystemError: " + msg) {
    }

    const char* what() const noexcept override {
        return msg_.c_str();
    }

private:
    std::string msg_;
};

}  // namespace filesystem::exceptions
