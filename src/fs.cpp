#include "fs.hpp"

using namespace malexandria;

bool malexandria::path_exists(const std::filesystem::path &path) {
   return std::filesystem::exists(path);
}

std::filesystem::path malexandria::hash_fanout(const std::string &hash_string) {
   std::filesystem::path result = hash_string.substr(0,1);
   result /= hash_string.substr(1,2);
   result /= hash_string;

   return result;
}

std::filesystem::path malexandria::hash_fanout(const std::vector<std::uint8_t> &hash, bool in_uppercase) {
   return hash_fanout(to_hex_string(hash, in_uppercase));
}

bool malexandria::erase_file(const std::filesystem::path &path) {
   return std::filesystem::remove(path);
}

bool malexandria::directory_is_empty(const std::filesystem::path &path) {
   return path_exists(path) && std::filesystem::directory_iterator{path} == std::filesystem::directory_iterator{};
}

void malexandria::clear_directory(const std::filesystem::path &root, const std::filesystem::path &path) {
   auto target = path;

   while (target != root && directory_is_empty(target))
   {
      erase_file(target);
      target = target.parent_path();
   }
}

std::string malexandria::dos_to_unix_path(const std::string &path) {
   auto result = path;
   std::replace(result.begin(), result.end(), '\\', '/');

   return result;
}
