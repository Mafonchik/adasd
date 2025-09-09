#include "file.hpp"

namespace filesystem {

void File::Read(size_t bytes) const {
    if (bytes == 0 || bytes >= content.size()) {
        std::cout << content;
    } else {
        std::cout << content.substr(0, bytes);
    }
}

void File::Write() {
    content.clear();
}

void File::Append(const std::string& data) {
    content += data;
}

}  // namespace filesystem
