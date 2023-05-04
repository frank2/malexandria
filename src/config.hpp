#ifndef __MALEXANDRIA_CONFIG_HPP
#define __MALEXANDRIA_CONFIG_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <fstream>
#include <memory>
#include <string>

#include <nlohmann/json.hpp>

#include "exception.hpp"
#include "fs.hpp"
#include "platform.hpp"

using json = nlohmann::json;

namespace malexandria
{
   class Config
   {
      json _data;

   public:
      Config(void) {}
      Config(const json &data) : _data(data) {}
      Config(const std::filesystem::path &filename) { this->from_file(filename); }
      Config(const Config &other) : _data(other._data) {}

      Config &operator=(const Config &other) {
         this->_data = other._data;

         return *this;
      }
      Config &operator=(const json &data) {
         this->_data = data;

         return *this;
      }

      json &operator[](const json &key);
      const json &operator[](const json &key) const;

      json &data(void) { return this->_data; }
      const json &data(void) const { return this->_data; }
      void set_data(const json &data) { this->_data = data; }

      void from_file(const std::filesystem::path &filename);
      void parse(const std::string &str);
      void save(const std::filesystem::path &filename) const;
      std::string to_string(void) const;
   };

   class MainConfig : public Config
   {
      static std::unique_ptr<MainConfig> Instance;

      MainConfig(void);

   public:
      static MainConfig &GetInstance(void);

      void save() const;
      
      std::string vault_path() const;
      void set_vault_path(const std::string &path);
      
      std::string active_path() const;
      void set_active_path(const std::string &path);
      
      std::string database() const;
      void set_database(const std::string &path);
      
      std::string zip_password() const;
      void set_zip_password(const std::string &password);
      
      std::string benign_extension() const;
      void set_benign_extension(const std::string &ext);

      std::string notes_file() const;
      void set_notes_file(const std::string &file);

      std::optional<std::string> log_level() const;
      void set_log_level(const std::string &level);

      bool has_ssh_config(const std::string &config) const;
      std::string ssh_host(const std::string &config) const;
      void set_ssh_host(const std::string &config, const std::string &host);
      std::optional<std::string> ssh_user(const std::string &config) const;
      void set_ssh_user(const std::string &config, const std::string &user);
      std::optional<std::uint16_t> ssh_port(const std::string &config) const;
      void set_ssh_port(const std::string &config, std::uint16_t port);
      std::optional<std::string> ssh_keyfile(const std::string &config) const;
      void set_ssh_keyfile(const std::string &config, const std::string &keyfile);
   };
}
         
#endif
