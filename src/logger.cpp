#include "logger.hpp"
#include "config.hpp"

using namespace malexandria;

std::unique_ptr<Logger> Logger::Instance = nullptr;

Logger::Logger() : _streams({ &std::cerr })
{
   auto level = MainConfig::GetInstance().log_level();

   if (!level.has_value())
      level = "info";

   this->_level = Logger::LevelFromString(*level);
}
