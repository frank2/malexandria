#include "sample.hpp"

using namespace malexandria;

std::optional<std::int64_t> Tag::IDFromTag(const std::string &tag) {
   auto &db = Database::GetInstance();
   auto result = db.query("SELECT id FROM mlx_tags WHERE tag=?", tag);

   if (result.size() == 0)
      return std::nullopt;

   return std::get<std::int64_t>(result[0][0]);
}

void Tag::load_id(std::int64_t id) {
   auto &db = Database::GetInstance();
   auto result = db.query("SELECT tag FROM mlx_tags WHERE id=?", id);

   if (result.size() == 0)
      throw exception::TagIDNotFound(id);

   this->_tag = std::get<std::string>(result[0][0]);
   this->_row_id = id;
}

void Tag::save(void) {
   std::string query;
   auto tag_id = Tag::IDFromTag(this->tag());
   bool exists = tag_id.has_value() || this->_row_id.has_value();

   if (exists)
   {
      if (!this->_row_id.has_value() || *this->_row_id != *tag_id)
         throw exception::TagExists(this->tag());
      
      query = "\
UPDATE\
         mlx_tags \
SET\
         tag=? \
WHERE \
         id=?";
   }
   else {
      query = "\
INSERT INTO\
         mlx_tags \
         (tag) \
VALUES \
         (?)";
   }

   const char *tail;
   auto statement = Statement(query, &tail);
   statement.bind(this->_tag);
   
   if (exists)
      statement.bind(*this->_row_id);

   statement.execute();

   if (!exists)
      this->_row_id = Database::GetInstance().last_insert_rowid();
}

bool Tag::has_samples() const {
   if (!this->is_saved())
      throw exception::TagNotSaved();

   const char *tail;
   auto statement = Statement("SELECT sample_id FROM mlx_sample_tags WHERE tag_id=?", &tail, this->row_id());
   return statement.step().has_value();
}

void Tag::erase() {
   if (!this->is_saved())
      throw exception::TagNotSaved();

   Database::GetInstance().query("DELETE FROM mlx_tags WHERE id=?", this->row_id());
   this->_row_id = std::nullopt;
}

std::optional<std::int64_t> Family::IDFromFamily(const std::string &family) {
   auto &db = Database::GetInstance();
   auto result = db.query("SELECT id FROM mlx_families WHERE family=?", family);

   if (result.size() == 0)
      return std::nullopt;

   return std::get<std::int64_t>(result[0][0]);
}

void Family::load_id(std::int64_t id) {
   auto &db = Database::GetInstance();
   auto result = db.query("SELECT family FROM mlx_families WHERE id=?", id);

   if (result.size() == 0)
      throw exception::FamilyIDNotFound(id);

   this->_family = std::get<std::string>(result[0][0]);
   this->_row_id = id;
}

void Family::save(void) {
   std::string query;
   auto family_id = Family::IDFromFamily(this->family());
   bool exists = family_id.has_value() || this->_row_id.has_value();

   if (exists)
   {
      if (!this->_row_id.has_value() || *this->_row_id != *family_id)
         throw exception::FamilyExists(this->family());
      
      query = "\
UPDATE\
         mlx_families \
SET\
         family=? \
WHERE \
         id=?";
   }
   else {
      query = "\
INSERT INTO\
         mlx_families \
         (family) \
VALUES \
         (?)";
   }

   const char *tail;
   auto statement = Statement(query, &tail, this->_family);

   if (exists)
      statement.bind(*this->_row_id);

   statement.execute();

   if (!exists)
      this->_row_id = Database::GetInstance().last_insert_rowid();
}

bool Family::has_samples() const {
   if (!this->is_saved())
      throw exception::FamilyNotSaved();

   const char *tail;
   auto statement = Statement("SELECT sample_id FROM mlx_sample_families WHERE family_id=?", &tail, this->row_id());
   return statement.step().has_value();
}

void Family::erase() {
   if (!this->is_saved())
      throw exception::FamilyNotSaved();
   
   Database::GetInstance().query("DELETE FROM mlx_families WHERE id=?", this->row_id());
   this->_row_id = std::nullopt;
}

std::optional<std::int64_t> Sample::IDFromHash(const std::vector<std::uint8_t> &hash)
{
   const char *hash_type;

   if (hash.size() == 16)
      hash_type = "md5";
   else if (hash.size() == 20)
      hash_type = "sha1";
   else if (hash.size() == 32)
      hash_type = "sha256";
   else if (hash.size() == 48)
      hash_type = "sha3_384";
   else
      throw exception::UnrecognizedHashType(to_hex_string(hash));

   std::string query = fmt::format("SELECT id FROM mlx_samples WHERE {}=?", hash_type);
   auto &database = Database::GetInstance();
   auto hex_hash = to_hex_string(hash);
   auto result = database.query(query, hex_hash);

   if (result.size() == 0)
      return std::nullopt;

   return std::get<std::int64_t>(result[0][0]);
}

std::optional<std::int64_t> Sample::IDFromAlias(const std::string &alias)
{
   auto result = Database::GetInstance().query("SELECT sample_id FROM mlx_aliases WHERE alias=?", alias);

   if (result.size() == 0)
      return std::nullopt;

   return std::get<std::int64_t>(result[0][0]);
}

bool Sample::Exists(const std::vector<std::uint8_t> &hash) {
   return Sample::IDFromHash(hash).has_value();
}

bool Sample::Exists(const std::string &alias) {
   return Sample::IDFromAlias(alias).has_value();
}

bool Sample::Exists(std::int64_t id) {
   return Database::GetInstance().query("SELECT id FROM mlx_samples WHERE id=?", id).size() > 0;
}

Sample Sample::ByHash(const std::vector<std::uint8_t> &hash) {
   return Sample(hash);
}

Sample Sample::ByAlias(const std::string &alias) {
   auto id = Sample::IDFromAlias(alias);

   if (!id.has_value())
      throw exception::AliasNotFound(alias);

   return Sample(*id);
}

Sample Sample::ByID(std::int64_t id) {
   return Sample(id);
}

Sample Sample::FromFile(const std::filesystem::path &path) {
   return Sample(path);
}

Sample Sample::FromIdentifier(const std::string &str) {
   auto alias_id = Sample::IDFromAlias(str);

   if (!alias_id.has_value() && path_exists(str))
      return Sample::FromFile(str);

   if (alias_id.has_value()) { return Sample::ByID(*alias_id); }

   try {
      return Sample::ByHash(from_hex_string(str));
   }
   catch (exception::Exception &exc)
   {
      throw exception::InvalidIdentifier(str);
   }
}

Sample Sample::FromData(const std::vector<std::uint8_t> &vec) {
   Sample result;
   result.load_data(vec);

   return result;
}

std::filesystem::path Sample::Export(const std::vector<Sample> &samples, std::optional<std::filesystem::path> filename) {
   auto &main_config = MainConfig::GetInstance();
   auto active_path = std::filesystem::path(main_config.active_path());
   
   if (!filename.has_value())
      filename = std::filesystem::path(main_config.active_path()) / std::string("export.mlx");

   if (filename->extension().string() != ".mlx")
      *filename += std::string(".mlx");

   MLX_DEBUGN("export filename: {}", filename->string());

   auto export_archive = Zip(*filename, ZIP_CREATE | ZIP_TRUNCATE);
   export_archive.set_password(main_config.zip_password());
   export_archive.insert_directory("samples");

   auto export_metadata = Config();
   auto benign_extension = main_config.benign_extension();
   std::vector<std::vector<std::uint8_t>> sample_data;

   Logger::InfoN("exporting {} samples.", samples.size());

   for (auto &sample : samples)
   {
      if (!sample.is_saved())
         throw exception::SampleNotSaved();

      Logger::Info("   exporting {}...", sample.label());
      
      auto hash_string = to_hex_string(sample.sha256());
      auto benign_file = hash_string + std::string(".") + benign_extension;
      auto archive_file = std::string("samples/") + benign_file;

      Logger::Raw(Logger::Level::Info, "creating metadata...");
      
      export_metadata[hash_string]["sample_name"] = benign_file;
      export_metadata[hash_string]["filename"] = sample.filename();

      if (sample.has_alias())
         export_metadata[hash_string]["alias"] = sample.alias();
      else
         export_metadata[hash_string]["alias"] = std::string();

      export_metadata[hash_string]["tags"] = std::vector<std::string>();

      for (auto &tag : sample.tags())
         export_metadata[hash_string]["tags"].push_back(*tag);

      export_metadata[hash_string]["families"] = std::vector<std::string>();

      for (auto &family : sample.families())
         export_metadata[hash_string]["families"].push_back(*family);

      export_metadata[hash_string]["parents"] = std::vector<std::string>();
      export_metadata[hash_string]["children"] = std::vector<std::string>();

      Logger::Raw(Logger::Level::Info, "extracting sample to memory...");
      sample_data.push_back(sample.extract_to_memory());

      Logger::Raw(Logger::Level::Info, "adding sample to archive...");
      export_archive.insert_buffer(sample_data.back(), archive_file);

      Logger::Raw(Logger::Level::Info, "done.\n");
      MLX_DEBUGN("metadata: {}", export_metadata[hash_string].dump(4));
   }

   Logger::Info("samples processed, establishing relationships...");

   for (auto &sample : samples)
   {
      auto main_hash = to_hex_string(sample.sha256());
      
      for (auto &child : sample.children())
      {
         auto child_hash = to_hex_string(child.sha256());

         if (export_metadata.data().find(child_hash) != export_metadata.data().end())
            export_metadata[main_hash]["children"].push_back(child_hash);
      }
      
      for (auto &parent : sample.parents())
      {
         auto parent_hash = to_hex_string(parent.sha256());

         if (export_metadata.data().find(parent_hash) != export_metadata.data().end())
            export_metadata[main_hash]["parents"].push_back(parent_hash);
      }
   }

   Logger::Raw(Logger::Level::Info, "done.\n");

   Logger::Info("adding metadata file to archive...");
   auto metadata_string = export_metadata.to_string();
   export_archive.insert_buffer(reinterpret_cast<const std::uint8_t *>(metadata_string.c_str()), metadata_string.size(), "metadata.json");
   Logger::Raw(Logger::Level::Info, "done.\n");
   Logger::Info("saving exported data to {}...", filename->string());
   export_archive.close();
   Logger::Raw(Logger::Level::Info, "done.\n");

   Logger::InfoN("samples exported.");
   return *filename;
}

std::vector<Sample> Sample::Import(const std::filesystem::path &file, std::optional<std::string> password) {
   auto &main_config = MainConfig::GetInstance();

   if (!password.has_value())
      password = main_config.zip_password();

   MLX_DEBUGN("password: {}", *password);
   
   auto import_archive = Zip(file.string(), ZIP_RDONLY);
   import_archive.set_password(*password);
   
   auto import_config = Config();
   auto import_config_data = import_archive.extract_to_memory(std::string("metadata.json"));
   import_config.parse(std::string(import_config_data.begin(), import_config_data.end()));

   for (auto &entry : import_config.data().items())
   {
      std::string key;

      try {
         key = entry.key();
      }
      catch (std::exception &exc) {
         throw exception::JSONException(exc);
      }

      MLX_DEBUGN("key: {}", key);

      json data;

      try {
         data = entry.value();
      }
      catch (std::exception &exc) {
         throw exception::JSONException(exc);
      }

      std::string sample_root;

      try {
         sample_root = data["sample_name"];
      }
      catch (std::exception &exc) {
         throw exception::JSONException(exc);
      }

      auto sample_archive_string = std::string("samples/") + sample_root;
      Logger::Info("loading sample file {}...", sample_archive_string);
      auto sample_data = import_archive.extract_to_memory(sample_archive_string);
      Logger::Raw(Logger::Level::Info, "done.\n");

      Logger::Info("loading sample into malexandria...");
      auto local_sample = Sample::FromData(sample_data);
      Logger::Raw(Logger::Level::Info, "done.\n");
      
      std::string sample_alias;

      try {
         sample_alias = data["alias"];
      }
      catch (std::exception &exc) {
         throw exception::JSONException(exc);
      }

      if (sample_alias.size() > 0)
         local_sample.set_alias(sample_alias);

      std::string sample_filename;

      try {
         sample_filename = data["filename"];
      }
      catch (std::exception &exc) {
         throw exception::JSONException(exc);
      }

      if (sample_filename.size() > 0)
         local_sample.set_filename(sample_filename);

      local_sample.remove_tags();

      for (auto &tag : data["tags"])
      {
         std::string tag_str;

         try {
            tag_str = tag;
         }
         catch (std::exception &exc)
         {
            throw exception::JSONException(exc);
         }

         local_sample.add_tag(tag_str);
      }

      local_sample.remove_families();

      for (auto &family : data["families"])
      {
         std::string family_str;

         try {
            family_str = family;
         }
         catch (std::exception &exc)
         {
            throw exception::JSONException(exc);
         }

         local_sample.add_family(family_str);
      }

      Logger::Info("saving sample to disk...");
      local_sample.save();
      Logger::Raw(Logger::Level::Info, "done.\n");
   }

   import_archive.close();
   std::vector<Sample> samples;
      
   for (auto &entry : import_config.data().items())
   {
      std::string key = entry.key();
      json data = entry.value();
      auto main_hash = from_hex_string(key);
      auto main_sample = Sample::ByHash(main_hash);
      samples.push_back(main_sample);

      for (auto &children : data["children"])
      {
         std::string child_str;

         try {
            child_str = children;
         }
         catch (std::exception &exc)
         {
            throw exception::JSONException(exc);
         }

         auto child_hash = from_hex_string(child_str);
         auto child_sample = Sample::ByHash(child_hash);

         main_sample.add_child(child_sample);
      }

      for (auto &parent : data["parents"])
      {
         std::string parent_str;

         try {
            parent_str = parent;
         }
         catch (std::exception &exc)
         {
            throw exception::JSONException(exc);
         }

         auto parent_hash = from_hex_string(parent_str);
         auto parent_sample = Sample::ByHash(parent_hash);

         main_sample.add_parent(parent_sample);
      }
   }

   return samples;
}

void Sample::remove_alias(void) {
   if (!this->is_saved())
      throw exception::SampleNotSaved();

   std::string query = "DELETE FROM mlx_aliases WHERE sample_id=?";
   sqlite3_stmt *statement;
   const char *tail;
   auto &database = Database::GetInstance();

   auto result = sqlite3_prepare_v2(*database,
                                    query.c_str(),
                                    query.size(),
                                    &statement,
                                    &tail);

   if (result != SQLITE_OK)
      throw exception::SQLiteException(result, sqlite3_errmsg(*database));

   result = sqlite3_bind_int64(statement, 1, *this->_row_id);

   if (result != SQLITE_OK)
   {
      sqlite3_finalize(statement);
      throw exception::SQLiteException(result, sqlite3_errmsg(*database));
   }

   result = sqlite3_step(statement);
   sqlite3_finalize(statement);

   if (result != SQLITE_DONE)
      throw exception::SQLiteException(result, sqlite3_errmsg(*database));

   this->_alias = std::nullopt;
}
      
void Sample::load_file(const std::filesystem::path &path) {
   if (!path_exists(path))
      throw exception::FileNotFound(path.string());

   this->_filename = path.filename().string();

   MLX_DEBUG("calculating sha256 of {} to determine if it exists...", this->_filename);
   this->_sha256 = malexandria::sha256(path);
   Logger::Raw(Logger::Level::Debug, "{}\n", to_hex_string(this->_sha256));
   
   auto id = Sample::IDFromHash(this->_sha256);

   if (id.has_value())
   {
      MLX_DEBUGN("got db id {}, loading it instead.", *id);
      
      this->load_id(*id);
   }
   else
   {
      MLX_DEBUG("sample is new. calculating md5...");
      this->_md5 = malexandria::md5(path);
      Logger::Raw(Logger::Level::Debug, "{}\n", to_hex_string(this->_md5));
      
      MLX_DEBUG("calculating sha1...");
      this->_sha1 = malexandria::sha1(path);
      Logger::Raw(Logger::Level::Debug, "{}\n", to_hex_string(this->_sha1));

      MLX_DEBUG("calculating sha3-384...");
      this->_sha3_384 = malexandria::sha3_384(path);
      Logger::Raw(Logger::Level::Debug, "{}\n", to_hex_string(this->_sha3_384));
   }
   
   this->_loaded_file = path;
}

void Sample::load_data(const std::vector<std::uint8_t> &data) {
   MLX_DEBUG("calculating sha256 of data to determine if it exists...");
   this->_sha256 = malexandria::sha256(data.data(), data.size());
   Logger::Raw(Logger::Level::Debug, "{}\n", to_hex_string(this->_sha256));
   
   auto id = Sample::IDFromHash(this->_sha256);

   if (id.has_value())
   {
      MLX_DEBUGN("got db id {}, loading it instead.", *id);
      
      this->load_id(*id);
   }
   else
   {
      MLX_DEBUG("sample is new. calculating md5...");
      this->_md5 = malexandria::md5(data.data(), data.size());
      Logger::Raw(Logger::Level::Debug, "{}\n", to_hex_string(this->_md5));
      
      MLX_DEBUG("calculating sha1...");
      this->_sha1 = malexandria::sha1(data.data(), data.size());
      Logger::Raw(Logger::Level::Debug, "{}\n", to_hex_string(this->_sha1));

      MLX_DEBUG("calculating sha3-384...");
      this->_sha3_384 = malexandria::sha3_384(data.data(), data.size());
      Logger::Raw(Logger::Level::Debug, "{}\n", to_hex_string(this->_sha3_384));

      this->_data = data;
   }
}

void Sample::load_hash(const std::vector<std::uint8_t> &hash) {
   auto id = Sample::IDFromHash(hash);

   if (!id.has_value())
      throw exception::HashNotFound(to_hex_string(hash));

   this->load_id(*id);
}

void Sample::load_id(std::int64_t id) {
   auto result = Database::GetInstance().query("\
SELECT\
   filename,\
   md5,\
   sha1,\
   sha256,\
   sha3_384, \
   created, \
   modified \
FROM \
   mlx_samples \
WHERE \
   id = ?",
                                               id);

   if (result.size() == 0)
      throw exception::SampleIDNotFound(id);

   this->_row_id = id;
   this->_filename = std::get<std::string>(result[0][0]);
   this->_md5 = from_hex_string(std::get<std::string>(result[0][1]));
   this->_sha1 = from_hex_string(std::get<std::string>(result[0][2]));
   this->_sha256 = from_hex_string(std::get<std::string>(result[0][3]));
   this->_sha3_384 = from_hex_string(std::get<std::string>(result[0][4]));
   this->_created = std::get<std::int64_t>(result[0][5]);
   this->_modified = std::get<std::int64_t>(result[0][6]);
      
   this->load_alias();
   this->load_tags();
   this->load_families();
}

void Sample::save_to_disk(void) {
   if (!this->_loaded_file.has_value() && !this->_data.has_value())
      throw exception::OriginalFileNotPresent();

   auto &config = MainConfig::GetInstance();
   auto sample_archive = this->archive_file();

   if (!path_exists(sample_archive.parent_path()) && !std::filesystem::create_directories(sample_archive.parent_path()))
      throw exception::CreateDirectoryFailure(sample_archive.parent_path().string());

   auto sample_zip = this->archive(ZIP_CREATE | ZIP_TRUNCATE);
   auto hex_string = to_hex_string(this->_sha256);
   auto benign_ext = config.benign_extension();
   auto zip_filename = hex_string + std::string(".") + benign_ext;

   if (this->_loaded_file.has_value())
      sample_zip.insert_file(*this->_loaded_file, zip_filename);
   else if (this->_data.has_value())
      sample_zip.insert_buffer(*this->_data, zip_filename);
   
   sample_zip.close();

   if (this->_data.has_value())
      this->_data = std::nullopt;
}

void Sample::save_to_database(void) {
   std::string query;
   bool exists = (this->_row_id.has_value() && Sample::Exists(*this->_row_id));

   if (exists)
   {
      query = "\
UPDATE\
         mlx_samples                            \
SET\
         filename=?,                            \
         md5=?,\
         sha1=?,\
         sha256=?,\
         sha3_384=?, \
         modified=unixepoch() \
WHERE\
         id=?";
   }
   else {
      query = "\
INSERT INTO\
         mlx_samples\
         (filename,\
          md5,\
          sha1,\
          sha256,\
          sha3_384,\
          created,\
          modified) \
VALUES\
         (?,?,?,?,?,unixepoch(),unixepoch())";
   }

   auto md5_string = to_hex_string(this->_md5);
   auto sha1_string = to_hex_string(this->_sha1);
   auto sha256_string = to_hex_string(this->_sha256);
   auto sha3_384_string = to_hex_string(this->_sha3_384);
   
   const char *tail;
   auto statement = Statement(query,
                              &tail,
                              this->_filename,
                              md5_string,
                              sha1_string,
                              sha256_string,
                              sha3_384_string);
                              
   if (exists)
      statement.bind(*this->_row_id);

   statement.execute();
   
   if (!exists)
      this->_row_id = Database::GetInstance().last_insert_rowid();

   this->save_alias();
   this->save_tags();
   this->save_families();
}

void Sample::load_alias(void) {
   if (!this->is_saved())
      throw exception::SampleNotSaved();

   auto result = Database::GetInstance().query("SELECT alias FROM mlx_aliases WHERE sample_id = ?", this->row_id());

   if (result.size() == 0)
      this->_alias = std::nullopt;
   else
      this->_alias = std::get<std::string>(result[0][0]);
}

void Sample::save_alias(void) {
   if (!this->has_alias())
      return;

   if (!this->is_saved())
      throw exception::SampleNotSaved();

   auto current_alias_id = Sample::IDFromAlias(*this->_alias);

   if (current_alias_id.has_value())
   {
      if (*current_alias_id == *this->_row_id)
         return;

      throw exception::AliasExists(*this->_alias);
   }

   auto &db = Database::GetInstance();
   auto current_alias_check = db.query("SELECT alias FROM mlx_aliases WHERE sample_id=?", this->row_id());
   std::optional<std::string> current_alias = std::nullopt;

   if (current_alias_check.size() > 0)
      current_alias = std::get<std::string>(current_alias_check[0][0]);

   if (current_alias == this->_alias)
      return;

   std::string query;
   
   if (current_alias.has_value())
   {
      query = "\
UPDATE \
         mlx_aliases \
SET \
         alias=? \
WHERE \
         sample_id=?";
   }
   else
   {
      query = "\
INSERT INTO\
         mlx_aliases\
         (alias,\
          sample_id)\
VALUES\
         (?,?)";
   }

   db.query(query, *this->_alias, this->row_id());
}

std::vector<Tag> Sample::current_tags(void) {
   if (!this->_row_id.has_value())
      throw exception::SampleNotSaved();
   
   auto result = Database::GetInstance().query("SELECT tag_id FROM mlx_sample_tags WHERE sample_id=?", this->row_id());
   std::vector<Tag> return_value;

   for (auto &row : result)
      return_value.push_back(Tag(std::get<std::int64_t>(row[0])));

   return return_value;
}

void Sample::load_tags() {
   this->_tags = this->current_tags();
}

bool Sample::has_tag_in_database(const Tag &tag) const {
   if (!this->is_saved())
      throw exception::SampleNotSaved();
   
   if (!tag.is_saved())
      return false;

   return Database::GetInstance().query("SELECT sample_id FROM mlx_sample_tags WHERE sample_id=? AND tag_id=?",
                                        this->row_id(),
                                        tag.row_id()).size() > 0;
}

void Sample::add_tag_to_database(const Tag &tag) const {
   if (!this->is_saved())
      throw exception::SampleNotSaved();
   
   if (!tag.is_saved())
      throw exception::TagNotSaved();

   Database::GetInstance().query("INSERT INTO mlx_sample_tags (sample_id, tag_id) VALUES (?,?)",
                                 this->row_id(),
                                 tag.row_id());
}

void Sample::remove_tag_from_database(const Tag &tag) const {
   if (!this->is_saved())
      throw exception::SampleNotSaved();
   
   if (!tag.is_saved())
      throw exception::TagNotSaved();

   Database::GetInstance().query("DELETE FROM mlx_sample_tags WHERE sample_id=? AND tag_id=?",
                                 this->row_id(),
                                 tag.row_id());
}

void Sample::save_tags() {
   for (auto &tag : this->_tags)
      if (!tag.is_saved())
         tag.save();
   
   auto current = this->current_tags();
   auto db_set = std::set<Tag>(current.begin(), current.end());
   auto instance_set = std::set<Tag>(this->_tags.begin(), this->_tags.end());

   // in instance set but not in db = new
   std::vector<Tag> new_tags;
   std::set_difference(instance_set.begin(),
                       instance_set.end(),
                       db_set.begin(),
                       db_set.end(),
                       std::back_inserter(new_tags));
   
   // in db set but not in instance = deleted
   std::vector<Tag> removed_tags;
   std::set_difference(db_set.begin(),
                       db_set.end(),
                       instance_set.begin(),
                       instance_set.end(),
                       std::back_inserter(removed_tags));

   for (auto &tag : new_tags)
      this->add_tag_to_database(tag);

   for (auto &tag : removed_tags)
   {
      this->remove_tag_from_database(tag);

      if (!tag.has_samples())
         tag.erase();
   }
}

std::vector<Family> Sample::current_families(void) {
   if (!this->is_saved())
      throw exception::SampleNotSaved();
   
   auto result = Database::GetInstance().query("SELECT family_id FROM mlx_sample_families WHERE sample_id=?",
                                               this->row_id());
   std::vector<Family> return_value;

   for (auto &row : result)
      return_value.push_back(Family(std::get<std::int64_t>(row[0])));
   
   return return_value;
}

void Sample::load_families() {
   this->_families = this->current_families();
}

bool Sample::has_family_in_database(const Family &family) const {
   if (!this->is_saved())
      throw exception::SampleNotSaved();
   
   if (!family.is_saved())
      return false;

   return Database::GetInstance().query("SELECT sample_id FROM mlx_sample_families WHERE sample_id=? AND family_id=?",
                                        this->row_id(),
                                        family.row_id()).size() > 0;
}

void Sample::add_family_to_database(const Family &family) const {
   if (!this->is_saved())
      throw exception::SampleNotSaved();
   
   if (!family.is_saved())
      throw exception::FamilyNotSaved();

   Database::GetInstance().query("INSERT INTO mlx_sample_families (sample_id, family_id) VALUES (?,?)",
                                 this->row_id(),
                                 family.row_id());
}

void Sample::remove_family_from_database(const Family &family) const {
   if (!this->is_saved())
      throw exception::SampleNotSaved();
   
   if (!family.is_saved())
      throw exception::FamilyNotSaved();

   Database::GetInstance().query("DELETE FROM mlx_sample_families WHERE sample_id=? AND family_id=?",
                                 this->row_id(),
                                 family.row_id());
}

void Sample::save_families() {
   for (auto &family : this->_families)
      if (!family.is_saved())
         family.save();
   
   auto current = this->current_families();
   auto db_set = std::set<Family>(current.begin(), current.end());
   auto instance_set = std::set<Family>(this->_families.begin(), this->_families.end());

   // in instance set but not in db = new
   std::vector<Family> new_families;
   std::set_difference(instance_set.begin(),
                       instance_set.end(),
                       db_set.begin(),
                       db_set.end(),
                       std::back_inserter(new_families));
   
   // in db set but not in instance = deleted
   std::vector<Family> removed_families;
   std::set_difference(db_set.begin(),
                       db_set.end(),
                       instance_set.begin(),
                       instance_set.end(),
                       std::back_inserter(removed_families));

   for (auto &family : new_families)
      this->add_family_to_database(family);

   for (auto &family : removed_families)
   {
      this->remove_family_from_database(family);

      if (!family.has_samples())
         family.erase();
   }
}

void Sample::save(void) {
   if (this->_loaded_file.has_value() || this->_data.has_value())
      this->save_to_disk();

   this->save_to_database();
}

void Sample::erase(void) {
   if (!this->is_saved())
      throw exception::SampleNotSaved();

   this->remove_alias();
   this->remove_tags();
   this->remove_families();
   this->remove_parents();
   this->remove_children();

   Database::GetInstance().query("DELETE FROM mlx_samples WHERE id=?",
                                 this->row_id());
   this->_row_id = std::nullopt;

   if (path_exists(this->archive_file()))
      erase_file(this->archive_file());

   //if (path_exists(this->analysis_file()))
   //   erase_file(this->analysis_file());

   //if (path_exists(this->analysis_metadata_file()))
   //   erase_file(this->analysis_metadata_file());

   auto &config = MainConfig::GetInstance();
   clear_directory(config.vault_path(), this->vault_path());
}

bool Sample::has_tag(const Tag &tag) const {
   return std::find(this->_tags.begin(), this->_tags.end(), tag) != this->_tags.end();
}

void Sample::add_tag(const Tag &tag) {
   if (!this->has_tag(tag))
      this->_tags.push_back(tag);
}

void Sample::remove_tag(const Tag &tag) {
   if (!this->has_tag(tag))
      throw exception::TagNotFound(*tag);

   this->_tags.erase(std::find(this->_tags.begin(), this->_tags.end(), tag));
}

void Sample::remove_tags() {
   if (!this->is_saved())
   {
      for (auto &tag : this->_tags)
         if (!tag.has_samples())
            tag.erase();
      
      this->_tags.clear();
      return;
   }
   
   Database::GetInstance().query("DELETE FROM mlx_sample_tags WHERE sample_id=?",
                                 this->row_id());
   
   for (auto &tag : this->_tags)
      if (!tag.has_samples())
         tag.erase();

   this->_tags.clear();
}

bool Sample::has_family(const Family &family) const {
   return std::find(this->_families.begin(), this->_families.end(), family) != this->_families.end();
}

void Sample::add_family(const Family &family) {
   if (!this->has_family(family))
      this->_families.push_back(family);
}

void Sample::remove_family(const Family &family) {
   if (!this->has_family(family))
      throw exception::FamilyNotFound(*family);

   this->_families.erase(std::find(this->_families.begin(), this->_families.end(), family));
}

void Sample::remove_families() {
   if (!this->is_saved())
   {
      for (auto &family : this->_families)
         if (!family.has_samples())
            family.erase();

      this->_families.clear();
      return;
   }
   
   Database::GetInstance().query("DELETE FROM mlx_sample_families WHERE sample_id=?",
                                 this->row_id());

   for (auto &family : this->_families)
      if (!family.has_samples())
         family.erase();

   this->_families.clear();
}

std::vector<Sample> Sample::parents(void) const {
   if (!this->is_saved())
      throw exception::SampleNotSaved();
   
   auto result = Database::GetInstance().query("SELECT parent_id FROM mlx_sample_relationships WHERE child_id=?",
                                               this->row_id());
   std::vector<Sample> samples;

   for (auto &row : result)
      samples.push_back(std::get<std::int64_t>(row[0]));

   return samples;
}

bool Sample::is_parent_of(const Sample &sample) const {
   if (!this->is_saved() || !sample.is_saved())
      throw exception::SampleNotSaved();
   
   return Database::GetInstance().query("SELECT parent_id FROM mlx_sample_relationships WHERE parent_id=? AND child_id=?",
                                        this->row_id(),
                                        sample.row_id()).size() > 0;
}

void Sample::add_parent(const Sample &sample) {
   if (!this->is_saved() || !sample.is_saved())
      throw exception::SampleNotSaved();
   
   if (sample.is_parent_of(*this))
      return;

   Database::GetInstance().query("INSERT INTO mlx_sample_relationships (parent_id, child_id) VALUES (?,?)",
                                 sample.row_id(),
                                 this->row_id());
}

void Sample::remove_parent(const Sample &sample) {
   if (!this->is_saved() || !sample.is_saved())
      throw exception::SampleNotSaved();
   
   if (!sample.is_parent_of(*this))
      return;

   Database::GetInstance().query("DELETE FROM mlx_sample_relationships WHERE parent_id=? AND child_id=?",
                                 sample.row_id(),
                                 this->row_id());
}
   
void Sample::remove_parents() {
   if (!this->is_saved())
      throw exception::SampleNotSaved();
   
   Database::GetInstance().query("DELETE FROM mlx_sample_relationships WHERE child_id=?",
                                 this->row_id());
}
   
std::vector<Sample> Sample::children(void) const {
   if (!this->is_saved())
      throw exception::SampleNotSaved();
   
   auto result = Database::GetInstance().query("SELECT child_id FROM mlx_sample_relationships WHERE parent_id=?",
                                               this->row_id());
   std::vector<Sample> samples;

   for (auto &row : result)
      samples.push_back(std::get<std::int64_t>(row[0]));

   return samples;
}

bool Sample::is_child_of(const Sample &sample) const {
   if (!this->is_saved() || !sample.is_saved())
      throw exception::SampleNotSaved();
   
   return Database::GetInstance().query("SELECT parent_id FROM mlx_sample_relationships WHERE parent_id=? AND child_id=?",
                                        sample.row_id(),
                                        this->row_id()).size() > 0;
}

void Sample::add_child(const Sample &sample) {
   if (!this->is_saved() || !sample.is_saved())
      throw exception::SampleNotSaved();
   
   if (sample.is_child_of(*this))
      return;

   Database::GetInstance().query("INSERT INTO mlx_sample_relationships (parent_id, child_id) VALUES (?,?)",
                                 this->row_id(),
                                 sample.row_id());
}

void Sample::remove_child(const Sample &sample) {
   if (!this->is_saved() || !sample.is_saved())
      throw exception::SampleNotSaved();
   
   if (!sample.is_child_of(*this))
      return;

   Database::GetInstance().query("DELETE FROM mlx_sample_relationships WHERE parent_id=? AND child_id=?",
                                 this->row_id(),
                                 sample.row_id());
}
   
void Sample::remove_children() {
   if (!this->is_saved())
      throw exception::SampleNotSaved();
   
   Database::GetInstance().query("DELETE FROM mlx_sample_relationships WHERE parent_id=?",
                                 this->row_id());
}

std::string Sample::label() const {
   if (this->has_alias())
      return this->alias();

   return to_hex_string(this->_sha256);
}

std::filesystem::path Sample::vault_path() const {
   auto &config = MainConfig::GetInstance();
   std::filesystem::path vault_root = config.vault_path();
   auto fanout = hash_fanout(this->_sha256);

   return vault_root / std::filesystem::path("by-sha256") / fanout;
}

std::filesystem::path Sample::alias_path() const {
   if (!this->has_alias())
      throw exception::NoAlias();
   
   auto &config = MainConfig::GetInstance();
   std::filesystem::path vault_root = config.vault_path();
   auto alias_path = this->alias();
   std::replace(alias_path.begin(), alias_path.end(), '.', '/');

   return vault_root / std::filesystem::path("by-alias") / alias_path;
}

std::filesystem::path Sample::active_file() const {
   auto &config = MainConfig::GetInstance();
   std::filesystem::path active_root = config.active_path();

   return active_root / this->_filename;
}

std::filesystem::path Sample::archive_file() const {
   return this->vault_path() / std::string("sample.zip");
}

std::string Sample::benign_file() const {
   auto hex = to_hex_string(this->_sha256);
   hex.push_back('.');
   hex += MainConfig::GetInstance().benign_extension();

   return hex;
}

Zip Sample::archive(int zip_flags) const {
   auto archive_file = this->archive_file().string();
   auto result = Zip(archive_file, zip_flags);
   auto &config = MainConfig::GetInstance();
   auto password = config.zip_password();
   result.set_password(password);

   return result;
}

std::vector<std::uint8_t> Sample::extract_to_memory() const {
   return this->archive(ZIP_RDONLY).extract_to_memory((std::uint64_t)0);
}
   
void Sample::extract_to_disk(std::optional<std::filesystem::path> filename) const {
   if (!filename.has_value())
      filename = this->active_file();

   if (!path_exists(filename->parent_path()))
      if (!std::filesystem::create_directories(filename->parent_path()))
         throw exception::CreateDirectoryFailure(filename->parent_path().string());

   this->archive(ZIP_RDONLY).extract_to_disk((std::uint64_t)0, *filename);
}

bool Sample::is_active() const {
   return path_exists(this->active_file());
}

void Sample::activate() const {
   this->extract_to_disk();
}

void Sample::deactivate() const {
   if (!this->is_active())
      throw exception::SampleNotActive();

   erase_file(this->active_file());
}
