#ifndef __MALEXANDRIA_LOGGER_HPP
#define __MALEXANDRIA_LOGGER_HPP

#include <cstddef>
#include <cstdint>
#include <format>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "config.hpp"

namespace malexandria
{
   class Logger
   {
   public:
      enum class Level : std::uint8_t
      {
         Silent = 0,
         Fatal,
         Notice,
         Info,
         Debug
      };

   private:
      static std::unique_ptr<Logger> Instance;
      
      Level _level;
      std::vector<std::ostream *> _streams;

      Logger();
      Logger(const Logger &other) : _level(other._level), _streams(other._streams) {}

      Logger &operator=(const Logger &other)
      {
         this->_level = other._level;
         this->_streams = other._streams;

         return *this;
      }

      static Logger &GetInstance(void) {
         if (Logger::Instance == nullptr)
            Logger::Instance = std::unique_ptr<Logger>(new Logger());

         return *Logger::Instance;
      }

      void write(Level level, const std::string &data) {
         if (this->_level < level)
            return;
         
         for (auto s : this->_streams)
            *s << data << std::flush;
      }

   public:
      static Level LevelFromString(const std::string &level) {
         auto level_copy = level;
         std::transform(level_copy.begin(), level_copy.end(), level_copy.begin(),
                        [](unsigned char c) { return std::tolower(c); });

         if (level_copy == "silent")
            return Level::Silent;
         else if (level_copy == "fatal")
            return Level::Fatal;
         else if (level_copy == "notice")
            return Level::Notice;
         else if (level_copy == "info")
            return Level::Info;
         else if (level_copy == "debug")
            return Level::Debug;
         else
            throw exception::UnknownLogLevel(level);
      }
      static void SetLevel(Logger::Level level) { Logger::GetInstance()._level = level; }
      static Logger::Level GetLevel() { return Logger::GetInstance()._level; }
      static void AddStream(std::ostream &stream) { Logger::GetInstance()._streams.push_back(&stream); }
      
      template <typename... Args>
      static void Raw(Level level, const std::string &message, Args&&... args)
      {
         Logger::GetInstance().write(level, std::vformat(message, std::make_format_args(args...)));
      }

      template <typename... Args>
      static void Prefixed(Level level, const std::string &prefix, const std::string &message, Args&&... args)
      {
         Logger::Raw(level, std::format("{} {}", prefix, message), args...);
      }

      template <typename... Args>
      static void DebugN(const std::string &message, Args&&... args) {
         Logger::Prefixed(Level::Debug, "$$", std::format("{}\n", message), args...);
      }

      template <typename... Args>
      static void Debug(const std::string &message, Args&&... args) {
         Logger::Prefixed(Level::Debug, "$$", message, args...);
      }

      template <typename... Args>
      static void InfoN(const std::string &message, Args&&... args) {
         Logger::Prefixed(Level::Info, ">>", std::format("{}\n", message), args...);
      }

      template <typename... Args>
      static void Info(const std::string &message, Args&&... args) {
         Logger::Prefixed(Level::Info, ">>", message, args...);
      }

      template <typename... Args>
      static void NoticeN(const std::string &message, Args&&... args) {
         Logger::Prefixed(Level::Notice, "!!", std::format("{}\n", message), args...);
      }

      template <typename... Args>
      static void Notice(const std::string &message, Args&&... args) {
         Logger::Prefixed(Level::Notice, "!!", message, args...);
      }

      template <typename... Args>
      static void FatalN(const std::string &message, Args&&... args) {
         Logger::Prefixed(Level::Fatal, "##", std::format("{}\n", message), args...);
      }

      template <typename... Args>
      static void Fatal(const std::string &message, Args&&... args) {
         Logger::Prefixed(Level::Fatal, "##", message, args...);
      }
   };
}
#endif
