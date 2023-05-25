#ifndef __MALEXANDRIA_ZIP_HPP
#define __MALEXANDRIA_ZIP_HPP

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

#include <fmt/core.h>
#include <zip.h>

#include "config.hpp"
#include "fs.hpp"
#include "hash.hpp"
#include "logger.hpp"
#include "utility.hpp"

namespace malexandria
{
   class Zip
   {
      using FileID = std::variant<std::string,std::uint64_t>;

      std::shared_ptr<zip_t *> _zip;
      std::optional<std::string> _password;
      std::shared_ptr<zip_source_t> _source;

      std::shared_ptr<zip_file_t> open(FileID id);

   public:
      using FileEntry = std::pair<std::int64_t,std::string>;
      
      Zip() {}
      Zip(std::optional<std::vector<std::uint8_t>> data, int flags=ZIP_CREATE) { this->open(data, flags); }
      Zip(const std::filesystem::path &path, int flags=ZIP_CREATE) { this->open(path, flags); }
      Zip(const Zip &other) : _zip(other._zip), _password(other._password), _source(other._source) {}
      ~Zip() {
         if (this->_zip != nullptr && *this->_zip != nullptr && this->_zip.use_count() == 1) { this->discard(); }
      }

      Zip &operator=(const Zip &other)
      {
         this->_zip = other._zip;
         this->_password = other._password;
         this->_source = other._source;

         return *this;
      }

      zip_t *operator*() { return this->handle(); }
      
      zip_t *handle(void) {
         if (this->_zip == nullptr || *this->_zip == nullptr)
            throw exception::NullPointer();

         return *this->_zip;
      }

      void set_password(const std::string &password);
      void clear_password(void);

      void open(std::optional<std::vector<std::uint8_t>> init_data, int flags=ZIP_CREATE);
      void open(const std::filesystem::path &path, int flags=ZIP_CREATE);
      void close(void);
      void discard(void);
      std::vector<std::uint8_t> read(void);
      void save(const std::filesystem::path &filename);
      
      zip_stat_t stat(FileID id);

      std::int64_t locate(const std::string &filename);
      std::string get_name(std::uint64_t index);

      std::int64_t entries(void);
      std::vector<FileEntry> files(void);
      std::map<std::string,std::vector<FileEntry>> file_tree(void);

      std::vector<std::uint8_t> extract_to_memory(FileID file_or_index);
      void extract_to_disk(FileID file_or_index, const std::filesystem::path &path);

      std::vector<std::pair<std::filesystem::path,std::int64_t>> create_directories(const std::filesystem::path &with_dirs);
      void clear_directories(const std::filesystem::path &filename);
      std::int64_t insert_directory(const std::string &dir);
      std::int64_t insert_file(const std::filesystem::path &filename, const std::string &zip_filename);
      void delete_file(FileID file_or_index);
      void rename_file(FileID file_or_index, std::string filename);
      std::int64_t insert_buffer(const std::uint8_t *ptr, std::size_t size, const std::string &filename);
      std::int64_t insert_buffer(const std::vector<std::uint8_t> &vec, const std::string &filename);
   };}
#endif
