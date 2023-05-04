#ifndef __MALEXANDRIA_HASH_HPP
#define __MALEXANDRIA_HASH_HPP

#include <cstddef>
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

#include <cryptlib.h>
#include <md5.h>
#include <sha.h>
#include <sha3.h>

#include "exception.hpp"
#include "fs.hpp"

namespace malexandria
{
   std::vector<std::uint8_t> md5(const std::uint8_t *ptr, std::size_t size);
   std::vector<std::uint8_t> md5(const std::filesystem::path &filename);

   std::vector<std::uint8_t> sha1(const std::uint8_t *ptr, std::size_t size);
   std::vector<std::uint8_t> sha1(const std::filesystem::path &filename);

   std::vector<std::uint8_t> sha256(const std::uint8_t *ptr, std::size_t size);
   std::vector<std::uint8_t> sha256(const std::filesystem::path &filename);

   std::vector<std::uint8_t> sha3_384(const std::uint8_t *ptr, std::size_t size);
   std::vector<std::uint8_t> sha3_384(const std::filesystem::path &filename);
}

#endif
