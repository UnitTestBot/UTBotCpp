#include "FileSystemUtils.h"

#include "exceptions/FileSystemException.h"

#include <fstream>
#include <system_error>
#include "Paths.h"

namespace FileSystemUtils {
    void writeToFile(const fs::path &path, std::string_view text) {
        std::error_code e;
        auto parentPath = path.parent_path();
        if (parentPath.empty())
            return;
        try {
            fs::create_directories(parentPath);
        } catch (const fs::filesystem_error &e) {
            LOG_S(ERROR) << "create_directories failed, path: " + parentPath.string()
                         << e.what();
            throw FileSystemException("create_directories failed, path: " + parentPath.string(), e);
        }
        std::ofstream os(path);
        os << text;
        if (!os) {
            std::error_code ec;
            ec.assign(errno, std::system_category());
            auto error = fs::filesystem_error("writing to file failed, file: " + path.string(), ec);
            LOG_S(ERROR) << "writing to file failed, file: " + path.string();
            throw FileSystemException(error);
        }
    }

    void removeAll(const fs::path &path) {
        if (path == fs::current_path().root_path()) {
            std::string message = "Couldn't remove files from root directory.";
            LOG_S(ERROR) << message;
            throw BaseException(message);
        }
        fs::remove_all(path);
    }

    void copyFile(const fs::path &from, const fs::path &to) {
        fs::copy_file(from, to, fs::copy_options::overwrite_existing);
        fs::last_write_time(to, fs::file_time_type::clock::now());
    }

    std::vector<fs::path> recursiveDirectories(const fs::path& root) {
        std::vector<fs::path> directories;
        directories.push_back(Paths::normalizedTrimmed(fs::absolute(root)));
        for(const auto& p: fs::recursive_directory_iterator(root)) {
            if (fs::is_directory(p.path())) {
                directories.push_back(Paths::normalizedTrimmed(fs::absolute(p.path())));
            }
        }
        return directories;
    }

    DirectoryIterator::DirectoryIterator(const fs::path &directory) : fs::directory_iterator(directory) {
        this->directory = directory;
    }

    size_t DirectoryIterator::size() const {
        return std::distance(fs::directory_iterator(directory), fs::directory_iterator());
    }

    fs::directory_iterator cbegin(const DirectoryIterator &iterator) {
        return fs::begin(iterator);
    }
    fs::directory_iterator cend(const DirectoryIterator &iterator) {
        return fs::end(iterator);
    }

    RecursiveDirectoryIterator::RecursiveDirectoryIterator(const fs::path &directory) : fs::recursive_directory_iterator(directory) {
        this->directory = directory;
    }

    size_t RecursiveDirectoryIterator::size() const {
        return std::distance(fs::recursive_directory_iterator(directory), fs::recursive_directory_iterator());
    }
}
