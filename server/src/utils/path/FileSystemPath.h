/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2021. All rights reserved.
 */

#ifndef UNITTESTBOT_FILESYSTEMPATH_H
#define UNITTESTBOT_FILESYSTEMPATH_H

#include <filesystem>
#include <vector>

namespace fs {

    using file_time_type = std::filesystem::file_time_type;
    using filesystem_error = std::filesystem::filesystem_error;
    using copy_options = std::filesystem::copy_options;
    using perms = std::filesystem::perms;

    class path {
    public:
        path(const std::filesystem::path& p) : path_(normalizedTrimmed(p)) {
        }

        path() {}

        path(const std::string& s) : path_(normalizedTrimmed(s)) {
        }

        path(const char *s) : path_(normalizedTrimmed(s)) {

        }

        path root_path() const {
            return path(path_.root_path());
        }

        bool has_extension() const {
            return path_.has_extension();
        }

        path filename() const {
            return path(path_.filename());
        }

        path parent_path() const {
            return path(path_.parent_path());
        }

        path extension() const {
            return path(path_.extension());
        }

        path stem() const {
            return path(path_.stem());
        }

        path lexically_normal() const {
            return path(path_.lexically_normal());
        }

        std::string string() const {
            return path_.string();
        }

        friend path operator/(path a, const path& b);

        operator std::string() const {
            return path_.string();
        }

        path& operator/=(const path& p) {
            *this = path(path_ /= p.path_);
            return *this;
        }

        path &replace_filename( const path& replacement ) {
            path_.replace_filename(replacement.path_);
            return *this;
        }

        path &replace_extension( const path& replacement ) {
            path_.replace_extension(replacement.path_);
            return *this;
        }

        bool has_filename() const {
            return path_.has_filename();
        }

        const char * c_str() const {
            return path_.c_str();
        }

        bool empty() const noexcept {
            return path_.empty();
        }

        class iterator {
            std::filesystem::path::iterator iter_;
            std::filesystem::path::iterator end_;
            mutable std::vector<path> entries_cache;
        public:
            using difference_type	= std::ptrdiff_t;
            using value_type		= path;
            using reference		= const path&;
            using pointer		= const path*;
            using iterator_category	= std::bidirectional_iterator_tag;

            explicit iterator(std::filesystem::path::iterator iter, std::filesystem::path::iterator end) : iter_(iter), end_(end) {
                if (iter_ != end_) {
                    entries_cache.emplace_back(*iter_);
                }
            }

            bool operator==(const iterator& other) const {
                return iter_ == other.iter_;
            }

            bool operator!=(const iterator& other) const {
                return iter_ != other.iter_;
            }

            iterator& operator++() {
                ++iter_;
                if (iter_ != end_) {
                    entries_cache.emplace_back(*iter_);
                }
                return *this;
            }

            reference operator*() const {
                return entries_cache.back();
            }

            pointer operator->() const {
                return &entries_cache.back();
            }
        };

        iterator begin() const {
            return iterator(path_.begin(), path_.end());
        }
        iterator end() const {
            return iterator(path_.end(), path_.end());
        }


        friend bool operator == (const path &a, const path &b);
        friend bool operator != (const path &a, const path &b);
        friend bool operator<( const path& lhs, const path& rhs ) noexcept;

        friend bool remove( const path& p );
        friend std::uintmax_t remove_all( const path& p );
        friend bool create_directories( const path& p );
        friend void copy( const path& from, const path& to);
        friend void copy( const path& from, const path& to, std::filesystem::copy_options options );
        friend bool copy_file( const path& from,
                               const path& to,
                               copy_options options );
        friend bool exists( const path& p );
        friend bool is_empty( const path& p );
        friend bool is_directory( const path& p );
        friend path relative( const path& p, const path& base);
        friend path canonical( const path& p );
        friend path weakly_canonical( const path& p );
        friend path absolute(const path& p);
        friend void permissions( const path& p,
                                 std::filesystem::perms prms,
                                 std::filesystem::perm_options opts);
        friend void last_write_time(const path& p,
                                    file_time_type new_time);
        friend std::filesystem::file_time_type last_write_time(const path& p);
        friend std::size_t hash_value( const path& p ) noexcept;

        template< class CharT, class Traits >
        friend std::basic_ostream<CharT,Traits>&
        operator<<( std::basic_ostream<CharT,Traits>& os, const path& p );

        friend class recursive_directory_iterator;
        friend class directory_iterator;

    private:
        std::filesystem::path path_;

        std::filesystem::path normalizedTrimmed(const std::filesystem::path &p) {
            auto r = p.lexically_normal();
            if (r.has_filename()) {
                return r;
            }
            return r.parent_path();
        }
    };

    inline bool remove( const path& p ) {
        return remove(p.path_);
    }

    inline void permissions( const path& p,
                             std::filesystem::perms prms,
                             std::filesystem::perm_options opts = std::filesystem::perm_options::replace ) {
        permissions(p.path_, prms, opts);
    }

    template< class CharT, class Traits >
    inline std::basic_ostream<CharT,Traits>&
    operator<<( std::basic_ostream<CharT,Traits>& os, const path& p ) {
        os << p.path_;
        return os;
    }

    inline path current_path() {
        return path(std::filesystem::current_path());
    }

    inline bool is_empty( const path& p ) {
        return is_empty(p.path_);
    }

    inline void copy( const path& from,
                      const path& to) {
        copy(from.path_, to.path_);
    }

    inline void copy( const path& from,
                      const path& to, std::filesystem::copy_options options ) {
        copy(from.path_, to.path_, options);
    }

    inline std::filesystem::file_time_type last_write_time(const path& p) {
        return last_write_time(p.path_);
    }

    inline bool operator<( const path& lhs, const path& rhs ) noexcept {
        return lhs.path_ < rhs.path_;
    }

    inline std::uintmax_t remove_all( const path& p ) {
        return remove_all(p.path_);
    }

    inline bool copy_file( const path& from,
                           const path& to,
                           copy_options options ) {
        return copy_file(from.path_, to.path_, options);
    }

    inline void last_write_time(const path& p,
                                file_time_type new_time) {
        last_write_time(p.path_, new_time);
    }

    inline std::size_t hash_value( const path& p ) noexcept {
        return hash_value(p.path_);
    }

    inline bool exists( const path& p ) {
        return exists(p.path_);
    }

    inline bool is_directory( const path& p ) {
        return is_directory(p.path_);
    }

    inline path relative( const path& p, const path& base) {
        return path(relative(p.path_, base.path_));
    }

    inline bool create_directories( const path& p ) {
        return create_directories(p.path_);
    }

    inline path operator/(path a, const path& b) {
        return a /= b;
    }

    inline bool operator == (const path &a, const path &b) {
        return a.path_ == b.path_;
    }

    inline bool operator != (const path &a, const path &b) {
        return a.path_ != b.path_;
    }

    inline path canonical( const path& p ) {
        return path(canonical(p.path_));
    }

    inline path weakly_canonical( const path& p ) {
        return path(weakly_canonical(p.path_));
    }

    inline path absolute(const path& p) {
        return path(absolute(p.path_));
    }

    class directory_entry {
        fs::path path_;
        std::filesystem::directory_entry entry_;
    public:
        explicit directory_entry(std::filesystem::directory_entry entry_) : path_(entry_.path()), entry_(std::move(entry_))  {}

        bool is_regular_file() const {
            return entry_.is_regular_file();
        }

        const path& path() const noexcept {
            return path_;
        }
    };

    class recursive_directory_iterator {
        std::filesystem::recursive_directory_iterator iter_;
        mutable std::vector<std::filesystem::recursive_directory_iterator> entries_cache;
    public:
        typedef directory_entry value_type;
        typedef ptrdiff_t difference_type;
        typedef const directory_entry *pointer;
        typedef const directory_entry &reference;
        typedef std::input_iterator_tag iterator_category;

        recursive_directory_iterator(const fs::path &directory) : iter_(directory.path_) {
            entries_cache.emplace_back(iter_);
        }

        recursive_directory_iterator() = default;

        bool operator==(const recursive_directory_iterator &other) const {
            return iter_ == other.iter_;
        }

        bool operator!=(const recursive_directory_iterator &other) const {
            return iter_ != other.iter_;
        }

        recursive_directory_iterator& operator++() {
            ++iter_;
            entries_cache.emplace_back(iter_);
            return *this;
        }

        directory_entry operator*() const {
            return directory_entry(*entries_cache.back());
        }
    };

    inline recursive_directory_iterator begin( recursive_directory_iterator iter ) noexcept {
        return iter;
    }
    inline recursive_directory_iterator end( const recursive_directory_iterator& ) noexcept {
        return recursive_directory_iterator();
    }

    class directory_iterator {
        std::filesystem::directory_iterator iter_;
        mutable std::vector<std::filesystem::directory_iterator> entries_cache;
    public:
        typedef directory_entry        value_type;
        typedef ptrdiff_t              difference_type;
        typedef const directory_entry* pointer;
        typedef const directory_entry& reference;
        typedef std::input_iterator_tag     iterator_category;

        directory_iterator (const fs::path &directory) : iter_(directory.path_) {
            entries_cache.emplace_back(iter_);
        }
        directory_iterator () = default;

        bool operator==(const directory_iterator& other) const {
            return iter_ == other.iter_;
        }

        bool operator!=(const directory_iterator& other) const {
            return iter_ != other.iter_;
        }

        directory_iterator& operator++() {
            ++iter_;
            entries_cache.emplace_back(iter_);
            return *this;
        }

        directory_entry operator*() const {
            return directory_entry(*entries_cache.back());
        }
    };

    inline directory_iterator begin( directory_iterator iter ) noexcept {
        return iter;
    }
    inline directory_iterator end( const directory_iterator& ) noexcept {
        return directory_iterator();
    }
}

#endif //UNITTESTBOT_FILESYSTEMPATH_H
