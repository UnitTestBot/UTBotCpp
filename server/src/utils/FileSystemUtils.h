#ifndef UNITTESTBOT_FILESYSTEMUTILS_H
#define UNITTESTBOT_FILESYSTEMUTILS_H

#include "utils/path/FileSystemPath.h"
#include <string_view>
#include <vector>

namespace FileSystemUtils {
    void writeToFile(fs::path const &path, std::string_view text);

    void removeAll(const fs::path &path);

    void copyFile(const fs::path& from, const fs::path& to);

    std::vector<fs::path> recursiveDirectories(const fs::path &root);

    std::string read(const fs::path &path);

    class DirectoryIterator : public fs::directory_iterator {
        fs::path directory;

    public:
        explicit DirectoryIterator(const fs::path &directory);

        [[nodiscard]] size_t size() const;
    };

    fs::directory_iterator cbegin(DirectoryIterator const& iterator);

    fs::directory_iterator cend(DirectoryIterator const& iterator);


    class RecursiveDirectoryIterator : public fs::recursive_directory_iterator {
    private:
        fs::path directory;

    public:
        explicit RecursiveDirectoryIterator(const fs::path &directory);

        [[nodiscard]] size_t size() const;
    };
}

#endif //UNITTESTBOT_FILESYSTEMUTILS_H
