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

std::pair<bool,std::error_code> malexandria::erase_file(const std::filesystem::path &path) {
   std::error_code ec;
   bool result = std::filesystem::remove(path, ec);
   return std::make_pair(result, ec);
}

bool malexandria::directory_is_empty(const std::filesystem::path &path) {
   return path_exists(path) && std::filesystem::directory_iterator{path} == std::filesystem::directory_iterator{};
}

std::pair<bool,std::error_code> malexandria::clear_directory(const std::filesystem::path &root, const std::filesystem::path &path) {
   auto root_normal = std::filesystem::path(dos_to_unix_path(root.string()));
   auto path_normal = std::filesystem::path(dos_to_unix_path(path.string()));
   auto target = path_normal;
   std::pair<bool,std::error_code> erase_result = {true, {}};

   while (erase_result.first && target != root_normal && directory_is_empty(target))
   {
      erase_result = erase_file(target);
      target = target.parent_path();
   }

   return erase_result;
}

std::string malexandria::dos_to_unix_path(const std::string &path) {
   auto result = path;
   std::replace(result.begin(), result.end(), '\\', '/');

   return result;
}

bool malexandria::is_rooted_in(const std::filesystem::path &root, const std::filesystem::path &path) {
   auto root_normal = std::filesystem::path(dos_to_unix_path(root.string()));
   auto path_normal = std::filesystem::path(dos_to_unix_path(path.string()));
   auto root_iter = root_normal.begin();
   auto path_iter = path_normal.begin();

   while (root_iter != root_normal.end() && path_iter != path_normal.end())
   {
      if (root_iter->string() != path_iter->string())
         break;

      ++root_iter;
      ++path_iter;
   }

   return root_iter == root_normal.end();
}

std::filesystem::path malexandria::skip_root(const std::filesystem::path &root, const std::filesystem::path &path) {
   if (!is_rooted_in(root, path))
      return path;

   auto root_normal = std::filesystem::path(dos_to_unix_path(root.string()));
   auto path_normal = std::filesystem::path(dos_to_unix_path(path.string()));
   auto root_iter = root_normal.begin();
   auto path_iter = path_normal.begin();
   
   std::filesystem::path result;

   while (root_iter != root_normal.end())
   {
      ++root_iter;
      ++path_iter;
   }

   if (path_iter == path_normal.end())
      return result;

   result = *path_iter;

   while (++path_iter != path_normal.end())
      result /= *path_iter;

   return result;
}

std::optional<std::error_code> malexandria::rename_file(const std::filesystem::path &old_file, const std::filesystem::path &new_file) {
   std::error_code ec;

   std::filesystem::rename(old_file, new_file, ec);

   if (ec.value() == 0)
      return std::nullopt;
   else
      return ec;
}
