#ifndef __MALEXANDRIA_UTILITY_HPP
#define __MALEXANDRIA_UTILITY_HPP

#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <vector>

#include "exception.hpp"

namespace malexandria
{
   std::string to_hex_string(const std::uint8_t *ptr, std::size_t size, bool in_uppercase=false);
   std::string to_hex_string(const std::vector<std::uint8_t> &vec, bool in_uppercase=false);
   std::vector<std::uint8_t> from_hex_string(const std::string &str);
}

#endif
