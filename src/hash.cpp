#include "hash.hpp"

using namespace malexandria;

std::uint32_t malexandria::crc32(const std::uint8_t *ptr, std::size_t size) {
   CryptoPP::CRC32 hash;
   std::vector<std::uint8_t> digest(hash.DigestSize(), 0);

   hash.Update(ptr, size);
   hash.Final(digest.data());

   std::uint32_t result = 0;

   for (std::size_t i=0; i<digest.size(); ++i)
      result |= digest[i] << (i * 8);

   return result;
}

std::uint32_t malexandria::crc32(const std::vector<std::uint8_t> &vec) {
   return crc32(vec.data(), vec.size());
}

std::uint32_t malexandria::crc32(const std::filesystem::path &filename) {
   std::basic_ifstream<std::uint8_t> stream(filename.string(), std::ios::binary);

   if (!stream)
      throw exception::OpenFileFailure(filename.string());

   CryptoPP::CRC32 hash;
   std::vector<std::uint8_t> digest(hash.DigestSize(), 0), buffer(1024 * 1024, 0);

   while (!stream.eof() && stream.good())
   {
      stream.read(buffer.data(), buffer.size());
      hash.Update(buffer.data(), stream.gcount());
   }

   hash.Final(digest.data());

   std::uint32_t result = 0;

   for (std::size_t i=0; i<digest.size(); ++i)
      result |= digest[i] << (i * 8);

   return result;
}

std::vector<std::uint8_t> malexandria::md5(const std::uint8_t *ptr, std::size_t size) {
   CryptoPP::Weak::MD5 hash;
   std::vector<std::uint8_t> digest(hash.DigestSize(), 0);

   hash.Update(ptr, size);
   hash.Final(digest.data());

   return digest;
}

std::vector<std::uint8_t> malexandria::md5(const std::vector<std::uint8_t> &vec) {
   return md5(vec.data(), vec.size());
}

std::vector<std::uint8_t> malexandria::md5(const std::filesystem::path &filename) {
   std::basic_ifstream<std::uint8_t> stream(filename.string(), std::ios::binary);

   if (!stream)
      throw exception::OpenFileFailure(filename.string());

   CryptoPP::Weak::MD5 hash;
   std::vector<std::uint8_t> digest(hash.DigestSize(), 0), buffer(1024 * 1024, 0);

   while (!stream.eof() && stream.good())
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

std::vector<std::uint8_t> malexandria::sha1(const std::vector<std::uint8_t> &vec) {
   return sha1(vec.data(), vec.size());
}

std::vector<std::uint8_t> malexandria::sha1(const std::filesystem::path &filename) {
   std::basic_ifstream<std::uint8_t> stream(filename.string(), std::ios::binary);

   if (!stream)
      throw exception::OpenFileFailure(filename.string());

   CryptoPP::SHA1 hash;
   std::vector<std::uint8_t> digest(hash.DigestSize(), 0), buffer(1024 * 1024, 0);

   while (!stream.eof() && stream.good())
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

std::vector<std::uint8_t> malexandria::sha256(const std::vector<std::uint8_t> &vec) {
   return sha256(vec.data(), vec.size());
}

std::vector<std::uint8_t> malexandria::sha256(const std::filesystem::path &filename) {
   std::basic_ifstream<std::uint8_t> stream(filename.string(), std::ios::binary);

   if (!stream)
      throw exception::OpenFileFailure(filename.string());

   CryptoPP::SHA256 hash;
   std::vector<std::uint8_t> digest(hash.DigestSize(), 0), buffer(1024 * 1024, 0);

   while (!stream.eof() && stream.good())
   {
      stream.read(buffer.data(), buffer.size());
      hash.Update(buffer.data(), stream.gcount());
   }

   hash.Final(digest.data());

   MLX_DEBUGN("got sha256 hash {}", to_hex_string(digest));

   return digest;
}


std::vector<std::uint8_t> malexandria::sha3_384(const std::uint8_t *ptr, std::size_t size) {
   CryptoPP::SHA3_384 hash;
   std::vector<std::uint8_t> digest(hash.DigestSize(), 0);

   hash.Update(ptr, size);
   hash.Final(digest.data());

   return digest;
}

std::vector<std::uint8_t> malexandria::sha3_384(const std::vector<std::uint8_t> &vec) {
   return sha3_384(vec.data(), vec.size());
}

std::vector<std::uint8_t> malexandria::sha3_384(const std::filesystem::path &filename) {
   std::basic_ifstream<std::uint8_t> stream(filename.string(), std::ios::binary);

   if (!stream)
      throw exception::OpenFileFailure(filename.string());

   CryptoPP::SHA3_384 hash;
   std::vector<std::uint8_t> digest(hash.DigestSize(), 0), buffer(1024 * 1024, 0);

   while (!stream.eof() && stream.good())
   {
      stream.read(buffer.data(), buffer.size());
      hash.Update(buffer.data(), stream.gcount());
   }

   hash.Final(digest.data());

   return digest;
}
