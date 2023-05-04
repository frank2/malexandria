#include "utility.hpp"

using namespace malexandria;

std::string malexandria::to_hex_string(const std::uint8_t *ptr, std::size_t size, bool in_uppercase) {
   const static auto uppercase = "0123456789ABCDEF";
   const static auto lowercase = "0123456789abcdef";
   const auto charset = (in_uppercase) ? uppercase : lowercase;

   std::string result;

   for (std::size_t i=0; i<size; ++i)
   {
      result.push_back(charset[ptr[i] >> 4]);
      result.push_back(charset[ptr[i] & 0xF]);
   }

   return result;
}

std::string malexandria::to_hex_string(const std::vector<std::uint8_t> &vec, bool in_uppercase) {
   return malexandria::to_hex_string(vec.data(), vec.size(), in_uppercase);
}

std::vector<std::uint8_t> malexandria::from_hex_string(const std::string &str) {
   std::vector<std::uint8_t> result;

   if (str.size() % 2 != 0)
      throw exception::InvalidHexString(str);

   for (std::size_t i=0; i<str.size(); i+=2)
   {
      std::uint8_t value = 0;
      
      for (std::size_t j=0; j<2; ++j)
      {
         char c = str[i+j];
         std::uint8_t base_value = 0;

         if (c >= '0' && c <= '9')
            base_value = c - '0';
         else if (c >= 'a' && c <= 'f')
            base_value = c - 'a' + 10;
         else if (c >= 'A' && c <= 'F')
            base_value = c - 'A' + 10;
         else
            throw exception::InvalidHexCharacter(c);

         value |= base_value << (4 * (1 - j));
      }

      result.push_back(value);
   }

   return result;
}
