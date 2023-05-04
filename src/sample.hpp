#ifndef __MALEXANDRIA_SAMPLE_HPP
#define __MALEXANDRIA_SAMPLE_HPP

#include <algorithm>
#include <cstdint>
#include <cstddef>
#include <format>
#include <fstream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include <zip.h>

#include "config.hpp"
#include "db.hpp"
#include "fs.hpp"
#include "hash.hpp"
#include "logger.hpp"
#include "utility.hpp"
#include "zip.hpp"

namespace malexandria
{
   class Tag
   {
      std::optional<std::int64_t> _row_id;
      std::string _tag;

   public:
      Tag() {}
      Tag(const std::string tag) { this->load_tag(tag); }
      Tag(std::int64_t id) { this->load_id(id); }
      Tag(const Tag &other) : _row_id(other._row_id), _tag(other._tag) {}

      Tag &operator=(const Tag &other) {
         this->_row_id = other._row_id;
         this->_tag = other._tag;

         return *this;
      }

      bool operator==(const Tag &other) const {
         return !(*this < other) && !(other < *this);
      }

      bool operator<(const Tag &other) const {
         if (this->_row_id == other._row_id && this->_tag == other._tag)
            return false;
         
         std::int64_t this_row_id, other_row_id;

         this_row_id = (this->_row_id.has_value()) ? *this->_row_id : 0;
         other_row_id = (other._row_id.has_value()) ? *other._row_id : 0;

         if (this_row_id != other_row_id)
            return this_row_id < other_row_id;

         return this->_tag < other._tag;
      }

      const std::string &operator*(void) const { return this->tag(); }

      static std::optional<std::int64_t> IDFromTag(const std::string &tag);

      bool is_saved(void) const { return this->_row_id.has_value(); }
      std::int64_t row_id(void) const {
         if (!this->is_saved())
            throw exception::TagNotSaved();

         return *this->_row_id;
      }
      const std::string &tag(void) const { return this->_tag; }
      void set_tag(const std::string &tag) { this->_tag = tag; }

      void load_tag(const std::string &tag) {
         this->_row_id = Tag::IDFromTag(tag); // it's okay if this fails, we can save it later.
         this->set_tag(tag);
      }
      void load_id(std::int64_t id);
      void save(void);

      bool has_samples() const;
      void erase();
   };

   class Family
   {
      std::optional<std::int64_t> _row_id;
      std::string _family;

   public:
      Family() {}
      Family(const std::string family) { this->load_family(family); }
      Family(std::int64_t id) { this->load_id(id); }
      Family(const Family &other) : _row_id(other._row_id), _family(other._family) {}

      Family &operator=(const Family &other) {
         this->_row_id = other._row_id;
         this->_family = other._family;

         return *this;
      }

      bool operator==(const Family &other) const {
         return !(*this < other) && !(other < *this);
      }

      bool operator<(const Family &other) const {
         if (this->_row_id == other._row_id && this->_family == other._family)
            return false;
         
         std::int64_t this_row_id, other_row_id;

         this_row_id = (this->_row_id.has_value()) ? *this->_row_id : 0;
         other_row_id = (other._row_id.has_value()) ? *other._row_id : 0;

         if (this_row_id != other_row_id)
            return this_row_id < other_row_id;

         return this->_family < other._family;
      }

      const std::string &operator*(void) const { return this->family(); }

      static std::optional<std::int64_t> IDFromFamily(const std::string &family);

      bool is_saved(void) const { return this->_row_id.has_value(); }
      std::int64_t row_id(void) const {
         if (!this->is_saved())
            throw exception::FamilyNotSaved();

         return *this->_row_id;
      }
      const std::string &family(void) const { return this->_family; }
      void set_family(const std::string &family) { this->_family = family; }

      void load_family(const std::string &family) {
         this->_row_id = Family::IDFromFamily(family); // it's okay if this fails, we can save it later.
         this->set_family(family);
      }
      void load_id(std::int64_t id);
      void save(void);

      bool has_samples() const;
      void erase();
   };

   class Sample
   {
      std::optional<std::int64_t> _row_id;
      std::string _filename;
      std::vector<std::uint8_t> _md5, _sha1, _sha256, _sha3_384;
      std::int64_t _created, _modified;
      std::optional<std::filesystem::path> _loaded_file;
      std::optional<std::string> _alias;
      std::vector<Tag> _tags;
      std::vector<Family> _families;
      std::optional<std::vector<std::uint8_t>> _data;

      void save_to_disk();
      void save_to_database();

      void load_alias();
      void save_alias();

      std::vector<Tag> current_tags();
      void load_tags();
      bool has_tag_in_database(const Tag &tag) const;
      void add_tag_to_database(const Tag &tag) const;
      void remove_tag_from_database(const Tag &tag) const;
      void save_tags();

      std::vector<Family> current_families();
      void load_families();
      bool has_family_in_database(const Family &family) const;
      void add_family_to_database(const Family &family) const;
      void remove_family_from_database(const Family &family) const;
      void save_families();

   public:
      Sample() : _created(0), _modified(0) {}
      Sample(const std::filesystem::path &file) : _created(0), _modified(0) { this->load_file(file); }
      Sample(const std::vector<std::uint8_t> &hash) : _created(0), _modified(0) { this->load_hash(hash); }
      Sample(std::int64_t id) : _created(0), _modified(0) { this->load_id(id); }
      Sample(const Sample &other) : _row_id(other._row_id),
                                    _filename(other._filename),
                                    _md5(other._md5),
                                    _sha1(other._sha1),
                                    _sha256(other._sha256),
                                    _sha3_384(other._sha3_384),
                                    _created(other._created),
                                    _modified(other._modified),
                                    _loaded_file(other._loaded_file),
                                    _alias(other._alias),
                                    _tags(other._tags),
                                    _families(other._families),
                                    _data(other._data)
      {}

      Sample &operator=(const Sample &other) {
         this->_row_id = other._row_id;
         this->_filename = other._filename;
         this->_md5 = other._md5;
         this->_sha1 = other._sha1;
         this->_sha256 = other._sha256;
         this->_sha3_384 = other._sha3_384;
         this->_created = other._created;
         this->_modified = other._modified;
         this->_loaded_file = other._loaded_file;
         this->_alias = other._alias;
         this->_tags = other._tags;
         this->_families = other._families;
         this->_data = other._data;

         return *this;
      }

      static std::optional<std::int64_t> IDFromHash(const std::vector<std::uint8_t> &hash);
      static std::optional<std::int64_t> IDFromAlias(const std::string &alias);
      
      static bool Exists(const std::vector<std::uint8_t> &hash);
      static bool Exists(const std::string &alias);
      static bool Exists(std::int64_t id);
      
      static Sample ByHash(const std::vector<std::uint8_t> &hash);
      static Sample ByAlias(const std::string &alias);
      static Sample ByID(std::int64_t id);
      static Sample FromFile(const std::filesystem::path &path);
      static Sample FromIdentifier(const std::string &string);
      static Sample FromData(const std::vector<std::uint8_t> &vec);

      static std::filesystem::path Export(const std::vector<Sample> &samples, std::optional<std::filesystem::path> filename=std::nullopt);
      static std::vector<Sample> Import(const std::filesystem::path &file, std::optional<std::string> password=std::nullopt);

      bool is_saved(void) const { return this->_row_id.has_value(); }
      std::int64_t row_id(void) const {
         if (!this->is_saved())
            throw exception::SampleNotSaved();

         return *this->_row_id;
      }
      
      const std::string &filename(void) const { return this->_filename; }
      void set_filename(const std::string &filename) { this->_filename = filename; }
      
      const std::vector<std::uint8_t> &md5(void) const { return this->_md5; }
      const std::vector<std::uint8_t> &sha1(void) const { return this->_sha1; }
      const std::vector<std::uint8_t> &sha256(void) const { return this->_sha256; }
      const std::vector<std::uint8_t> &sha3_384(void) const { return this->_sha3_384; }

      bool has_alias(void) const { return this->_alias.has_value(); }
      const std::string &alias(void) const {
         if (!this->has_alias())
            throw exception::NoAlias();

         return *this->_alias;
      }
      void set_alias(const std::string &alias) { this->_alias = alias; }
      void remove_alias(void);

      void load_file(const std::filesystem::path &path);
      void load_data(const std::vector<std::uint8_t> &data);
      void load_hash(const std::vector<std::uint8_t> &hash);
      void load_id(std::int64_t id);

      void save(void);
      void erase(void);

      const std::vector<Tag> &tags(void) const { return this->_tags; }
      bool has_tag(const Tag &tag) const;
      void add_tag(const Tag &tag);
      void remove_tag(const Tag &tag);
      void remove_tags();

      const std::vector<Family> &families(void) const { return this->_families; }
      bool has_family(const Family &family) const;
      void add_family(const Family &family);
      void remove_family(const Family &family);
      void remove_families();

      std::vector<Sample> parents(void) const;
      bool is_parent_of(const Sample &sample) const;
      void add_parent(const Sample &sample);
      void remove_parent(const Sample &sample);
      void remove_parents();
      
      std::vector<Sample> children(void) const;
      bool is_child_of(const Sample &sample) const;
      void add_child(const Sample &sample);
      void remove_child(const Sample &sample);
      void remove_children();

      std::string label() const;
      std::filesystem::path vault_path() const;
      std::filesystem::path alias_path() const;
      std::filesystem::path active_file() const;
      std::filesystem::path archive_file() const;
      std::filesystem::path analysis_file() const;
      std::filesystem::path analysis_metadata_file() const;
      std::filesystem::path analysis_path() const;

      Zip archive(int zip_flags) const;
      std::vector<std::uint8_t> extract_to_memory() const;
      void extract_to_disk(std::optional<std::filesystem::path> filename=std::nullopt) const;

      bool is_active() const;
      void activate() const;
      void deactivate() const;
   };
}

#endif
