#ifndef __MALEXANDRIA_FS_HPP
#define __MALEXANDRIA_FS_HPP

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "utility.hpp"

namespace malexandria
{
   bool path_exists(const std::filesystem::path &path);
   std::filesystem::path hash_fanout(const std::string &hash_string);
   std::filesystem::path hash_fanout(const std::vector<std::uint8_t> &hash, bool in_uppercase=false);
   bool erase_file(const std::filesystem::path &path);
   bool directory_is_empty(const std::filesystem::path &path);
   void clear_directory(const std::filesystem::path &root, const std::filesystem::path &path);
}

#endif
