#ifndef __MALEXANDRIA_ARGS_MODULE_HPP
#define __MALEXANDRIA_ARGS_MODULE_HPP

#include <memory>
#include <string>
#include <vector>

#include <argparse/argparse.hpp>

#include "exception.hpp"
#include "logger.hpp"
#include "utility.hpp"

namespace malexandria
{
   struct Module : public argparse::ArgumentParser
   {
      std::vector<std::shared_ptr<Module>> submodules;

      Module() : argparse::ArgumentParser() {}
      Module(const std::string &name,
             const std::string &description,
             std::vector<std::shared_ptr<Module>> submodules={},
             const argparse::default_arguments &defaults=argparse::default_arguments::help)
         : argparse::ArgumentParser(name, MALEXANDRIA_VERSION, defaults)
      {
         this->submodules = submodules;
         this->add_description(description);
      
         for (auto submodule : this->submodules)
            this->add_subparser(*submodule);
      }
      Module(const Module &other) : submodules(other.submodules), argparse::ArgumentParser(other) {}

      Module &operator=(const Module &other) {
         this->submodules = other.submodules;
         argparse::ArgumentParser::operator=(other);

         return *this;
      }

      bool operator()() { return this->execute(); }
      bool operator()(Module &root) { return this->execute(root); }

      bool execute(void) {
         return this->execute(*this);
      }

      virtual bool execute(Module &root) {
         Logger::DebugN("executing {} module", this->m_program_name);
         
         for (auto submodule : this->submodules)
            if (this->is_subcommand_used(submodule->m_program_name))
               return (*submodule)(root);

         Logger::FatalN("error: submodules exhausted, no action selected.");
         std::cerr << this->help().str() << std::endl;

         return false;
      }
   };
}

#endif
