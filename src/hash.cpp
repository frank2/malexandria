#include "hash.hpp"

using namespace malexandria;

std::vector<std::uint8_t> malexandria::md5(const std::uint8_t *ptr, std::size_t size) {
   CryptoPP::Weak::MD5 hash;
   std::vector<std::uint8_t> digest(hash.DigestSize(), 0);

   hash.Update(ptr, size);
   hash.Final(digest.data());

   return digest;
}

std::vector<std::uint8_t> malexandria::md5(const std::filesystem::path &filename) {
   std::basic_ifstream<std::uint8_t> stream(filename.string(), std::ios::binary);

   if (!stream)
      throw exception::OpenFileFailure(filename.string());

   CryptoPP::Weak::MD5 hash;
   std::vector<std::uint8_t> digest(hash.DigestSize(), 0), buffer(1024 * 1024, 0);

   while (!stream.eof())
   {
      stream.read(buffer.data(), buffer.size());
      hash.Update(buffer.data(), stream.gcount());
   }

   hash.Final(digest.data());

   return digest;
}

std::vector<std::uint8_t> malexandria::sha1(const std::uint8_t *ptr, std::size_t size) {
   CryptoPP::SHA1 hash;
   std::vector<std::uint8_t> digest(hash.DigestSize(), 0);

   hash.Update(ptr, size);
   hash.Final(digest.data());

   return digest;
}

std::vector<std::uint8_t> malexandria::sha1(const std::filesystem::path &filename) {
   std::basic_ifstream<std::uint8_t> stream(filename.string(), std::ios::binary);

   if (!stream)
      throw exception::OpenFileFailure(filename.string());

   CryptoPP::SHA1 hash;
   std::vector<std::uint8_t> digest(hash.DigestSize(), 0), buffer(1024 * 1024, 0);

   while (!stream.eof())
   {
      stream.read(buffer.data(), buffer.size());
      hash.Update(buffer.data(), stream.gcount());
   }

   hash.Final(digest.data());

   return digest;
}

std::vector<std::uint8_t> malexandria::sha256(const std::uint8_t *ptr, std::size_t size) {
   CryptoPP::SHA256 hash;
   std::vector<std::uint8_t> digest(hash.DigestSize(), 0);

   hash.Update(ptr, size);
   hash.Final(digest.data());

   return digest;
}

std::vector<std::uint8_t> malexandria::sha256(const std::filesystem::path &filename) {
   std::basic_ifstream<std::uint8_t> stream(filename.string(), std::ios::binary);

   if (!stream)
      throw exception::OpenFileFailure(filename.string());

   CryptoPP::SHA256 hash;
   std::vector<std::uint8_t> digest(hash.DigestSize(), 0), buffer(1024 * 1024, 0);

   while (!stream.eof())
   {
      stream.read(buffer.data(), buffer.size());
      hash.Update(buffer.data(), stream.gcount());
   }

   hash.Final(digest.data());

   return digest;
}


std::vector<std::uint8_t> malexandria::sha3_384(const std::uint8_t *ptr, std::size_t size) {
   CryptoPP::SHA3_384 hash;
   std::vector<std::uint8_t> digest(hash.DigestSize(), 0);

   hash.Update(ptr, size);
   hash.Final(digest.data());

   return digest;
}

std::vector<std::uint8_t> malexandria::sha3_384(const std::filesystem::path &filename) {
   std::basic_ifstream<std::uint8_t> stream(filename.string(), std::ios::binary);

   if (!stream)
      throw exception::OpenFileFailure(filename.string());

   CryptoPP::SHA3_384 hash;
   std::vector<std::uint8_t> digest(hash.DigestSize(), 0), buffer(1024 * 1024, 0);

   while (!stream.eof())
   {
      stream.read(buffer.data(), buffer.size());
      hash.Update(buffer.data(), stream.gcount());
   }

   hash.Final(digest.data());

   return digest;
}
