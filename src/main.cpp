#include <stdlib.h> // for setenv
#include <iostream>

#include <argparse/argparse.hpp>

#include "args.hpp"
#include "config.hpp"
#include "db.hpp"
#include "hash.hpp"
#include "logger.hpp"
#include "platform.hpp"
#include "sample.hpp"
#include "utility.hpp"

using namespace malexandria;

int main(int argc, char *argv[])
{
   auto main_module = MainModule::GetInstance();

   try {
      main_module->parse_args(argc, argv);
   }
   catch (std::exception &exc)
   {
      // use std::cerr here because logging might be wrapped up behind a config that was never initialized
      std::cerr << "error: argparse error: " << exc.what() << std::endl;
      return 1;
   }

   try {
      auto log_level = main_module->present("--log-level");

      if (log_level.has_value())
         Logger::SetLevel(Logger::LevelFromString(*log_level));
   }
   catch (exception::Exception &exc)
   {
      std::cerr << "error: " << exc.what() << std::endl;
      return 1;
   }

   MLX_DEBUGN("arguments parsed.");

   //#if defined(MALEXANDRIA_STATIC)
   if (ssh_init() == SSH_ERROR)
   {
      std::cerr << "error: ssh_init failed" << std::endl;
      return 1;
   }
   //#endif

   int exit_code;

   try
   {
      exit_code = static_cast<int>(!(*main_module)());
   }
   catch (exception::Exception &exc)
   {
      Logger::FatalN("error: unhandled exception: {}", exc.what());
      exit_code = 1;
   }
   catch (std::exception &exc)
   {
      Logger::FatalN("error: unknown exception: {}", exc.what());
      exit_code = 1;
   }

   //#if defined(MALEXANDRIA_STATIC)
   ssh_finalize();
   //#endif
   
   return exit_code;
}
