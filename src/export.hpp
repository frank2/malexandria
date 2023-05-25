#ifndef __MALEXANDRIA_EXPORT_HPP
#define __MALEXANDRIA_EXPORT_HPP

#include "analysis.hpp"
#include "config.hpp"
#include "sample.hpp"
#include "zip.hpp"

namespace malexandria
{
   class Export
   {
   public:
      class Config : public malexandria::Config
      {
      public:
         Config() : malexandria::Config() {
            (*this)["samples"] = nullptr;
            (*this)["analyses"] = nullptr;
         }
         Config(const json &data) : malexandria::Config(data) {}
         Config(std::filesystem::path &filename) : malexandria::Config(filename) {}
         Config(const Config &other) : malexandria::Config(other) {}

         bool has_sample(const std::string &hash) const;
         std::optional<std::vector<std::string>> samples(void) const;
         void load_sample(const Sample &sample);
         void remove_sample(const std::string &hash);
         
         std::string get_export_filename(const std::string &hash) const;
         void set_export_filename(const std::string &hash, const std::string &filename);
         
         std::string get_active_filename(const std::string &hash) const;
         void set_active_filename(const std::string &hash, const std::string &filename);

         std::optional<std::string> get_alias(const std::string &hash) const;
         void set_alias(const std::string &hash, std::optional<std::string> alias);
         
         std::optional<std::vector<std::string>> get_tags(const std::string &hash) const;
         void add_tag(const std::string &hash, const std::string &tag);
         void remove_tag(const std::string &hash, const std::string &tag);
         
         std::optional<std::vector<std::string>> get_families(const std::string &hash) const;
         void add_family(const std::string &hash, const std::string &family);
         void remove_family(const std::string &hash, const std::string &family);
         
         std::optional<std::vector<std::string>> get_parents(const std::string &hash) const;
         void add_parent(const std::string &hash, const std::string &parent_hash);
         void remove_parent(const std::string &hash, const std::string &parent_hash);
         
         std::optional<std::vector<std::string>> get_children(const std::string &hash) const;
         void add_child(const std::string &hash, const std::string &child_hash);
         void remove_child(const std::string &hash, const std::string &child_hash);

         bool has_analysis(const std::string &id) const;
         std::optional<std::vector<std::string>> analyses(void) const;
         void load_analysis(const Analysis &analysis);
         std::string get_analysis_file(const std::string &id) const;
         void remove_analysis(const std::string &id);
      };

   private:
      Config _config;
      Zip _archive;
      std::optional<std::filesystem::path> _filename;

   public:
      Export() {}
      Export(std::optional<std::vector<std::uint8_t>> data, int flags=ZIP_CREATE) { this->open(data, flags); }
      Export(const std::filesystem::path &filename, int flags=ZIP_CREATE) : _filename(filename) { this->open(filename, flags); }
      Export(const Export &other) : _config(other._config), _archive(other._archive), _filename(other._filename) {}

      Export &operator=(const Export &other) {
         this->_config = other._config;
         this->_archive = other._archive;
         this->_filename = other._filename;

         return *this;
      }

      void open(std::optional<std::vector<std::uint8_t>> zip_data, int flags=ZIP_CREATE) {
         this->_archive.open(zip_data, flags);

         if (zip_data.has_value())
            this->load_config();
      }
      void open(const std::filesystem::path &filename, int flags=ZIP_CREATE) {
         if (flags & ZIP_CREATE != 0 && !path_exists(filename) && !path_exists(filename.parent_path()))
            if (!std::filesystem::create_directories(filename.parent_path()))
               throw exception::CreateDirectoryFailure(filename.parent_path().string());

         auto exists = path_exists(filename);
         this->_archive.open(filename, flags);

         if (exists)
            this->load_config();
      }
      void discard(void) { this->_archive.discard(); }
      void close(void) { this->_archive.close(); }
      std::vector<std::uint8_t> read(void) { return this->_archive.read(); }
      void save(const std::filesystem::path &filename) { this->_archive.save(filename); }

      void set_password(const std::string &password) { this->_archive.set_password(password); }
      void load_config(void);
      void save_config(void);

      std::vector<std::vector<std::uint8_t>> samples(void);
      Sample import_sample(const std::vector<std::uint8_t> &hash);
      std::vector<Sample> import_samples(void);
      void add_sample(const Sample &sample);
      void remove_sample(const std::vector<std::uint8_t> &hash);

      std::vector<uuids::uuid> analyses(void);
      Analysis import_analysis(const uuids::uuid &id);
      std::vector<Analysis> import_analyses(void);
      void add_analysis(const Analysis &analysis);
      void remove_analysis(const uuids::uuid &id);
   };
}

#endif
