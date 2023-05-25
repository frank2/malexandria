#ifndef __MALEXANDRIA_ARGS_CONFIG_HPP
#define __MALEXANDRIA_ARGS_CONFIG_HPP

#include <fmt/core.h>

#include "args/module.hpp"
#include "../config.hpp"

namespace malexandria
{
   struct ConfigLeaf : public Module
   {
      ConfigLeaf(const std::string &name, const std::string &description)
         : Module(name, description)
      {
         this->add_argument("value")
            .help(fmt::format("The value to set for option {}.", name))
            .nargs(0, 1);
      }

      virtual bool execute(Module &root) {
         auto value = this->present("value");

         if (!value.has_value())
            return this->print_current_value();
         else
            return this->set_value(*value);
      }

      virtual bool print_current_value(void) {
         return false;
      }

      virtual bool set_value(const std::string &value) {
         return false;
      }
   };
   
   struct ConfigModule : public Module
   {
      struct BenignExtensionLeaf : public ConfigLeaf
      {
         BenignExtensionLeaf()
            : ConfigLeaf("benign_extension",
                         "Set or get the benign extension of samples stored in the database.")
         {}
         
         bool print_current_value(void) { std::cout << MainConfig::GetInstance().benign_extension() << std::endl; return true; }
         bool set_value(const std::string &value) {
            MainConfig::GetInstance().set_benign_extension(value);
            MainConfig::GetInstance().save();
            return true;
         }
      };

      struct LogLevelLeaf : public ConfigLeaf
      {
         LogLevelLeaf()
            : ConfigLeaf("log_level",
                         "Set or show the current log level. Options are: silent, fatal, notice, info, debug.")
         {}

         bool print_current_value(void) {
            auto log_level = MainConfig::GetInstance().log_level();

            if (!log_level.has_value())
               return false;

            std::cout << *log_level << std::endl;

            return true;
         }
         bool set_value(const std::string &value) {
            MainConfig::GetInstance().set_log_level(value);
            MainConfig::GetInstance().save();
            return true;
         }
      };

      struct NotesFileLeaf : public ConfigLeaf
      {
         NotesFileLeaf()
            : ConfigLeaf("notes_file",
                         "Set or get the name of the default notes file for created analysis.")
         {}
         
         bool print_current_value(void) { std::cout << MainConfig::GetInstance().notes_file() << std::endl; return true; }
         bool set_value(const std::string &value) {
            MainConfig::GetInstance().set_notes_file(value);
            MainConfig::GetInstance().save();
            return true;
         }
      };

      struct PathsModule : public Module
      {
         struct ActiveLeaf : public ConfigLeaf
         {
            ActiveLeaf()
               : ConfigLeaf("active",
                            "Set or print the path where samples are extracted and considered hazardous and active.")
            {}

            bool print_current_value() { std::cout << MainConfig::GetInstance().active_path() << std::endl; return true; }
            bool set_value(const std::string &value) {
               MainConfig::GetInstance().set_active_path(value);
               MainConfig::GetInstance().save();
               return true;
            }
         };

         struct DatabaseLeaf : public ConfigLeaf
         {
            DatabaseLeaf()
               : ConfigLeaf("database",
                            "Set or print the file where the SQLite database will reside.")
            {}

            bool print_current_value() { std::cout << MainConfig::GetInstance().database() << std::endl; return true; }
            bool set_value(const std::string &value) {
               MainConfig::GetInstance().set_database(value);
               MainConfig::GetInstance().save();
               return true;
            }
         };

         struct VaultLeaf : public ConfigLeaf
         {
            VaultLeaf()
               : ConfigLeaf("vault",
                            "Set or print the path where samples and analysis are encrypted and isolated.")
            {}

            bool print_current_value() { std::cout << MainConfig::GetInstance().vault_path() << std::endl; return true; }
            bool set_value(const std::string &value) {
               MainConfig::GetInstance().set_vault_path(value);
               MainConfig::GetInstance().save();
               return true;
            }
         };
         
         PathsModule()
            : Module("paths",
                     "Configure or print the various related paths for Malexandria.",
                     {
                        std::make_shared<ActiveLeaf>(),
                        std::make_shared<DatabaseLeaf>(),
                        std::make_shared<VaultLeaf>()
                     })
         {}
      };

      struct ZipPasswordLeaf : public ConfigLeaf
      {
         ZipPasswordLeaf()
            : ConfigLeaf("zip_password",
                         "Set or get the current password for any encrypted archives produced by Malexandria.")
         {}
         
         bool print_current_value(void) { std::cout << MainConfig::GetInstance().zip_password() << std::endl; return true; }
         bool set_value(const std::string &value) {
            MainConfig::GetInstance().set_zip_password(value);
            MainConfig::GetInstance().save();
            return true;
         }
      };

      ConfigModule()
         : Module("config",
                  "Print or set various settings within the Malexandria config file.",
                  {
                     std::make_shared<BenignExtensionLeaf>(),
                     std::make_shared<LogLevelLeaf>(),
                     std::make_shared<NotesFileLeaf>(),
                     std::make_shared<PathsModule>(),
                     std::make_shared<ZipPasswordLeaf>()
                  })
      {}
   };
}
#endif
