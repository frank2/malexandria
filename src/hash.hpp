#ifndef __MALEXANDRIA_HASH_HPP
#define __MALEXANDRIA_HASH_HPP

#include <cerrno>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include <cryptlib.h>
#include <crc.h>
#include <md5.h>
#include <sha.h>
#include <sha3.h>

#include "exception.hpp"
#include "fs.hpp"
#include "logger.hpp"

namespace malexandria
{
   std::uint32_t crc32(const std::uint8_t *ptr, std::size_t size);
   std::uint32_t crc32(const std::vector<std::uint8_t> &vec);
   std::uint32_t crc32(const std::filesystem::path &filename);

   std::vector<std::uint8_t> md5(const std::uint8_t *ptr, std::size_t size);
   std::vector<std::uint8_t> md5(const std::vector<std::uint8_t> &vec);
   std::vector<std::uint8_t> md5(const std::filesystem::path &filename);

   std::vector<std::uint8_t> sha1(const std::uint8_t *ptr, std::size_t size);
   std::vector<std::uint8_t> sha1(const std::vector<std::uint8_t> &vec);
   std::vector<std::uint8_t> sha1(const std::filesystem::path &filename);

   std::vector<std::uint8_t> sha256(const std::uint8_t *ptr, std::size_t size);
   std::vector<std::uint8_t> sha256(const std::vector<std::uint8_t> &vec);
   std::vector<std::uint8_t> sha256(const std::filesystem::path &filename);

   std::vector<std::uint8_t> sha3_384(const std::uint8_t *ptr, std::size_t size);
   std::vector<std::uint8_t> sha3_384(const std::vector<std::uint8_t> &vec);
   std::vector<std::uint8_t> sha3_384(const std::filesystem::path &filename);
}

#endif
