#include "export.hpp"

using namespace malexandria;

bool Export::Config::has_sample(const std::string &hash) const {
   return this->data().find("samples") != this->data().end() &&
      (*this)["samples"] != nullptr &&
      (*this)["samples"].find(hash) != (*this)["samples"].end();
}

std::optional<std::vector<std::string>> Export::Config::samples(void) const {
   if (this->data().find("samples") == this->data().end())
      return std::nullopt;

   std::vector<std::string> result;
   
   for (auto &entry : (*this)["samples"].items())
      result.push_back(entry.key());

   return result;
}

void Export::Config::load_sample(const Sample &sample) {
   auto hash_string = to_hex_string(sample.sha256());
   auto archive_filename = std::filesystem::path("samples") / sample.benign_file();
   archive_filename = std::filesystem::path(dos_to_unix_path(archive_filename.string()));

   this->set_export_filename(hash_string, archive_filename.string());
   this->set_active_filename(hash_string, sample.filename());

   if (!sample.has_alias())
      this->set_alias(hash_string, std::nullopt);
   else
      this->set_alias(hash_string, sample.alias());

   auto tags = sample.tags();

   if (tags.size() == 0)
      (*this)["samples"][hash_string]["tags"] = nullptr;
   else
      for (auto &tag : tags)
         this->add_tag(hash_string, *tag);

   auto families = sample.families();

   if (families.size() == 0)
      (*this)["samples"][hash_string]["families"] = nullptr;
   else
      for (auto &family : families)
         this->add_family(hash_string, *family);

   auto parents = sample.parents();

   if (parents.size() == 0)
      (*this)["samples"][hash_string]["parents"] = nullptr;
   else
      for (auto &parent : parents)
         this->add_parent(hash_string, to_hex_string(parent.sha256()));

   auto children = sample.children();

   if (children.size() == 0)
      (*this)["samples"][hash_string]["children"] = nullptr;
   else
      for (auto &child : children)
         this->add_child(hash_string, to_hex_string(child.sha256()));
}

void Export::Config::remove_sample(const std::string &hash) {
   if (!this->has_sample(hash))
      throw exception::SampleNotExported(hash);

   (*this)["samples"].erase((*this)["samples"].find(hash));
}

std::string Export::Config::get_export_filename(const std::string &hash) const {
   if (!this->has_sample(hash))
      throw exception::SampleNotExported(hash);

   try {
      return (*this)["samples"][hash]["export_filename"];
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void Export::Config::set_export_filename(const std::string &hash, const std::string &filename) {
   try {
      (*this)["samples"][hash]["export_filename"] = filename;
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

std::string Export::Config::get_active_filename(const std::string &hash) const {
   if (!this->has_sample(hash))
      throw exception::SampleNotExported(hash);

   try {
      return (*this)["samples"][hash]["active_filename"];
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void Export::Config::set_active_filename(const std::string &hash, const std::string &filename) {
   try {
      (*this)["samples"][hash]["active_filename"] = filename;
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

std::optional<std::string> Export::Config::get_alias(const std::string &hash) const {
   if (!this->has_sample(hash))
      throw exception::SampleNotExported(hash);

   try {
      if ((*this)["samples"][hash]["alias"] == nullptr)
         return std::nullopt;

      return (*this)["samples"][hash]["alias"];
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void Export::Config::set_alias(const std::string &hash, std::optional<std::string> alias) {
   try {
      if (alias.has_value())
         (*this)["samples"][hash]["alias"] = *alias;
      else
         (*this)["samples"][hash]["alias"] = nullptr;
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

std::optional<std::vector<std::string>> Export::Config::get_tags(const std::string &hash) const {
   if (!this->has_sample(hash))
      throw exception::SampleNotExported(hash);

   try {
      if ((*this)["samples"][hash]["tags"] == nullptr)
         return std::nullopt;
      else
         return (*this)["samples"][hash]["tags"].get<std::vector<std::string>>();
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void Export::Config::add_tag(const std::string &hash, const std::string &tag) {
   try {
      if ((*this)["samples"][hash].find("tags") == (*this)["samples"][hash].end() || (*this)["samples"][hash]["tags"] == nullptr)
         (*this)["samples"][hash]["tags"] = std::vector<std::string>();

      (*this)["samples"][hash]["tags"].push_back(tag);
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void Export::Config::remove_tag(const std::string &hash, const std::string &tag) {
   if (!this->has_sample(hash))
      throw exception::SampleNotExported(hash);

   try {
      if ((*this)["samples"][hash]["tags"] == nullptr)
         return;

      auto result = std::find((*this)["samples"][hash]["tags"].begin(),
                              (*this)["samples"][hash]["tags"].end(),
                              tag);

      if (result != (*this)["samples"][hash]["tags"].end())
         (*this)["samples"][hash]["tags"].erase(tag);

      if ((*this)["samples"][hash]["tags"].size() == 0)
         (*this)["samples"][hash]["tags"] = nullptr;
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

std::optional<std::vector<std::string>> Export::Config::get_families(const std::string &hash) const {
   if (!this->has_sample(hash))
      throw exception::SampleNotExported(hash);

   try {
      if ((*this)["samples"][hash]["families"] == nullptr)
         return std::nullopt;
      else
         return (*this)["samples"][hash]["families"].get<std::vector<std::string>>();
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void Export::Config::add_family(const std::string &hash, const std::string &family) {
   try {
      if ((*this)["samples"][hash].find("families") == (*this)["samples"][hash].end() || (*this)["samples"][hash]["families"] == nullptr)
         (*this)["samples"][hash]["families"] = std::vector<std::string>();

      (*this)["samples"][hash]["families"].push_back(family);
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void Export::Config::remove_family(const std::string &hash, const std::string &family) {
   if (!this->has_sample(hash))
      throw exception::SampleNotExported(hash);

   try {
      if ((*this)["samples"][hash]["families"] == nullptr)
         return;

      auto result = std::find((*this)["samples"][hash]["families"].begin(),
                              (*this)["samples"][hash]["families"].end(),
                              family);

      if (result != (*this)["samples"][hash]["families"].end())
         (*this)["samples"][hash]["families"].erase(family);

      if ((*this)["samples"][hash]["families"].size() == 0)
         (*this)["samples"][hash]["families"] = nullptr;
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

std::optional<std::vector<std::string>> Export::Config::get_parents(const std::string &hash) const {
   if (!this->has_sample(hash))
      throw exception::SampleNotExported(hash);

   try {
      if ((*this)["samples"][hash]["parents"] == nullptr)
         return std::nullopt;
      else
         return (*this)["samples"][hash]["parents"].get<std::vector<std::string>>();
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void Export::Config::add_parent(const std::string &hash, const std::string &parent) {
   try {
      if ((*this)["samples"][hash].find("parents") == (*this)["samples"][hash].end() || (*this)["samples"][hash]["parents"] == nullptr)
         (*this)["samples"][hash]["parents"] = std::vector<std::string>();

      (*this)["samples"][hash]["parents"].push_back(parent);
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void Export::Config::remove_parent(const std::string &hash, const std::string &parent) {
   if (!this->has_sample(hash))
      throw exception::SampleNotExported(hash);

   try {
      if ((*this)["samples"][hash]["parents"] == nullptr)
         return;

      auto result = std::find((*this)["samples"][hash]["parents"].begin(),
                              (*this)["samples"][hash]["parents"].end(),
                              parent);

      if (result != (*this)["samples"][hash]["parents"].end())
         (*this)["samples"][hash]["parents"].erase(parent);

      if ((*this)["samples"][hash]["parents"].size() == 0)
         (*this)["samples"][hash]["parents"] = nullptr;
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

std::optional<std::vector<std::string>> Export::Config::get_children(const std::string &hash) const {
   if (!this->has_sample(hash))
      throw exception::SampleNotExported(hash);

   try {
      if ((*this)["samples"][hash]["children"] == nullptr)
         return std::nullopt;
      else
         return (*this)["samples"][hash]["children"].get<std::vector<std::string>>();
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void Export::Config::add_child(const std::string &hash, const std::string &child) {
   try {
      if ((*this)["samples"][hash].find("children") == (*this)["samples"][hash].end() || (*this)["samples"][hash]["children"] == nullptr)
         (*this)["samples"][hash]["children"] = std::vector<std::string>();

      (*this)["samples"][hash]["children"].push_back(child);
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void Export::Config::remove_child(const std::string &hash, const std::string &child) {
   if (!this->has_sample(hash))
      throw exception::SampleNotExported(hash);

   try {
      if ((*this)["samples"][hash]["children"] == nullptr)
         return;

      auto result = std::find((*this)["samples"][hash]["children"].begin(),
                              (*this)["samples"][hash]["children"].end(),
                              child);

      if (result != (*this)["samples"][hash]["children"].end())
         (*this)["samples"][hash]["children"].erase(child);

      if ((*this)["samples"][hash]["children"].size() == 0)
         (*this)["samples"][hash]["children"] = nullptr;
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

bool Export::Config::has_analysis(const std::string &id) const {
   return this->data().find("analyses") != this->data().end() &&
      (*this)["analyses"] != nullptr &&
      (*this)["analyses"].find(id) != (*this)["analyses"].end();
}

std::optional<std::vector<std::string>> Export::Config::analyses(void) const {
   if (this->data().find("analyses") == this->data().end())
      return std::nullopt;

   std::vector<std::string> result;
   
   for (auto &entry : (*this)["analyses"].items())
      result.push_back(entry.key());

   return result;
}

void Export::Config::load_analysis(const Analysis &analysis) {
   try {
      auto id = uuids::to_string(analysis.id());
      auto filename = std::filesystem::path("analyses") / id;
      filename += std::string(".zip");
      auto filename_string = dos_to_unix_path(filename.string());
      (*this)["analyses"][id] = filename_string;
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

std::string Export::Config::get_analysis_file(const std::string &id) const {
   if (!this->has_analysis(id))
      throw exception::AnalysisNotExported(id);

   try {
      return (*this)["analyses"][id];
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void Export::Config::remove_analysis(const std::string &id) {
   if (!this->has_analysis(id))
      throw exception::AnalysisNotExported(id);

   try {
      (*this)["analyses"].erase((*this)["analyses"].find(id));
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void Export::load_config(void) {
   auto config_data = this->_archive.extract_to_memory("metadata.json");
   auto config_string = std::string(config_data.begin(), config_data.end());
   this->_config.parse(config_string);
}

void Export::save_config(void) {
   auto config_string = this->_config.to_string();
   auto config_data = std::vector<std::uint8_t>(config_string.begin(), config_string.end());
   this->_archive.insert_buffer(config_data, "metadata.json");
}

std::vector<std::vector<std::uint8_t>> Export::samples(void) {
   std::vector<std::vector<std::uint8_t>> result;
   auto samples = this->_config.samples();

   if (!samples.has_value())
      return result;

   for (auto &sample : *samples)
      result.push_back(from_hex_string(sample));

   return result;
}

Sample Export::import_sample(const std::vector<std::uint8_t> &hash) {
   auto hash_string = to_hex_string(hash);

   if (!this->_config.has_sample(hash_string))
      throw exception::SampleNotExported(hash_string);

   auto export_filename = this->_config.get_export_filename(hash_string);
   auto sample = Sample::FromData(this->_archive.extract_to_memory(export_filename));
   sample.set_filename(this->_config.get_active_filename(hash_string));

   auto alias = this->_config.get_alias(hash_string);

   if (alias.has_value())
      sample.set_alias(*alias);

   auto tags = this->_config.get_tags(hash_string);

   if (tags.has_value())
   {
      sample.remove_tags();
      
      for (auto &tag : *tags)
         sample.add_tag(tag);
   }

   auto families = this->_config.get_families(hash_string);

   if (families.has_value())
   {
      sample.remove_families();

      for (auto &family : *families)
         sample.add_family(family);
   }

   sample.save();

   auto parents = this->_config.get_parents(hash_string);

   if (parents.has_value())
   {
      for (auto &parent : *parents)
      {
         Sample parent_sample;
         auto parent_hash = from_hex_string(parent);
         
         if (this->_config.has_sample(parent))
            parent_sample = this->import_sample(parent_hash);
         else
            parent_sample = Sample::ByHash(parent_hash);

         sample.add_parent(parent_sample);
      }
   }
   
   auto children = this->_config.get_children(hash_string);

   if (children.has_value())
   {
      for (auto &child : *children)
      {
         Sample child_sample;
         auto child_hash = from_hex_string(child);
         
         if (this->_config.has_sample(child))
            child_sample = this->import_sample(child_hash);
         else
            child_sample = Sample::ByHash(child_hash);

         sample.add_child(child_sample);
      }
   }

   sample.save();

   return sample;
}

std::vector<Sample> Export::import_samples(void) {
   std::vector<Sample> result;
   
   for (auto &hash : this->samples())
      result.push_back(this->import_sample(hash));

   return result;
}

void Export::add_sample(const Sample &sample) {
   auto zip_filename = std::filesystem::path("samples") / sample.benign_file();
   zip_filename = std::filesystem::path(dos_to_unix_path(zip_filename.string()));

   this->_config.load_sample(sample);
   this->_archive.create_directories("samples/");
   this->_archive.insert_buffer(sample.extract_to_memory(), zip_filename.string());

   this->save_config();
}

void Export::remove_sample(const std::vector<std::uint8_t> &hash) {
   auto sample = Sample::ByHash(hash);
   auto hash_string = to_hex_string(hash);
   auto zip_filename = std::filesystem::path("samples") / sample.benign_file();
   zip_filename = std::filesystem::path(dos_to_unix_path(zip_filename.string()));

   this->_config.remove_sample(hash_string);
   this->_archive.delete_file(zip_filename.string());
   this->_archive.clear_directories(zip_filename);

   this->save_config();
}

std::vector<uuids::uuid> Export::analyses(void) {
   std::vector<uuids::uuid> result;
   auto analyses = this->_config.analyses();

   if (!analyses.has_value())
      return result;
   
   for (auto &id : *analyses)
   {
      auto parsed = uuids::uuid::from_string(id);

      if (!parsed.has_value())
         throw exception::InvalidUUID(id);

      result.push_back(*parsed);
   }

   return result;
}

Analysis Export::import_analysis(const uuids::uuid &id) {
   auto id_string = uuids::to_string(id);

   MLX_DEBUGN("importing {}...", id_string);
   
   if (!this->_config.has_analysis(id_string))
      throw exception::AnalysisNotExported(id_string);

   std::filesystem::path analysis_archive = MainConfig::GetInstance().analysis_path();
   analysis_archive /= hash_fanout(id_string);
   analysis_archive += std::string(".zip");

   MLX_DEBUGN("saving to {}...", analysis_archive.string());

   auto zip_file = this->_config.get_analysis_file(id_string);

   if (!path_exists(analysis_archive.parent_path()))
      if (!std::filesystem::create_directories(analysis_archive.parent_path()))
         throw exception::CreateDirectoryFailure(analysis_archive.parent_path().string());

   MLX_DEBUGN("extracting...");
   this->_archive.extract_to_disk(zip_file, analysis_archive);
   MLX_DEBUGN("loading {}...", analysis_archive.string());
   auto zip_archive = Zip(analysis_archive, ZIP_RDONLY);
   zip_archive.set_password(MainConfig::GetInstance().zip_password()); // FIXME this is a bug to address
   MLX_DEBUGN("enumerating file tree...");
   auto file_tree = zip_archive.files();

   for (auto &entry : file_tree)
      MLX_DEBUGN("\t{}", entry.second);
   
   MLX_DEBUGN("extracting metadata...");
   auto config_data = zip_archive.extract_to_memory("metadata.json");
   MLX_DEBUGN("parsing config...");
   auto config_string = std::string(config_data.begin(), config_data.end());
   auto config = Analysis::Config();
   config.parse(config_string);

   auto &db = Database::GetInstance();
   auto has_id = db.query("SELECT id FROM mlx_analysis WHERE analysis_id = ?", id_string);
   std::int64_t row_id;

   if (has_id.size() != 0)
      row_id = std::get<std::int64_t>(has_id[0][0]);
   else
   {
      db.query("INSERT INTO mlx_analysis (analysis_id) VALUES (?)", id_string);
      row_id = db.last_insert_rowid();
   }

   for (auto &hash : config.samples())
   {
      auto hash_bytes = from_hex_string(hash.first);
      MLX_DEBUGN("importing {}...", hash.first);
      
      Sample sample;

      if (this->_config.has_sample(hash.first))
      {
         MLX_DEBUGN("sample exists in archive.");
         sample = this->import_sample(hash_bytes);
      }
      else
      {
         MLX_DEBUGN("sample does not exist in archive.");
         sample = Sample::ByHash(hash_bytes);
      }

      auto has_sample = db.query("SELECT analysis_id FROM mlx_analysis_samples WHERE analysis_id = ? AND sample_id = ?",
                                 row_id,
                                 sample.row_id());

      if (has_sample.size() == 0)
         db.query("INSERT INTO mlx_analysis_samples (analysis_id, sample_id) VALUES (?,?)",
                  row_id,
                  sample.row_id());
   }

   return Analysis(id);
}

std::vector<Analysis> Export::import_analyses(void) {
   std::vector<Analysis> result;
   
   for (auto &id : this->analyses())
      result.push_back(this->import_analysis(id));

   return result;
}

void Export::add_analysis(const Analysis &analysis) {
   if (!analysis.is_saved())
      throw exception::AnalysisNotSaved(uuids::to_string(analysis.id()));

   for (auto &sample_id : analysis.samples())
      this->add_sample(analysis.get_sample(sample_id));

   this->_config.load_analysis(analysis);

   auto filename = std::filesystem::path("analyses") / uuids::to_string(analysis.id());
   filename += std::string(".zip");
   filename = std::filesystem::path(dos_to_unix_path(filename.string()));

   this->_archive.create_directories("analyses/");
   this->_archive.insert_file(analysis.archive_file(), filename.string());

   this->save_config();
}

void Export::remove_analysis(const uuids::uuid &id) {
   auto id_string = uuids::to_string(id);

   if (!this->_config.has_analysis(id_string))
      throw exception::AnalysisNotExported(id_string);

   auto zip_filename = std::filesystem::path("analyses") / id_string;
   zip_filename += std::string(".zip");
   zip_filename = std::filesystem::path(dos_to_unix_path(zip_filename.string()));

   this->_archive.delete_file(zip_filename.string());
   this->_archive.clear_directories(zip_filename.string());

   this->save_config();
}
 
