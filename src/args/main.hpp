#ifndef __MALEXANDRIA_ARGS_MAIN_HPP
#define __MALEXANDRIA_ARGS_MAIN_HPP

#include "args/module.hpp"
#include "args/sample.hpp"
#include "exception.hpp"
#include "utility.hpp"

namespace malexandria
{
   struct MainModule : public Module
   {
      static std::shared_ptr<MainModule> Instance;
   
      MainModule()
         : Module("malexandria",
                  "A program for organizing and transporting malware samples and analysis.",
                  {std::make_shared<SampleModule>()},
                  argparse::default_arguments::all)
      {
         this->add_argument("-l", "--log-level")
            .help("Set the logging level of the program. Options are: silent, fatal, notice, info, debug.");
      }

      static std::shared_ptr<MainModule> GetInstance() {
         if (MainModule::Instance == nullptr)
            MainModule::Instance = std::make_shared<MainModule>();

         return MainModule::Instance;
      }
   };
}

#endif
