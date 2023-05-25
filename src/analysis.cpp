#include "analysis.hpp"

using namespace malexandria;

std::string Analysis::Config::analysis_id(void) const {
   try {
      return (*this)["analysis_id"];
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void Analysis::Config::set_analysis_id(const std::string &id) {
   (*this)["analysis_id"] = id;
}

std::optional<std::string> Analysis::Config::analysis_alias(void) const {
   try {
      auto value = (*this)["analysis_alias"];

      if (value == nullptr)
         return std::nullopt;
      else
         return std::string(value);
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void Analysis::Config::set_analysis_alias(std::optional<std::string> alias) {
   try {
      if (alias.has_value())
         (*this)["analysis_alias"] = *alias;
      else
         (*this)["analysis_alias"] = nullptr;
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

std::map<std::string,std::optional<std::string>> Analysis::Config::samples(void) const {
   std::map<std::string,std::optional<std::string>> result;
   
   try {
      auto samples = (*this)["samples"];

      for (auto &entry : samples.items())
      {
         auto hash = entry.key();
         auto filename = entry.value();

         if (filename == nullptr)
            result[hash] = std::nullopt;
         else
            result[hash] = std::string(filename);
      }

      return result;
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

bool Analysis::Config::has_sample(const std::string &hash) const {
   try {
      return (*this)["samples"].find(hash) != (*this)["samples"].end();
   }
   catch (std::exception &exc) {
      return false;
   }
}

void Analysis::Config::add_sample(const std::string &hash, std::optional<std::string> active_file) {
   if (this->has_sample(hash))
      return;

   this->set_sample_active_file(hash, active_file);
}

std::optional<std::string> Analysis::Config::get_sample_active_file(const std::string &hash) const {
   if (!this->has_sample(hash))
      throw exception::SampleNotAnalyzed(hash);

   try {
      if ((*this)["samples"][hash] == nullptr)
         return std::nullopt;
      else
         return std::string((*this)["samples"][hash]);
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void Analysis::Config::set_sample_active_file(const std::string &hash, std::optional<std::string> active_file) {
   try {
      if (active_file.has_value())
         (*this)["samples"][hash] = *active_file;
      else
         (*this)["samples"][hash] = nullptr;
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void Analysis::Config::remove_sample(const std::string &hash) {
   if (!this->has_sample(hash))
   {
      MLX_DEBUGN("don't have sample {}.", hash);
      return;
   }

   try {
      (*this)["samples"].erase((*this)["samples"].find(hash));
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

Analysis::Config::FileMap Analysis::Config::files(void) const {
   Analysis::Config::FileMap result;

   try {
      for (auto &entry : (*this)["files"].items())
      {
         auto filename = entry.key();
         auto file_info = entry.value();

         auto artifact_json = file_info["artifact"];
         auto link_json = file_info["link"];

         std::optional<std::string> artifact;

         if (artifact_json == nullptr)
            artifact = std::nullopt;
         else
            artifact = std::string(artifact_json);

         FileLink link;

         if (link_json == nullptr)
            link = std::nullopt;
         else
            link = std::make_pair(std::string(link_json["analysis_id"]),std::string(link_json["filename"]));

         result[filename] = std::make_pair(artifact, link);
      }

      return result;
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

bool Analysis::Config::has_file(const std::string &filename) const {
   try {
      return (*this)["files"].find(filename) != (*this)["files"].end();
   }
   catch (std::exception &exc) {
      return false;
   }
}

void Analysis::Config::add_file(const std::string &filename, std::optional<std::string> artifact, Analysis::Config::FileLink link) {
   if (this->has_file(filename))
      throw exception::FileExists(filename);

   (*this)["files"][filename] = {};
   this->set_file_artifact(filename, artifact);
   this->set_file_link(filename, link);
}

std::optional<std::string> Analysis::Config::get_file_artifact(const std::string &filename) const {
   if (!this->has_file(filename))
      throw exception::FileNotAnalyzed(filename);

   try {
      if ((*this)["files"][filename]["artifact"] == nullptr)
         return std::nullopt;
      else
         return std::string((*this)["files"][filename]["artifact"]);
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void Analysis::Config::set_file_artifact(const std::string &filename, std::optional<std::string> artifact) {
   try {
      if (artifact.has_value())
         (*this)["files"][filename]["artifact"] = *artifact;
      else
         (*this)["files"][filename]["artifact"] = nullptr;
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

Analysis::Config::FileLink Analysis::Config::get_file_link(const std::string &filename) const {
   if (!this->has_file(filename))
      throw exception::FileNotAnalyzed(filename);

   try {
      if ((*this)["files"][filename]["link"] == nullptr)
         return std::nullopt;
      else
         return std::make_pair(std::string((*this)["files"][filename]["link"]["analysis_id"]),
                               std::string((*this)["files"][filename]["link"]["filename"]));
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void Analysis::Config::set_file_link(const std::string &filename, Analysis::Config::FileLink link) {
   try {
      if (link.has_value())
      {
         (*this)["files"][filename]["link"] = {};
         (*this)["files"][filename]["link"]["analysis_id"] = link->first;
         (*this)["files"][filename]["link"]["filename"] = link->second;
      }
      else
         (*this)["files"][filename]["link"] = nullptr;
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void Analysis::Config::remove_file(const std::string &filename) {
   if (!this->has_file(filename))
      throw exception::FileNotAnalyzed(filename);

   try {
      (*this)["files"].erase((*this)["files"].find(filename));
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

std::map<std::string,std::vector<std::string>> Analysis::Config::dependencies(void) const {
   std::map<std::string,std::vector<std::string>> result;

   try {
      for (auto &entry : (*this)["dependencies"].items())
      {
         auto filename = entry.key();
         auto ids = entry.value();

         for (auto &analysis_id : ids)
            result[filename].push_back(analysis_id);
      }

      return result;
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

bool Analysis::Config::has_dependencies(const std::string &filename) const {
   try {
      return (*this)["dependencies"].find(filename) != (*this)["dependencies"].end();
   }
   catch (std::exception &exc) {
      return false;
   }
}

bool Analysis::Config::has_dependency(const std::string &filename, const std::string &id) const {
   try {
      return this->has_dependencies(filename) &&               \
         std::find((*this)["dependencies"][filename].begin(),
                   (*this)["dependencies"][filename].end(),
                   id) != (*this)["dependencies"][filename].end();
   }
   catch (std::exception &exc) {
      return false;
   }
}

std::vector<std::string> Analysis::Config::get_dependencies(const std::string &filename) const {
   if (!this->has_dependencies(filename))
      throw exception::NoDependencies(filename);

   try {
      return (*this)["dependencies"][filename];
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void Analysis::Config::add_dependency(const std::string &filename, const std::string &id) {
   if (this->has_dependency(filename, id))
      return;
   
   try {
      if (!this->has_key("dependencies"))
         (*this)["dependencies"] = {};

      if ((*this)["dependencies"].find(filename) == (*this)["dependencies"].end())
         (*this)["dependencies"][filename] = std::vector<std::string>();

      (*this)["dependencies"][filename].push_back(id);
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void Analysis::Config::remove_dependency(const std::string &filename, const std::string &id) {
   if (!this->has_dependency(filename, id))
      return;

   try {
      (*this)["dependencies"][filename].erase(std::find((*this)["dependencies"][filename].begin(),
                                                        (*this)["dependencies"][filename].end(),
                                                        id));

      if ((*this)["dependencies"][filename].size() == 0)
         (*this)["dependencies"].erase((*this)["dependencies"].find(filename));
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

std::optional<std::filesystem::path> Analysis::FindAnalysis(std::optional<std::filesystem::path> start) {
   std::filesystem::path target_path;

   if (start.has_value())
      target_path = *start;
   else
      target_path = std::filesystem::current_path();
   
   if (!target_path.has_root_path())
      target_path = std::filesystem::current_path() / target_path;

   target_path = std::filesystem::path(dos_to_unix_path(target_path.lexically_normal().string()));
   auto parent_path = target_path.parent_path();
   bool changed = target_path != parent_path;

   do
   {
      auto metadata = target_path / std::filesystem::path(".mlx/metadata.json");

      MLX_DEBUGN("checking for {}...", metadata.string());

      if (path_exists(metadata))
      {
         try
         {
            auto config = Analysis::Config(metadata);
            auto id = uuids::uuid::from_string(config.analysis_id());

            if (!id.has_value())
               throw exception::InvalidUUID(config.analysis_id());

            return metadata;
         }
         catch (exception::Exception &exc)
         {
         }
      }
      
      target_path = parent_path;
      parent_path = target_path.parent_path();
      changed = target_path != parent_path;
   } while (changed);

   return std::nullopt;
}

Analysis Analysis::FromIdentifier(const std::string &ident) {
   MLX_DEBUGN("searching for metadata file...");
   
   auto path = std::filesystem::path(ident);

   if (path.filename() == "metadata.json" && path_exists(path))
   {
      MLX_DEBUGN("found metadata file.");
      return Analysis(path);
   }

   path /= std::string(".mlx/metadata.json");

   if (path_exists(path))
   {
      MLX_DEBUGN("found metadata file.");
      return Analysis(path);
   }

   MLX_DEBUGN("trying uuid...");
   auto uuid = uuids::uuid::from_string(ident);

   if (uuid.has_value())
   {
      MLX_DEBUGN("found uuid: {}", ident);
      return Analysis(*uuid);
   }

   MLX_DEBUGN("trying alias...");
   auto &db = Database::GetInstance();
   auto alias = db.query("SELECT analysis.analysis_id FROM mlx_analysis_alias AS alias, mlx_analysis AS analysis \
                          WHERE alias.analysis_id = analysis.id AND alias.alias = ?", ident);

   if (alias.size() > 0)
   {
      MLX_DEBUGN("found alias {}", ident);
      uuid = uuids::uuid::from_string(std::get<std::string>(alias[0][0]));
      return Analysis(*uuid);
   }

   MLX_DEBUGN("parse attempts exhausted.");

   throw exception::AnalysisNotFound();
}

bool Analysis::has_config(void) const {
   return this->_open_config.has_value();
}

bool Analysis::has_id(void) const {
   return !this->_id.is_nil();
}

const uuids::uuid &Analysis::id(void) const {
   if (!this->has_id())
      throw exception::NullUUID();

   return this->_id;
}

const uuids::uuid &Analysis::generate_id(void) {
   this->_id = uuids::uuid_system_generator{}();
   this->_config.set_analysis_id(uuids::to_string(this->_id));

   return this->_id;
}

bool Analysis::has_row_id(void) const {
   return this->_row_id.has_value();
}

std::int64_t Analysis::row_id(void) const {
   if (!this->has_row_id())
      throw exception::AnalysisNotSaved(uuids::to_string(this->id()));

   return *this->_row_id;
}

std::filesystem::path Analysis::archive_file(std::optional<uuids::uuid> id) const {
   std::string id_string;

   if (!id.has_value())
      id_string = uuids::to_string(this->id());
   else
      id_string = uuids::to_string(*id);
   
   auto result = std::filesystem::path(MainConfig::GetInstance().analysis_path()) / std::filesystem::path(hash_fanout(id_string));
   result += std::filesystem::path(".zip");
   result = std::filesystem::path(dos_to_unix_path(result.string()));

   return result;
}

bool Analysis::is_saved(void) const {
   return this->has_id() && path_exists(this->archive_file());
}

bool Analysis::is_open(void) const {
   return this->has_id() && this->_open_config.has_value() && path_exists(*this->_open_config);
}

std::filesystem::path Analysis::disk_to_relative(const std::filesystem::path &filename) const {
   auto analysis_root = this->open_path();
   std::filesystem::path relative;

   if (!filename.has_root_path())
   {
      auto current_file = (std::filesystem::current_path() / filename).lexically_normal();

      if (!is_rooted_in(analysis_root, current_file))
         throw exception::FileOutsideAnalysisPath(current_file.string());

      relative = skip_root(analysis_root, current_file);
   }
   else
   {
      if (!is_rooted_in(analysis_root, filename.lexically_normal()))
         throw exception::FileOutsideAnalysisPath(filename.string());
      
      relative = skip_root(analysis_root, filename.lexically_normal());
   }

   relative = std::filesystem::path(dos_to_unix_path(relative.string()));

   return relative;
}

std::filesystem::path Analysis::relative_to_disk(const std::filesystem::path &filename) const {
   auto result = this->open_path() / filename;
   result = std::filesystem::path(dos_to_unix_path(result.string()));

   return result;
}

std::filesystem::path Analysis::default_path(void) const {
   auto result = std::filesystem::path(MainConfig::GetInstance().active_path()) / std::filesystem::path("analysis") / this->label();
   result = std::filesystem::path(dos_to_unix_path(result.string()));

   return result;
}

std::filesystem::path Analysis::config_file(void) const {
   if (!this->has_config())
      throw exception::AnalysisNotOpen(uuids::to_string(this->id()));

   return *this->_open_config;
}

std::filesystem::path Analysis::config_path(void) const {
   return this->config_file().parent_path();
}

std::filesystem::path Analysis::open_path(void) const {
   return this->config_path().parent_path();
}

std::filesystem::path Analysis::active_file(const Sample &sample) const {
   if (!this->has_sample(sample))
      throw exception::SampleNotAnalyzed(sample.label());
   
   auto id = to_hex_string(sample.sha256());
   auto filename = this->_config.get_sample_active_file(id);

   if (!filename.has_value())
      filename = sample.filename();

   filename = dos_to_unix_path(*filename);

   return *filename;
}

std::filesystem::path Analysis::taintable_file(const Sample &sample) const {
   if (!this->has_sample(sample))
      throw exception::SampleNotAnalyzed(sample.label());

   auto result = std::filesystem::path(".mlx/samples") / sample.benign_file();
   result = std::filesystem::path(dos_to_unix_path(result.string()));

   return result;
}

std::map<std::filesystem::path,std::vector<std::uint8_t>> Analysis::active_map(void) const {
   std::map<std::filesystem::path,std::vector<std::uint8_t>> result;

   for (auto &entry : this->_samples)
      result[this->active_file(entry.second)] = entry.first;

   return result;
}

std::map<std::filesystem::path,std::vector<std::uint8_t>> Analysis::taint_map(void) const
{
   std::map<std::filesystem::path,std::vector<std::uint8_t>> result;

   for (auto &entry : this->_samples)
   {
      auto benign_file = entry.second.benign_file();
      auto taint_relative = std::filesystem::path(".mlx/samples") / benign_file;
      taint_relative = std::filesystem::path(dos_to_unix_path(taint_relative.string()));

      result[taint_relative] = entry.first;
   }

   return result;
}

bool Analysis::has_archive(void) const {
   return this->_archive.has_value();
}

Zip &Analysis::archive(int flags) {
   if (!this->_archive.has_value())
   {
      auto archive_file = this->archive_file();

      if ((flags & ZIP_CREATE) != 0 && !path_exists(archive_file.parent_path()) && !std::filesystem::create_directories(archive_file.parent_path()))
         throw exception::CreateDirectoryFailure(archive_file.parent_path().string());
      
      this->_archive = Zip(this->archive_file(), flags);
      this->_archive->set_password(MainConfig::GetInstance().zip_password());
   }

   return *this->_archive;
}

void Analysis::close_archive(void) {
   if (!this->has_archive())
      return;
   
   this->_archive->close();
   this->_archive = std::nullopt;
}

void Analysis::discard_archive(void) {
   if (!this->has_archive())
      return;

   this->_archive->discard();

   if (path_exists(this->archive_file().parent_path()))
      clear_directory(MainConfig::GetInstance().analysis_path(), this->archive_file().parent_path());
   
   this->_archive = std::nullopt;
}

void Analysis::load_from_uuid(const uuids::uuid &id) {
   auto archive_file = this->archive_file(id);

   if (!path_exists(archive_file))
      throw exception::AnalysisNotSaved(uuids::to_string(id));

   this->_id = id;

   auto &db = Database::GetInstance();
   auto row_id_results = db.query("SELECT analysis.id FROM mlx_analysis AS analysis WHERE analysis.analysis_id = ?",
                                  uuids::to_string(id));

   if (row_id_results.size() == 0)
      throw exception::AnalysisNotSaved(uuids::to_string(id));
   
   this->_row_id = std::get<std::int64_t>(row_id_results[0][0]);
   
   auto open_results = db.query("SELECT path FROM mlx_analysis_opened WHERE analysis_id = ?", *this->_row_id);

   if (open_results.size() > 0)
   {
      auto config_file = std::filesystem::path(std::get<std::string>(open_results[0][0])) / std::string(".mlx/metadata.json");

      if (path_exists(config_file))
         this->_open_config = config_file;
   }

   this->load_config();
}

void Analysis::load_from_alias(const std::string &alias) {
   auto &db = Database::GetInstance();
   auto results = db.query("SELECT analysis.analysis_id FROM mlx_analysis_alias AS alias, mlx_analysis AS analysis \
                            WHERE alias.analysis_id = analysis.id AND alias.alias = ?",
                           alias);

   if (results.size() == 0)
      throw exception::AnalysisAliasNotFound(alias);

   auto analysis_id = std::get<std::string>(results[0][0]);
   auto uuid_parse = uuids::uuid::from_string(analysis_id);

   if (!uuid_parse.has_value())
      throw exception::InvalidUUID(analysis_id);

   this->load_from_uuid(*uuid_parse);
}

void Analysis::load_from_config(const std::filesystem::path &filename)
{
   this->_config.from_file(filename);

   auto uuid = uuids::uuid::from_string(this->_config.analysis_id());

   if (!uuid.has_value())
      throw exception::InvalidUUID(this->_config.analysis_id());

   this->_id = *uuid;
   this->_open_config = filename;

   auto &db = Database::GetInstance();
   auto exists = db.query("SELECT analysis.id FROM mlx_analysis AS analysis WHERE analysis.analysis_id = ?", uuids::to_string(*uuid));

   if (exists.size() > 0)
      this->_row_id = std::get<std::int64_t>(exists[0][0]);

   this->load_config();
}

void Analysis::load_config(void) {
   if (this->is_open())
   {
      this->_config = Analysis::Config(this->config_file());
   }
   else if (this->is_saved())
   {
      auto &archive = this->archive(ZIP_RDONLY);
      auto config_data = archive.extract_to_memory(std::string(".mlx/metadata.json"));
      this->_config.parse(std::string(config_data.begin(), config_data.end()));
      this->discard_archive();
   }
   else
      return;

   this->_alias = this->_config.analysis_alias();
   this->_samples.clear();

   for (auto &entry : this->_config.samples())
   {
      auto hash = from_hex_string(entry.first);
      this->_samples[hash] = Sample(hash);
   }
}

void Analysis::save_config(void) {
   if (this->_open_config.has_value())
   {
      auto config_file = this->config_file();

      if (!path_exists(config_file.parent_path()))
         if (!std::filesystem::create_directories(config_file.parent_path()))
            throw exception::CreateDirectoryFailure(config_file.parent_path().string());;

      this->_config.save(config_file);
   }
   else if (this->is_saved())
   {
      auto &archive = this->archive(ZIP_CREATE);
      auto config_string = this->_config.to_string();
      auto config_data = std::vector<std::uint8_t>(config_string.begin(), config_string.end());
      archive.insert_buffer(config_data, ".mlx/metadata.json");
      this->close_archive();
   }
   else
      throw exception::CannotSaveAnalysisConfig();
}

bool Analysis::is_tainted(const Sample &sample) const {
   if (!this->is_open())
      throw exception::AnalysisNotOpen(uuids::to_string(this->id()));
   
   auto taintable = this->taintable_file(sample);
   auto full_taintable = this->open_path() / taintable;
   auto hash = sha256(full_taintable);

   return hash != sample.sha256();
}

bool Analysis::is_tainted(const std::filesystem::path &filename) {
   if (!this->is_saved())
      throw exception::AnalysisNotSaved(uuids::to_string(this->id()));

   if (!this->has_file(filename))
      throw exception::FileNotFound(filename.string());

   std::uint32_t crc;

   if (this->is_linked(filename))
   {
      auto link = this->get_file_link(filename);

      if (!link.has_value())
         throw exception::FileNotLinked(filename.string());
      
      auto analysis = Analysis(link->first);

      if (!analysis.is_saved())
         throw exception::AnalysisNotSaved(uuids::to_string(link->first));
      
      auto &archive = analysis.archive(ZIP_RDONLY);
      auto stat = archive.stat(dos_to_unix_path(link->second.string()));
      crc = stat.crc;
   }
   else
   {
      auto &archive = this->archive(ZIP_RDONLY);
      auto stat = archive.stat(filename.string());
      crc = stat.crc;
   }

   return crc != crc32(this->relative_to_disk(filename));
}

std::vector<std::vector<std::uint8_t>> Analysis::samples(void) const {
   std::vector<std::vector<std::uint8_t>> result;
   
   for (auto &entry : this->_samples)
      result.push_back(entry.first);

   return result;
}

bool Analysis::has_sample(const Sample &sample) const {
   return this->_samples.find(sample.sha256()) != this->_samples.end();
}

Sample &Analysis::get_sample(const std::vector<std::uint8_t> &hash) {
   if (this->_samples.find(hash) == this->_samples.end())
      throw exception::SampleNotAnalyzed(to_hex_string(hash));

   return this->_samples[hash];
}

const Sample &Analysis::get_sample(const std::vector<std::uint8_t> &hash) const {
   if (this->_samples.find(hash) == this->_samples.end())
      throw exception::SampleNotAnalyzed(to_hex_string(hash));

   return this->_samples.at(hash);
}

std::optional<std::string> Analysis::get_sample_active_file(const Sample &sample) const {
   return this->_config.get_sample_active_file(to_hex_string(sample.sha256()));
}

void Analysis::set_sample_active_file(const Sample &sample, std::optional<std::string> filename) {
   this->_config.set_sample_active_file(to_hex_string(sample.sha256()), filename);
}

void Analysis::add_sample(const Sample &sample, std::optional<std::filesystem::path> active_file) {
   if (!this->is_open())
      throw exception::AnalysisNotOpen(uuids::to_string(this->id()));
   
   if (this->has_sample(sample))
      return;

   this->_samples[sample.sha256()] = sample;

   if (active_file.has_value())
      this->_config.add_sample(to_hex_string(sample.sha256()), active_file->string());
   else
      this->_config.add_sample(to_hex_string(sample.sha256()), std::nullopt);
   
   auto taint_file = this->taintable_file(sample);
   active_file = this->active_file(sample);

   auto taint_absolute = this->relative_to_disk(taint_file);
   auto active_absolute = this->relative_to_disk(*active_file);
   auto active_relative = std::filesystem::relative(taint_absolute, active_absolute.parent_path());

   if (!path_exists(taint_absolute.parent_path()))
      if (!std::filesystem::create_directories(taint_absolute.parent_path()))
         throw exception::CreateDirectoryFailure(taint_absolute.parent_path().string());

   if (!path_exists(active_absolute))
   {
      if (std::filesystem::is_symlink(active_absolute))
         erase_file(active_absolute);
      
      sample.extract_to_disk(taint_absolute);
   }
   else
   {
      copy_file(active_absolute, taint_absolute);
      erase_file(active_absolute);
   }

   if (!path_exists(active_absolute.parent_path()))
      if (!std::filesystem::create_directories(active_absolute.parent_path()))
         throw exception::CreateDirectoryFailure(active_absolute.parent_path().string());
   
   std::filesystem::create_symlink(active_relative, active_absolute);
}

void Analysis::remove_sample(const Sample &sample) {
   if (!this->is_open())
      throw exception::AnalysisNotOpen(uuids::to_string(this->id()));

   if (!this->has_sample(sample))
      throw exception::SampleNotAnalyzed(to_hex_string(sample.sha256()));

   auto taint_file = this->taintable_file(sample);
   auto active_file = this->active_file(sample);

   auto taint_absolute = this->open_path() / taint_file;
   auto active_absolute = this->open_path() / active_file;

   if (path_exists(active_absolute))
      erase_file(active_absolute);

   if (path_exists(taint_absolute))
      erase_file(taint_absolute);
   
   this->_config.remove_sample(to_hex_string(sample.sha256()));
   this->_samples.erase(this->_samples.find(sample.sha256()));
}

void Analysis::restore_sample(const Sample &sample) {
   if (!this->is_open())
      throw exception::AnalysisNotOpen(uuids::to_string(this->id()));
   
   if (!this->has_sample(sample))
      throw exception::SampleNotAnalyzed(to_hex_string(sample.sha256()));

   auto taint_file = this->taintable_file(sample);
   auto absolute_file = this->open_path() / taint_file;

   sample.extract_to_disk(absolute_file);
}

void Analysis::restore_taintable_sample(const Sample &sample) {
   if (!this->is_open())
      throw exception::AnalysisNotOpen(uuids::to_string(this->id()));

   this->extract_taintable_sample(sample, std::nullopt);
   this->link_sample(sample);
}

void Analysis::extract_taintable_sample(const Sample &sample, std::optional<std::filesystem::path> outfile)
{
   if (!this->is_saved())
      throw exception::AnalysisNotSaved(uuids::to_string(this->id()));

   auto taint_file = this->taintable_file(sample);
   std::filesystem::path extract_file;

   if (outfile.has_value())
      extract_file = *outfile;
   else
      extract_file = this->relative_to_disk(taint_file);

   if (!extract_file.has_root_path())
      extract_file = std::filesystem::current_path() / extract_file;

   auto &archive = this->archive(ZIP_RDONLY);
   archive.extract_to_disk(taint_file.string(), extract_file);
   this->discard_archive();
}

std::vector<std::uint8_t> Analysis::extract_taintable_sample(const Sample &sample)
{
   if (!this->is_saved())
      throw exception::AnalysisNotSaved(uuids::to_string(this->id()));

   auto taint_file = this->taintable_file(sample);
   auto &archive = this->archive(ZIP_RDONLY);
   auto result = archive.extract_to_memory(taint_file.string());
   this->discard_archive();

   return result;
}

void Analysis::link_sample(const Sample &sample, std::optional<std::filesystem::path> outlink, bool relative) {
   if (!this->is_open())
      throw exception::AnalysisNotOpen(uuids::to_string(this->id()));
   
   auto taint = this->taintable_file(sample);
   auto taint_absolute = this->open_path() / taint;

   if (!path_exists(taint_absolute))
      throw exception::FileNotFound(taint_absolute.string());

   if (!outlink.has_value())
      outlink = this->open_path() / this->active_file(sample);

   if (!outlink->has_root_path())
      outlink = std::filesystem::current_path() / *outlink;

   if (path_exists(*outlink) && std::filesystem::is_symlink(*outlink))
      return;

   std::filesystem::path target_link;

   if (relative)
      target_link = std::filesystem::relative(taint_absolute, outlink->parent_path());
   else
      target_link = taint_absolute;

   if (!path_exists(outlink->parent_path()))
      if (!std::filesystem::create_directories(outlink->parent_path()))
         throw exception::CreateDirectoryFailure(outlink->parent_path().string());

   std::filesystem::create_symlink(target_link, *outlink);
}

void Analysis::unlink_sample(const Sample &sample) {
   if (!this->is_open())
      throw exception::AnalysisNotOpen(uuids::to_string(this->id()));

   auto active = this->active_file(sample);
   auto absolute = this->relative_to_disk(active);

   if (std::filesystem::is_symlink(absolute))
   {
      erase_file(absolute);
      clear_directory(this->open_path(), absolute.parent_path());
   }
}

void Analysis::rename_active_file(const Sample &sample, std::optional<std::string> filename) {
   std::filesystem::path file_check;
   
   if (!filename.has_value())
      file_check = sample.filename();
   else
      file_check = dos_to_unix_path(*filename);

   if (this->has_file(file_check) || this->is_linked(file_check) || this->is_sample(file_check) || path_exists(this->relative_to_disk(file_check)))
      throw exception::FileExists(file_check.string());

   if (this->is_open())
      this->unlink_sample(sample);
   
   this->set_sample_active_file(sample, filename);

   if (this->is_open())
      this->link_sample(sample);
}

std::vector<std::filesystem::path> Analysis::files(void) const {
   std::vector<std::filesystem::path> result;

   auto supplements = this->supplements();
   auto artifacts = this->artifacts();

   result.insert(result.end(), supplements.begin(), supplements.end());

   for (auto &pair : artifacts)
      result.push_back(pair.first);

   return result;
}

std::vector<std::filesystem::path> Analysis::supplements(void) const {
   auto file_map = this->_config.files();
   std::vector<std::filesystem::path> result;
   
   for (auto &entry : file_map)
   {
      auto &relative = entry.first;
      auto &file_data = entry.second;
      auto artifact_data = file_data.first;
      auto link_data = file_data.second;

      if (link_data.has_value() ||
          (this->is_open() && path_exists(this->open_path() / relative) && std::filesystem::is_symlink(this->open_path() / relative)))
         continue;
      
      if (!artifact_data.has_value())
         result.push_back(relative);
   }

   return result;
}

std::vector<std::pair<std::filesystem::path,std::vector<std::uint8_t>>> Analysis::artifacts(void) const {
   auto file_map = this->_config.files();
   std::vector<std::pair<std::filesystem::path,std::vector<std::uint8_t>>> result;
   
   for (auto &entry : file_map)
   {
      auto &relative = entry.first;
      auto &file_data = entry.second;
      auto artifact_data = file_data.first;
      auto link_data = file_data.second;

      if (link_data.has_value() ||
          (this->is_open() && path_exists(this->relative_to_disk(relative)) && std::filesystem::is_symlink(this->relative_to_disk(relative))))
         continue;
      
      if (artifact_data.has_value())
         result.push_back(std::make_pair(std::filesystem::path(relative),from_hex_string(*artifact_data)));
   }

   return result;
}

std::vector<std::filesystem::path> Analysis::taintable_samples(void) const {
   std::vector<std::filesystem::path> result;

   for (auto &entry : this->active_map())
   {
      auto &relative = entry.first;
      auto &hash = entry.second;
      
      if (path_exists(this->relative_to_disk(relative)) && !std::filesystem::is_symlink(this->relative_to_disk(relative)))
         result.push_back(relative);
      else
         result.push_back(this->taintable_file(this->get_sample(hash)));
   }
      
   return result;
}

std::vector<std::filesystem::path> Analysis::links(void) const {
   auto file_map = this->_config.files();
   std::vector<std::filesystem::path> result;
   
   for (auto &entry : file_map)
   {
      auto &relative = entry.first;
      auto &file_data = entry.second;
      auto link_data = file_data.second;

      if (!link_data.has_value() ||
          (this->is_open() && path_exists(this->relative_to_disk(relative)) && !std::filesystem::is_symlink(this->relative_to_disk(relative))))
         continue;

      result.push_back(dos_to_unix_path(relative));
   }

   return result;
}

bool Analysis::has_file(const std::filesystem::path &filename) const {
   return this->_config.has_file(dos_to_unix_path(filename.string()));
}

void Analysis::add_file(const std::filesystem::path &filename,
                        std::optional<Sample> artifact,
                        std::optional<std::pair<uuids::uuid,std::filesystem::path>> link)
{
   if (!this->is_open())
      throw exception::AnalysisNotOpen(uuids::to_string(this->id()));

   if (this->has_file(filename))
      return;

   auto absolute = this->relative_to_disk(filename);

   if (!link.has_value() && !path_exists(absolute))
      throw exception::FileNotFound(absolute.string());
   
   std::optional<std::string> artifact_json = std::nullopt;
   Analysis::Config::FileLink link_json = std::nullopt;

   if (artifact.has_value())
   {
      if (!artifact->is_saved())
         throw exception::SampleNotSaved();
      
      artifact_json = to_hex_string(artifact->sha256());
   }

   if (link.has_value())
   {
      auto linked_analysis = Analysis(link->first);

      if (!linked_analysis.has_file(link->second))
         throw exception::FileNotFound(link->second.string());
      
      link_json = std::make_pair(uuids::to_string(link->first),link->second.string());
   }

   this->_config.add_file(dos_to_unix_path(filename.string()), artifact_json, link_json);

   if (link.has_value())
      this->link_file(filename);
}

bool Analysis::is_supplement(const std::filesystem::path &filename) const {
   return this->has_file(filename) && !this->get_file_artifact(filename).has_value();
}

bool Analysis::is_artifact(const std::filesystem::path &filename) const {
   return this->has_file(filename) && this->get_file_artifact(filename).has_value();
}

bool Analysis::is_artifact_of(const std::filesystem::path &filename, const Sample &sample) const {
   return this->is_artifact(filename) && this->has_sample(sample) && *this->get_file_artifact(filename) == sample.sha256();
}

bool Analysis::is_sample(const std::filesystem::path &filename) const {
   auto unix_filename = dos_to_unix_path(filename.string());
   auto active_map = this->active_map();
   auto taint_map = this->taint_map();

   return active_map.find(unix_filename) != active_map.end() || taint_map.find(unix_filename) != taint_map.end();
}

bool Analysis::is_linked(const std::filesystem::path &filename) const {
   if (!this->is_open())
      return false;
   
   if (!this->has_file(filename))
      return false;

   auto absolute = this->relative_to_disk(filename);
   return this->get_file_link(filename).has_value() || std::filesystem::is_symlink(absolute);
}

std::optional<std::vector<std::uint8_t>> Analysis::get_file_artifact(const std::filesystem::path &filename) const {
   auto artifact = this->_config.get_file_artifact(filename.string());

   if (artifact.has_value())
      return from_hex_string(*artifact);
   else
      return std::nullopt;
}

void Analysis::set_file_artifact(const std::filesystem::path &filename, const Sample &sample) {
   this->_config.set_file_artifact(filename.string(), to_hex_string(sample.sha256()));
}

std::optional<std::pair<uuids::uuid,std::filesystem::path>> Analysis::get_file_link(const std::filesystem::path &filename) const {
   auto result = this->_config.get_file_link(dos_to_unix_path(filename.string()));

   if (!result.has_value())
      return std::nullopt;
   else
   {
      auto id = uuids::uuid::from_string(result->first);

      if (!id.has_value())
         throw exception::InvalidUUID(result->first);

      auto path = std::filesystem::path(dos_to_unix_path(result->second));
      
      return std::make_pair(*id,path);
   }
}

void Analysis::set_file_link(const std::filesystem::path &filename, std::optional<std::pair<uuids::uuid,std::filesystem::path>> link) {
   if (!link.has_value())
      this->_config.set_file_link(dos_to_unix_path(filename.string()), std::nullopt);
   else
      this->_config.set_file_link(dos_to_unix_path(filename.string()),
                                  std::make_pair(uuids::to_string(link->first),
                                                 dos_to_unix_path(link->second.string())));
}

void Analysis::link_file(const std::filesystem::path &filename, bool relative) {
   if (!this->is_open())
      throw exception::AnalysisNotOpen(uuids::to_string(this->id()));

   auto link = this->get_file_link(filename);

   if (!link.has_value())
      throw exception::FileNotLinked(filename.string());

   auto analysis = Analysis(link->first);

   if (!analysis.is_open())
      throw exception::AnalysisNotOpen(uuids::to_string(this->id()));

   auto remote_file = analysis.open_path() / link->second;
   auto local_file = this->open_path() / filename;

   if (path_exists(local_file))
   {
      if (std::filesystem::is_symlink(local_file))
         return;

      throw exception::FileExists(local_file.string());
   }
   
   std::filesystem::path target_link;

   if (relative)
      target_link = std::filesystem::relative(remote_file, local_file.parent_path());
   else
      target_link = remote_file;

   if (!path_exists(local_file.parent_path()))
      if (!std::filesystem::create_directories(local_file.parent_path()))
         throw exception::CreateDirectoryFailure(local_file.parent_path().string());
   
   std::filesystem::create_symlink(target_link, local_file);
}

void Analysis::unlink_file(const std::filesystem::path &filename) {
   auto link = this->get_file_link(filename);

   if (!link.has_value())
      throw exception::FileNotLinked(filename.string());

   auto absolute = this->open_path() / filename;

   if (path_exists(absolute))
   {
      erase_file(absolute);
      clear_directory(this->open_path(), absolute.parent_path());
   }
}

void Analysis::rename_link(const std::filesystem::path &link, const std::filesystem::path &new_link) {
   auto config_link = this->get_file_link(link);
   auto link_artifact = this->get_file_artifact(link);
   std::optional<Sample> sample;

   if (link_artifact.has_value())
      sample = this->get_sample(*link_artifact);
   else
      sample = std::nullopt;

   if (this->is_open())
      this->unlink_file(link);
   
   this->remove_file(link);
   this->add_file(new_link, sample, config_link);

   if (this->is_open())
      this->link_file(new_link);
}

void Analysis::remove_file(const std::filesystem::path &filename) {
   this->_config.remove_file(dos_to_unix_path(filename.string()));
}

void Analysis::restore_file(const std::filesystem::path &filename) {
   if (!this->is_open())
      throw exception::AnalysisNotOpen(uuids::to_string(this->id()));

   if (this->is_sample(filename))
      this->restore_taintable_sample(this->active_map()[filename]);
   else if (this->is_linked(filename))
   {
      auto link = this->get_file_link(filename);

      if (!link.has_value())
         throw exception::FileNotLinked(filename.string());
      
      auto analysis = Analysis(link->first);

      if (!analysis.is_open())
         throw exception::AnalysisNotOpen(uuids::to_string(analysis.id()));

      auto absolute = this->relative_to_disk(filename);

      if (std::filesystem::is_symlink(absolute))
         analysis.restore_file(link->second);
      else
         analysis.extract_file(link->second, absolute);
   }
   else {
      this->extract_file(filename, std::nullopt);
   }
}

void Analysis::rename_file(const std::filesystem::path &current_filename, const std::filesystem::path &new_filename) {
   bool is_file = this->has_file(current_filename);
   
   if (this->is_sample(current_filename))
   {
      auto active_map = this->active_map();
      auto taint_map = this->taint_map();

      if (taint_map.find(current_filename) != taint_map.end())
         throw exception::TaintFileRename();

      auto &hash = active_map[current_filename];
      auto &sample = this->get_sample(hash);

      this->rename_active_file(sample, new_filename.string());
   }
   else if (this->is_linked(current_filename))
      this->rename_link(current_filename, new_filename);
   else {
      /* TODO handle file dependencies on rename */
      auto link = this->get_file_link(current_filename); // should basically be std::nullopt since it's not a link
      auto hash = this->get_file_artifact(current_filename);

      this->remove_file(current_filename);
   
      if (this->is_open())
      {
         auto current_absolute = this->relative_to_disk(current_filename);
         auto new_absolute = this->relative_to_disk(new_filename);
         
         if (!path_exists(new_absolute.parent_path()))
            if (!std::filesystem::create_directories(new_absolute.parent_path()))
               throw exception::CreateDirectoryFailure(new_absolute.parent_path().string());

         malexandria::rename_file(current_absolute, new_absolute);
         clear_directory(this->open_path(), current_absolute.parent_path());
      }
   
      if (hash.has_value())
         this->add_file(new_filename, this->get_sample(*hash), link);
      else
         this->add_file(new_filename, std::nullopt, link);
   }
   
   if (this->is_saved())
   {
      if (this->has_archive())
         this->close_archive();

      auto &archive = this->archive(ZIP_CREATE);

      if (is_file)
         archive.rename_file(dos_to_unix_path(current_filename.string()), dos_to_unix_path(new_filename.string()));

      auto config_data = this->_config.to_string();
      archive.insert_buffer(std::vector<std::uint8_t>(config_data.begin(), config_data.end()), ".mlx/metadata.json");
      this->close_archive();
   }
}

void Analysis::extract_file(const std::filesystem::path &filename, std::optional<std::filesystem::path> outfile) {
   if (!this->is_saved())
      throw exception::AnalysisNotSaved(uuids::to_string(this->id()));
   
   if (!this->has_file(filename))
      throw exception::FileNotAnalyzed(filename.string());

   if (!outfile.has_value())
      outfile = this->relative_to_disk(filename);

   if (!outfile->has_root_path())
      outfile = std::filesystem::current_path() / *outfile;

   if (!path_exists(outfile->parent_path()))
      if (!std::filesystem::create_directories(outfile->parent_path()))
         throw exception::CreateDirectoryFailure(outfile->parent_path().string());
   
   auto &archive = this->archive(ZIP_RDONLY);
   archive.extract_to_disk(dos_to_unix_path(filename.string()), *outfile);
   this->discard_archive();
}

std::vector<std::uint8_t> Analysis::extract_file(const std::filesystem::path &filename) {
   if (!this->is_saved())
      throw exception::AnalysisNotSaved(uuids::to_string(this->id()));

   if (!this->has_file(filename))
      throw exception::FileNotAnalyzed(filename.string());

   auto &archive = this->archive(ZIP_RDONLY);
   auto result = archive.extract_to_memory(dos_to_unix_path(filename.string()));
   this->discard_archive();

   return result;
}

bool Analysis::has_alias(void) const {
   return this->_alias.has_value();
}

std::string &Analysis::alias(void) {
   if (!this->has_alias())
      throw exception::AnalysisAliasNotSet();

   return *this->_alias;
}

const std::string &Analysis::alias(void) const {
   if (!this->has_alias())
      throw exception::AnalysisAliasNotSet();

   return *this->_alias;
}

void Analysis::set_alias(std::optional<std::string> alias) {
   this->_alias = alias;
   this->_config.set_analysis_alias(alias);
}

std::map<std::filesystem::path,std::vector<uuids::uuid>> Analysis::dependencies(void) const {
   std::map<std::filesystem::path,std::vector<uuids::uuid>> result;
   
   for (auto &entry : this->_config.dependencies())
   {
      auto &filename = entry.first;
      auto &analyses = entry.second;

      result[filename] = std::vector<uuids::uuid>();

      for (auto &id : analyses)
      {
         auto uuid_parse = uuids::uuid::from_string(id);

         if (!uuid_parse.has_value())
            throw exception::InvalidUUID(id);

         result[filename].push_back(*uuid_parse);
      }
   }

   return result;
}

bool Analysis::has_dependencies(const std::filesystem::path &filename) const {
   return this->_config.has_dependencies(dos_to_unix_path(filename.string()));
}

bool Analysis::has_dependency(const std::filesystem::path &filename, const uuids::uuid &id) const {
   return this->_config.has_dependency(dos_to_unix_path(filename.string()), uuids::to_string(id));
}

std::vector<uuids::uuid> Analysis::get_dependencies(const std::filesystem::path &filename) const {
   auto dependencies = this->_config.get_dependencies(dos_to_unix_path(filename.string()));
   std::vector<uuids::uuid> result;

   for (auto &dep : dependencies)
   {
      auto uuid_parse = uuids::uuid::from_string(dep);

      if (!uuid_parse.has_value())
         throw exception::InvalidUUID(dep);

      result.push_back(*uuid_parse);
   }

   return result;
}

void Analysis::add_dependency(const std::filesystem::path &filename, const uuids::uuid &id) {
   this->_config.add_dependency(dos_to_unix_path(filename.string()), uuids::to_string(id));
}

void Analysis::remove_dependency(const std::filesystem::path &filename, const uuids::uuid &id) {
   this->_config.remove_dependency(dos_to_unix_path(filename.string()), uuids::to_string(id));
}

std::string Analysis::label(void) const {
   if (this->has_alias())
      return this->alias();
   else
      return uuids::to_string(this->id());
}

std::filesystem::path Analysis::create(std::optional<std::filesystem::path> root_directory) {
   if (this->is_saved() || this->is_open())
      throw exception::AnalysisAlreadyCreated();

   MLX_DEBUG("generating analysis id...");
   this->generate_id();
   Logger::Raw(Logger::Level::Debug, "{}\n", uuids::to_string(this->_id));

   if (!root_directory.has_value())
      root_directory = this->default_path();

   if (!root_directory->has_root_path())
      root_directory = std::filesystem::current_path() / *root_directory;

   root_directory = root_directory->lexically_normal();

   auto config_file = *root_directory / std::filesystem::path(".mlx/metadata.json");
   config_file = std::filesystem::path(dos_to_unix_path(config_file.string()));

   if (!path_exists(config_file.parent_path()))
      if (!std::filesystem::create_directories(config_file.parent_path()))
         throw exception::CreateDirectoryFailure(config_file.parent_path().string());

   MLX_DEBUGN("creating analysis config skeleton...");

   if (this->has_alias())
      this->_config["analysis_alias"] = this->alias();
   else
      this->_config["analysis_alias"] = nullptr;
   
   this->_config["samples"] = {};
   this->_config["files"] = {};
   this->_config["dependencies"] = {};

   this->_open_config = config_file;

   MLX_DEBUGN("saving config skeleton to appear open...");
   this->save_config();

   MLX_DEBUGN("poking notes file...");
   auto notes_file = this->relative_to_disk(MainConfig::GetInstance().notes_file());
   std::ofstream notes_touch(notes_file.string());
   notes_touch.close();

   MLX_DEBUGN("adding note as a supplement...");
   this->add_file(this->disk_to_relative(notes_file));

   MLX_DEBUGN("saving analysis config to {}...", this->config_file().string());
   this->save_config();

   MLX_DEBUGN("done creating analysis repo.");

   return this->open_path();
}

void Analysis::save(void) {
   if (!this->is_open())
      throw exception::AnalysisNotOpen(uuids::to_string(this->id()));

   auto &db = Database::GetInstance();
   MLX_DEBUGN("checking alias state...");

   if (this->has_alias())
   {
      auto alias_taken = db.query("SELECT analysis.analysis_id FROM mlx_analysis_alias AS alias, mlx_analysis AS analysis \
                                   WHERE alias.analysis_id = analysis.id AND alias.alias = ?", this->alias());

      if (alias_taken.size() > 0)
      {
         auto found_id = std::get<std::string>(alias_taken[0][0]);
         auto found_parsed = uuids::uuid::from_string(found_id);

         if (!this->has_id() || found_parsed != this->id())
            throw exception::AnalysisAliasTaken(this->alias());
      }
   }  

   MLX_DEBUGN("saving analysis for {}...", this->label());

   if (this->has_archive())
      this->close_archive();

   auto &archive = this->archive(ZIP_CREATE);
   auto root = this->open_path();

   try {
      MLX_DEBUGN("saving config...");
      this->save_config();

      auto file_tree = archive.file_tree();
      MLX_DEBUGN("checking which files were deleted...");
      std::vector<std::string> deleted_files, ghosted_files;

      for (auto &tree_entry : file_tree)
      {
         auto zip_folder = tree_entry.first;
         auto &file_data = tree_entry.second;

         for (auto &file_entry : file_data)
         {
            MLX_DEBUG("checking {}...", file_entry.second);

            auto file_path = root / file_entry.second;

            if (this->disk_to_relative(this->config_file()) == file_entry.second)
               Logger::Raw(Logger::Level::Debug, "metadata.\n");
            else if (path_exists(file_path))
               Logger::Raw(Logger::Level::Debug, "exists.\n");
            else if (this->has_file(file_entry.second))
            {
               Logger::Raw(Logger::Level::Debug, "deleted, but not removed. File will be restored on next open.\n");
               ghosted_files.push_back(file_entry.second);
            }
            else if (this->is_sample(file_entry.second))
            {
               Logger::Raw(Logger::Level::Debug, "sample deleted, emitting warning.\n");
               Logger::NoticeN("Warning: sample file {} was deleted. It will be restored on next open.", file_entry.second);
               ghosted_files.push_back(file_entry.second);
            }
            else {
               Logger::Raw(Logger::Level::Debug, "deleted and removed.\n");
               archive.delete_file(static_cast<std::uint64_t>(file_entry.first));
               deleted_files.push_back(file_entry.second);
            }
         }

         /* TODO: links */
      }

      MLX_DEBUGN("checking which deleted file directories are now empty...");

      for (auto &deleted_file : deleted_files)
      {
         auto deleted_path = std::filesystem::path(deleted_file);
         auto rooted_path = root / deleted_path;
         auto target_root = rooted_path.parent_path();

         while (target_root != root) {
            if (!path_exists(target_root) || directory_is_empty(target_root))
            {
               MLX_DEBUGN("{} is empty.", target_root.string());
               auto skipped_target = skip_root(root, target_root);
               auto zip_name = dos_to_unix_path(skipped_target.string()) + std::string("/");
               archive.delete_file(zip_name);

               if (path_exists(target_root))
                  erase_file(target_root);
            }

            target_root = target_root.parent_path();
         }
      }

      MLX_DEBUGN("adding config to archive...");
      archive.create_directories(".mlx");
      archive.insert_file(this->config_file(), ".mlx/metadata.json");

      for (auto &filename : this->files())
      {
         auto unix_relative = dos_to_unix_path(filename.string());
         MLX_DEBUGN("adding file {}...", unix_relative);

         if (std::find(ghosted_files.begin(), ghosted_files.end(), unix_relative) != ghosted_files.end())
         {
            MLX_DEBUGN("file {} was deleted but not removed, skipping.", unix_relative);
            continue;
         }
            
         archive.create_directories(std::filesystem::path(unix_relative));
         archive.insert_file(root / unix_relative, unix_relative);
      }

      for (auto &entry : this->_samples)
      {
         auto active_file = this->active_file(entry.second);
         auto active_absolute = this->relative_to_disk(active_file);
         auto sample_zip = this->taintable_file(entry.second);
         std::filesystem::path sample_disk;

         if (path_exists(active_absolute) && !std::filesystem::is_symlink(active_absolute))
         {
            MLX_DEBUGN("sample {} wasn't linked, consuming it as the taintable sample and linking it next open.", entry.second.label());
            sample_disk = active_absolute;
         }
         else if (path_exists(this->relative_to_disk(sample_zip)))
            sample_disk = this->relative_to_disk(sample_zip);
         else if (std::find(ghosted_files.begin(), ghosted_files.end(), active_file.string()) != ghosted_files.end() ||
                  std::find(ghosted_files.begin(), ghosted_files.end(), sample_zip.string()) != ghosted_files.end())
         {
            if (this->is_saved())
            {
               MLX_DEBUGN("sample {} was deleted but not removed, it will be restored next open.", entry.second.label());
               continue;
            }

            throw exception::TaintSampleRemoved(to_hex_string(entry.second.sha256()));
         }

         MLX_DEBUGN("saving sample {} to {}...", sample_disk.string(), sample_zip.string());
         archive.insert_file(sample_disk, sample_zip.string());
      }

      MLX_DEBUGN("closing zip archive...");
      this->close_archive();
   }
   catch (exception::Exception &exc)
   {
      MLX_DEBUGN("encountered exception while saving: {}", exc.what());
      this->discard_archive();
      throw exc;
   }

   MLX_DEBUGN("{} saved to disk: {}", this->label(), this->archive_file().string());

   MLX_DEBUGN("checking if {} is in the database...", uuids::to_string(this->id()));

   auto has_id = db.query("SELECT analysis.id FROM mlx_analysis AS analysis WHERE analysis.analysis_id = ?",
                          uuids::to_string(this->id()));

   if (has_id.size() > 0)
   {
      this->_row_id = std::get<std::int64_t>(has_id[0][0]);
      MLX_DEBUGN("found row ID: {}", *this->_row_id);
   }
   else {
      MLX_DEBUGN("analysis is new. creating entry...");
      db.query("INSERT INTO mlx_analysis (analysis_id) VALUES (?)", uuids::to_string(this->id()));
      this->_row_id = db.last_insert_rowid();
      MLX_DEBUGN("insert id is {}.", *this->_row_id);
   }

   auto is_open = db.query("SELECT opened.path FROM mlx_analysis_opened AS opened WHERE opened.analysis_id = ?", *this->_row_id);

   if (is_open.size() == 0)
   {
      MLX_DEBUGN("marking analysis as open in the database...");
      db.query("INSERT INTO mlx_analysis_opened (analysis_id, path) VALUES (?, ?)",
               *this->_row_id,
               this->open_path().string());
   }

   auto db_sample_query = db.query("SELECT sample.id, sample.sha256 FROM mlx_samples AS sample, mlx_analysis_samples AS analysis \
                                    WHERE analysis.sample_id = sample.id AND analysis.analysis_id = ?", *this->_row_id);
   
   std::map<std::vector<std::uint8_t>,std::int64_t> db_samples;
   std::set<std::vector<std::uint8_t>> db_sample_set;

   if (db_sample_query.size() > 0)
   {
      for (auto &entry : db_sample_query)
      {
         auto hexvec = from_hex_string(std::get<std::string>(entry[1]));
         db_samples.insert(std::make_pair(hexvec, std::get<std::int64_t>(entry[0])));
         db_sample_set.insert(hexvec);
      }
   }

   std::set<std::vector<std::uint8_t>> analysis_set;

   for (auto &entry : this->_samples)
      analysis_set.insert(entry.second.sha256());

   std::vector<std::vector<std::uint8_t>> new_samples, deleted_samples;

   std::set_difference(db_sample_set.begin(), db_sample_set.end(),
                       analysis_set.begin(), analysis_set.end(),
                       std::back_inserter(deleted_samples));

   std::set_difference(analysis_set.begin(), analysis_set.end(),
                       db_sample_set.begin(), db_sample_set.end(),
                       std::back_inserter(new_samples));

   MLX_DEBUGN("{} new samples, {} deleted samples.", new_samples.size(), deleted_samples.size());

   if (new_samples.size() > 0)
   {
      for (auto &sample_id : new_samples)
      {
         auto sample_string = to_hex_string(sample_id);
         auto &sample = this->_samples[sample_id];

         MLX_DEBUGN("inserting {} as sample of {}...", sample.label(), this->label());
         db.query("INSERT INTO mlx_analysis_samples (analysis_id, sample_id) VALUES (?,?)",
                  *this->_row_id,
                  sample.row_id());
      }
   }

   if (deleted_samples.size() > 0)
   {
      for (auto &sample_id : deleted_samples)
      {
         auto row_id = db_samples[sample_id];

         MLX_DEBUGN("deleting hash {} from analysis {}...", to_hex_string(sample_id), this->label());
         db.query("DELETE FROM mlx_analysis_samples WHERE analysis_id = ? AND sample_id = ?",
                  *this->_row_id,
                  row_id);
      }
   }

   MLX_DEBUGN("getting current alias...");
   auto current_alias = db.query("SELECT alias.alias FROM mlx_analysis_alias AS alias WHERE alias.analysis_id=?", *this->_row_id);

   if (current_alias.size() == 0)
   {
      if (!this->has_alias())
      {
         MLX_DEBUGN("no alias set and no alias present, nothing to do.");
         return;
      }
      
      MLX_DEBUGN("creating new alias {}...", this->alias());
      db.query("INSERT INTO mlx_analysis_alias (analysis_id, alias) VALUES (?,?)",
               *this->_row_id,
               this->alias());
   }
   else if (!this->has_alias())
   {
      MLX_DEBUGN("unsetting alias for {}...", uuids::to_string(this->id()));
      db.query("DELETE FROM mlx_analysis_alias WHERE analysis_id=?", *this->_row_id);
   }
   else if (this->alias() != std::get<std::string>(current_alias[0][0]))
   {
      MLX_DEBUGN("alias changed, updating to {}...", uuids::to_string(this->id()));
      db.query("UPDATE mlx_analysis_alias SET alias=? WHERE analysis_id=?",
               this->alias(),
               *this->_row_id);
   }

   MLX_DEBUGN("finished saving analysis.");
}

void Analysis::close(void) {
   MLX_DEBUGN("closing analysis for {}...", this->label());

   if (this->is_open())
   {
      for (auto &file : this->files())
      {
         auto absolute = this->relative_to_disk(file);
            
         if (!path_exists(absolute))
         {
            MLX_DEBUGN("file {} ghosted, skipping.", file.string());
            continue;
         }

         MLX_DEBUGN("deleting {}...", absolute.string());
         erase_file(absolute);
         clear_directory(this->open_path(), absolute.parent_path());
      }

      for (auto &link : this->links())
      {
         auto absolute = this->relative_to_disk(link);

         if (!path_exists(absolute) && !std::filesystem::is_symlink(absolute))
         {
            MLX_DEBUGN("link {} already removed, skipping.", absolute.string());
            continue;
         }

         MLX_DEBUGN("deleting {}...", absolute.string());
         erase_file(absolute);
         clear_directory(this->open_path(), absolute.parent_path());
      }

      for (auto &entry : this->_samples)
      {
         auto &hash = entry.first;
         auto &sample = entry.second;
         auto taintable = this->relative_to_disk(this->taintable_file(sample));

         this->unlink_sample(sample);

         if (path_exists(taintable)) {
            erase_file(taintable);
            clear_directory(this->open_path(), taintable.parent_path());
         }
      }
      
      if (this->_open_config.has_value())
      {
         auto open_path = this->open_path();
         auto parent_path = open_path.parent_path();

         if (path_exists(this->config_file()))
         {
            MLX_DEBUGN("removing config file...");
            erase_file(this->config_file());
      
            MLX_DEBUGN("wiping final directories...");
            clear_directory(parent_path, this->config_file().parent_path());
         }
      }
   }

   MLX_DEBUGN("marking {} closed in database...", this->label());
   auto &db = Database::GetInstance();

   if (!this->_row_id.has_value())
   {
      auto row_id = db.query("SELECT id FROM mlx_analysis WHERE analysis_id=?", uuids::to_string(this->id()));

      if (row_id.size() > 0)
         this->_row_id = std::get<std::int64_t>(row_id[0][0]);
   }
         
   db.query("DELETE FROM mlx_analysis_opened WHERE analysis_id=?", *this->_row_id);

   MLX_DEBUGN("analysis {} closed.", this->label());
}

std::filesystem::path Analysis::open(std::optional<std::filesystem::path> open_path) {
   if (!this->is_saved())
      throw exception::AnalysisNotSaved(uuids::to_string(this->id()));

   if (!open_path.has_value())
      open_path = this->default_path();
   
   if (!path_exists(*open_path))
      if (!std::filesystem::create_directories(*open_path))
         throw exception::CreateDirectoryFailure(open_path->string());

   this->_open_config = *open_path / std::string(".mlx/metadata.json");

   MLX_DEBUGN("saving config to disk...");
   this->save_config();

   for (auto &file : this->files())
   {
      MLX_DEBUGN("extracting {}...", file.string());
      this->extract_file(file, std::nullopt);
   }

   for (auto &link : this->links())
   {
      MLX_DEBUGN("linking {}...", link.string());
      this->link_file(link);
   }

   for (auto &entry : this->_samples)
   {
      auto &hash = entry.first;
      auto &sample = entry.second;

      MLX_DEBUGN("extracting and linking {}...", sample.label());
      this->extract_taintable_sample(sample, std::nullopt);
      this->link_sample(sample);
   }

   MLX_DEBUGN("marking {} as opened...", this->label());
   Database::GetInstance().query("INSERT INTO mlx_analysis_opened (analysis_id, path) VALUES (?,?)",
                                 *this->_row_id,
                                 this->open_path().string());
   
   MLX_DEBUGN("analysis {} opened.", this->label());
   
   return *open_path;
}

void Analysis::erase(void) {
   if (!this->is_saved())
      throw exception::AnalysisNotSaved(uuids::to_string(this->id()));

   auto archive_file = this->archive_file();

   if (path_exists(archive_file))
   {
      erase_file(archive_file);
      clear_directory(MainConfig::GetInstance().analysis_path(), archive_file.parent_path());
   }

   if (this->_row_id.has_value())
   {
      auto &db = Database::GetInstance();

      db.query("DELETE FROM mlx_analysis_samples WHERE analysis_id=?", *this->_row_id);
      db.query("DELETE FROM mlx_analysis_alias WHERE analysis_id=?", *this->_row_id);
      db.query("DELETE FROM mlx_analysis WHERE id=?", *this->_row_id);
   }
}
