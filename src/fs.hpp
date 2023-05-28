#ifndef __MALEXANDRIA_FS_HPP
#define __MALEXANDRIA_FS_HPP

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <vector>

#include "utility.hpp"

namespace malexandria
{
   bool path_exists(const std::filesystem::path &path);
   std::filesystem::path hash_fanout(const std::string &hash_string);
   std::filesystem::path hash_fanout(const std::vector<std::uint8_t> &hash, bool in_uppercase=false);
   std::pair<bool,std::error_code> erase_file(const std::filesystem::path &path);
   bool directory_is_empty(const std::filesystem::path &path);
   std::pair<bool,std::error_code> clear_directory(const std::filesystem::path &root, const std::filesystem::path &path);
   std::string dos_to_unix_path(const std::string &path);
   bool is_rooted_in(const std::filesystem::path &root, const std::filesystem::path &path);
   std::filesystem::path skip_root(const std::filesystem::path &root, const std::filesystem::path &path);
   std::optional<std::error_code> rename_file(const std::filesystem::path &old_file, const std::filesystem::path &new_file);
   std::vector<std::filesystem::path> list_directory(const std::filesystem::path &root, bool recurse=false);
}

#endif
