#ifndef __MALEXANDRIA_ANALYSIS_HPP
#define __MALEXANDRIA_ANALYSIS_HPP

#include <uuid.h>

#include "sample.hpp"

namespace malexandria
{
   class Analysis
   {
   public:
      class Config : public malexandria::Config
      {
      public:
         using FileLink = std::optional<std::pair<std::string,std::string>>;
         using FileMap = std::map<std::string,std::pair<std::optional<std::string>,FileLink>>;
         
         Config() : malexandria::Config() {}
         Config(const json &data) : malexandria::Config(data) {}
         Config(const std::filesystem::path &filename) : malexandria::Config(filename) {}
         Config(const Config &other) : malexandria::Config(other) {}

         std::string analysis_id(void) const;
         void set_analysis_id(const std::string &id);

         std::optional<std::string> analysis_alias(void) const;
         void set_analysis_alias(std::optional<std::string> alias);

         std::map<std::string,std::optional<std::string>> samples(void) const;
         bool has_sample(const std::string &hash) const;
         void add_sample(const std::string &hash, std::optional<std::string> active_file=std::nullopt);
         std::optional<std::string> get_sample_active_file(const std::string &hash) const;
         void set_sample_active_file(const std::string &hash, std::optional<std::string> active_file=std::nullopt);
         void remove_sample(const std::string &hash);

         FileMap files(void) const;
         bool has_file(const std::string &filename) const;
         void add_file(const std::string &filename,
                       std::optional<std::string> artifact=std::nullopt,
                       FileLink link=std::nullopt);
         void remove_file(const std::string &filename);
         std::optional<std::string> get_file_artifact(const std::string &filename) const;
         void set_file_artifact(const std::string &filename, std::optional<std::string> artifact);
         FileLink get_file_link(const std::string &filename) const;
         void set_file_link(const std::string &filename, FileLink link);

         std::map<std::string,std::vector<std::string>> dependencies(void) const;
         bool has_dependencies(const std::string &filename) const;
         bool has_dependency(const std::string &filename, const std::string &id) const;
         std::vector<std::string> get_dependencies(const std::string &filename) const;
         void add_dependency(const std::string &filename, const std::string &id);
         void remove_dependency(const std::string &filename, const std::string &id);
      };

      enum class FileState : std::uint8_t
      {
         Untracked = 0,
         New,
         Clean,
         Tainted,
         Deleted
      };
        
   private:
      Config _config;
      uuids::uuid _id;
      std::optional<std::filesystem::path> _open_config;
      std::optional<std::int64_t> _row_id;
      std::optional<Zip> _archive;
      std::optional<std::string> _alias;
      std::map<std::vector<std::uint8_t>, Sample> _samples;

   public:
      Analysis() {}
      explicit Analysis(const uuids::uuid &id) { this->load_from_uuid(id); }
      explicit Analysis(const std::string &alias) { this->load_from_alias(alias); }
      explicit Analysis(const std::filesystem::path &filename) { this->load_from_config(filename); }
      Analysis(const Analysis &other)
         : _config(other._config),
           _id(other._id),
           _open_config(other._open_config),
           _row_id(other._row_id),
           _archive(other._archive),
           _alias(other._alias),
           _samples(other._samples)
      {}

      Analysis &operator=(const Analysis &other) {
         this->_config = other._config;
         this->_id = other._id;
         this->_open_config = other._open_config;
         this->_row_id = other._row_id;
         this->_archive = other._archive;
         this->_alias = other._alias;
         this->_samples = other._samples;

         return *this;
      }

      static std::optional<std::filesystem::path> FindAnalysis(std::optional<std::filesystem::path> start=std::nullopt);
      static Analysis FromIdentifier(const std::string &ident);
      static std::string FileStateToString(FileState file_state) {
         switch (file_state)
         {
         case FileState::Untracked:
            return "untracked";

         case FileState::New:
            return "new";

         case FileState::Clean:
            return "clean";

         case FileState::Tainted:
            return "tainted";

         case FileState::Deleted:
            return "deleted";
         }
      }
       
      bool has_config(void) const;
      bool has_id(void) const;
      const uuids::uuid &id(void) const;
      const uuids::uuid &generate_id(void);

      bool has_row_id(void) const;
      std::int64_t row_id(void) const;
      
      std::filesystem::path archive_file(std::optional<uuids::uuid> id=std::nullopt) const;
      
      bool is_saved() const;
      bool is_open() const;

      std::filesystem::path disk_to_relative(const std::filesystem::path &filename) const;
      std::filesystem::path relative_to_disk(const std::filesystem::path &filename) const;

      // these functions return the full path to disk, they can be turned into relative
      // files with disk_to_relative
      std::filesystem::path default_path() const;
      std::filesystem::path config_file() const;
      std::filesystem::path config_path() const;
      std::filesystem::path open_path() const;

      // these functions return the relative path to the files
      std::filesystem::path active_file(const Sample &sample) const;
      std::filesystem::path taintable_file(const Sample &sample) const;
      std::map<std::filesystem::path,std::vector<std::uint8_t>> active_map(void) const;
      std::map<std::filesystem::path,std::vector<std::uint8_t>> taint_map(void) const;

      bool has_archive(void) const;
      Zip &archive(int flags=0);
      void close_archive(void);
      void discard_archive(void);

      void load_from_uuid(const uuids::uuid &id);
      void load_from_alias(const std::string &alias);
      void load_from_config(const std::filesystem::path &filename);
      void load_config();
      void save_config();

      bool is_tainted(const Sample &sample) const;
      bool is_tainted(const std::filesystem::path &filename);

      std::vector<std::vector<std::uint8_t>> samples(void) const;
      bool has_sample(const Sample &sample) const;
      Sample &get_sample(const std::vector<std::uint8_t> &hash);
      const Sample &get_sample(const std::vector<std::uint8_t> &hash) const;
      std::optional<std::string> get_sample_active_file(const Sample &sample) const;
      void set_sample_active_file(const Sample &sample, std::optional<std::string> filename);
      void add_sample(const Sample &sample, std::optional<std::filesystem::path> active_file);
      void remove_sample(const Sample &sample);
      void restore_sample(const Sample &sample);
      void restore_taintable_sample(const Sample &sample);
      void extract_taintable_sample(const Sample &sample, std::optional<std::filesystem::path> outfile);
      std::vector<std::uint8_t> extract_taintable_sample(const Sample &sample);
      void link_sample(const Sample &sample, std::optional<std::filesystem::path> outlink=std::nullopt, bool relative=true);
      void unlink_sample(const Sample &sample);
      void rename_active_file(const Sample &sample, std::optional<std::string> filename);

      std::vector<std::filesystem::path> files(void) const;
      std::vector<std::filesystem::path> supplements(void) const;
      std::vector<std::pair<std::filesystem::path,std::vector<std::uint8_t>>> artifacts(void) const;
      std::vector<std::filesystem::path> taintable_samples(void) const;
      std::vector<std::filesystem::path> links(void) const;
      
      bool has_file(const std::filesystem::path &filename) const;
      void add_file(const std::filesystem::path &filename,
                    std::optional<Sample> artifact=std::nullopt,
                    std::optional<std::pair<uuids::uuid,std::filesystem::path>> link=std::nullopt);
      bool is_supplement(const std::filesystem::path &filename) const;
      bool is_artifact(const std::filesystem::path &filename) const;
      bool is_artifact_of(const std::filesystem::path &filename, const Sample &sample) const;
      bool is_sample(const std::filesystem::path &filename) const;
      bool is_linked(const std::filesystem::path &filename) const;
      std::optional<std::vector<std::uint8_t>> get_file_artifact(const std::filesystem::path &filename) const;
      void set_file_artifact(const std::filesystem::path &filename, const Sample &sample);
      std::optional<std::pair<uuids::uuid,std::filesystem::path>> get_file_link(const std::filesystem::path &filename) const;
      void set_file_link(const std::filesystem::path &filename, std::optional<std::pair<uuids::uuid,std::filesystem::path>> link=std::nullopt);
      void link_file(const std::filesystem::path &filename, bool relative=true);
      void unlink_file(const std::filesystem::path &filename);
      void rename_link(const std::filesystem::path &link, const std::filesystem::path &new_link);
      
      void remove_file(const std::filesystem::path &filename);
      void restore_file(const std::filesystem::path &filename);
      void rename_file(const std::filesystem::path &current_filename, const std::filesystem::path &new_filename);
      void extract_file(const std::filesystem::path &filename, std::optional<std::filesystem::path> outfile);
      std::vector<std::uint8_t> extract_file(const std::filesystem::path &filename);

      bool has_alias(void) const;
      std::string &alias(void);
      const std::string &alias(void) const;
      void set_alias(std::optional<std::string> alias);

      std::map<std::filesystem::path,std::vector<uuids::uuid>> dependencies(void) const;
      bool has_dependencies(const std::filesystem::path &filename) const;
      bool has_dependency(const std::filesystem::path &filename, const uuids::uuid &id) const;
      std::vector<uuids::uuid> get_dependencies(const std::filesystem::path &filename) const;
      void add_dependency(const std::filesystem::path &filename, const uuids::uuid &id);
      void remove_dependency(const std::filesystem::path &filename, const uuids::uuid &id);

      std::string label(void) const;
      std::map<std::vector<std::uint8_t>,FileState> sample_state(void) const;
      std::map<std::filesystem::path,FileState> archive_state(void);

      std::filesystem::path create(std::optional<std::filesystem::path> root_directory=std::nullopt);
      void save();
      void close();
      std::filesystem::path open(std::optional<std::filesystem::path> open_path=std::nullopt);
      void erase();
   };
}

#endif
