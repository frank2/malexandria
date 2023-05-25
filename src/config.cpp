#include "config.hpp"

using namespace malexandria;

json &Config::operator[](const json &key) {
   return this->_data[key];
}

const json &Config::operator[](const json &key) const {
   return this->_data[key];
}

bool Config::has_key(const json &data) const {
   return this->_data.find(data) != this->_data.end();
}

void Config::from_file(const std::filesystem::path &filename) {
   if (!path_exists(filename))
      throw exception::FileNotFound(filename.string());
   
   std::ifstream fp(filename.string());

   if (!fp)
      throw exception::OpenFileFailure(filename.string());

   try {
      this->_data = json::parse(fp);
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void Config::parse(const std::string &str) {
   try {
      this->_data = json::parse(str);
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void Config::save(const std::filesystem::path &filename) const {
   std::ofstream fp(filename.string());

   if (!fp)
      throw exception::OpenFileFailure(filename.string());

   fp << std::setw(4) << this->_data << std::endl;
   fp.close();
}

std::string Config::to_string(void) const {
   return this->_data.dump(4);
}

std::unique_ptr<MainConfig> MainConfig::Instance = nullptr;

MainConfig::MainConfig(void) : Config() {
   const char *env_file = nullptr;
   std::string config_file;
   
   if (env_file = std::getenv("MALEXANDRIA_CONFIG"))
      config_file = env_file;
   else {
#if defined(MALEXANDRIA_WIN32)
      const char *env_root = std::getenv("USERPROFILE");
#else
      const char *env_root = std::getenv("HOME");
#endif

      if (env_root == nullptr)
         throw exception::NoHomeDirectory();

      config_file = std::string(env_root) + std::string("/.malexandria/config.json");
   }

   if (!path_exists(config_file))
   {
      if (env_file != nullptr)
         throw exception::FileNotFound(config_file);

      std::filesystem::path config_path(config_file);
      auto path_root = config_path.parent_path();

      if (!std::filesystem::create_directories(path_root))
         throw exception::CreateDirectoryFailure(path_root.string());

      json default_config;

      default_config["paths"]["vault"] = path_root.string() + std::string("/vault");
      default_config["paths"]["active"] = path_root.string() + std::string("/active");
      default_config["paths"]["analysis"] = path_root.string() + std::string("/analysis");
      default_config["paths"]["database"] = path_root.string() + std::string("/db.sqlite3");
      default_config["zip_password"] = "infected";
      default_config["benign_extension"] = "000";
      default_config["notes_file"] = "notes.md";
         
      std::ofstream fp(config_file);
      fp << std::setw(4) << default_config << std::endl;

      fp.close();
   }

   this->from_file(config_file);
}

MainConfig &MainConfig::GetInstance(void) {
   if (MainConfig::Instance == nullptr)
      MainConfig::Instance = std::unique_ptr<MainConfig>(new MainConfig());

   return *MainConfig::Instance;
}

void MainConfig::save(void) const {
   const char *env_file = nullptr;
   std::string config_file;
   
   if (env_file = std::getenv("MALEXANDRIA_CONFIG"))
      config_file = env_file;
   else {
#if defined(MALEXANDRIA_WIN32)
      const char *env_root = std::getenv("USERPROFILE");
#else
      const char *env_root = std::getenv("HOME");
#endif

      if (env_root == nullptr)
         throw exception::NoHomeDirectory();

      config_file = std::string(env_root) + std::string("/.malexandria/config.json");
   }

   Config::save(config_file);
}

std::string MainConfig::vault_path(void) const {
   try {
      return (*this)["paths"]["vault"].get<std::string>();
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void MainConfig::set_vault_path(const std::string &path) {
   (*this)["paths"]["vault"] = path;
   this->save();
}

std::string MainConfig::active_path(void) const {
   try {
      return (*this)["paths"]["active"].get<std::string>();
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void MainConfig::set_active_path(const std::string &path) {
   (*this)["paths"]["active"] = path;
   this->save();
}

std::string MainConfig::analysis_path(void) const {
   try {
      return (*this)["paths"]["analysis"].get<std::string>();
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void MainConfig::set_analysis_path(const std::string &path) {
   (*this)["paths"]["analysis"] = path;
   this->save();
}

std::string MainConfig::database(void) const {
   try {
      return (*this)["paths"]["database"].get<std::string>();
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void MainConfig::set_database(const std::string &path) {
   (*this)["paths"]["database"] = path;
   this->save();
}

std::string MainConfig::zip_password(void) const {
   try {
      return (*this)["zip_password"].get<std::string>();
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void MainConfig::set_zip_password(const std::string &password) {
   (*this)["zip_password"] = password;
   this->save();
}

std::string MainConfig::benign_extension(void) const {
   try {
      return (*this)["benign_extension"].get<std::string>();
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}

void MainConfig::set_benign_extension(const std::string &ext) {
   (*this)["benign_extension"] = ext;
   this->save();
}

std::string MainConfig::notes_file() const {
   try {
      return (*this)["notes_file"].get<std::string>();
   }
   catch (std::exception &exc) {
      throw exception::JSONException(exc);
   }
}
      
void MainConfig::set_notes_file(const std::string &file) {
   (*this)["notes_file"] = file;
   this->save();
}

std::optional<std::string> MainConfig::log_level() const {
   if (this->data().find("log_level") == this->data().end())
      return std::nullopt;

   return (*this)["log_level"];
}

void MainConfig::set_log_level(const std::string &level) {
   if (level != "silent" &&
       level != "fatal" &&
       level != "notice" &&
       level != "info" &&
       level != "debug")
      throw exception::UnknownLogLevel(level);
   
   (*this)["log_level"] = level;
}

bool MainConfig::has_ssh_config(const std::string &config) const {
   return (*this)["transport"]["ssh"].find(config) != (*this)["transport"]["ssh"].end();
}

std::string MainConfig::ssh_host(const std::string &config) const {
   if (!this->has_ssh_config(config))
      throw exception::NoSSHConfig(config);

   if ((*this)["transport"]["ssh"][config].find("host") == (*this)["transport"]["ssh"][config].end())
      throw exception::NoSSHHost(config);

   return (*this)["transport"]["ssh"][config]["host"];
}

void MainConfig::set_ssh_host(const std::string &config, const std::string &host) {
   (*this)["transport"]["ssh"][config]["host"] = host;
}

std::optional<std::string> MainConfig::ssh_user(const std::string &config) const {
   if (!this->has_ssh_config(config))
      throw exception::NoSSHConfig(config);

   if ((*this)["transport"]["ssh"][config].find("user") == (*this)["transport"]["ssh"][config].end())
      return std::nullopt;

   return (*this)["transport"]["ssh"][config]["user"];
}

void MainConfig::set_ssh_user(const std::string &config, const std::string &user) {
   (*this)["transport"]["ssh"][config]["user"] = user;
}

std::optional<std::uint16_t> MainConfig::ssh_port(const std::string &config) const {
   if (!this->has_ssh_config(config))
      throw exception::NoSSHConfig(config);

   if ((*this)["transport"]["ssh"][config].find("port") == (*this)["transport"]["ssh"][config].end())
      return std::nullopt;

   return (*this)["transport"]["ssh"][config]["port"].get<std::uint16_t>();
}

void MainConfig::set_ssh_port(const std::string &config, std::uint16_t port) {
   (*this)["transport"]["ssh"][config]["port"] = port;
}

std::optional<std::string> MainConfig::ssh_keyfile(const std::string &config) const {
   if (!this->has_ssh_config(config))
      throw exception::NoSSHConfig(config);

   if ((*this)["transport"]["ssh"][config].find("keyfile") == (*this)["transport"]["ssh"][config].end())
      return std::nullopt;

   return (*this)["transport"]["ssh"][config]["keyfile"];
}

void MainConfig::set_ssh_keyfile(const std::string &config, const std::string &keyfile) {
   (*this)["transport"]["ssh"][config]["keyfile"] = keyfile;
}
