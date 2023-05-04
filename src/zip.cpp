#include "zip.hpp"

using namespace malexandria;

std::shared_ptr<zip_file_t> Zip::open(Zip::FileID id) {
   zip_file_t *fp;
   
   if (auto filename = std::get_if<std::string>(&id))
   {
      if (this->_password.has_value())
         fp = zip_fopen_encrypted(**this, filename->c_str(), 0, this->_password->c_str());
      else
         fp = zip_fopen(**this, filename->c_str(), 0);
   }
   else if (auto index = std::get_if<std::uint64_t>(&id))
   {
      if (this->_password.has_value())
         fp = zip_fopen_index_encrypted(**this, *index, 0, this->_password->c_str());
      else
         fp = zip_fopen_index(**this, *index, 0);
   }

   if (fp == nullptr)
      throw exception::ZipException(zip_get_error(**this));

   return std::shared_ptr<zip_file_t>(fp, zip_fclose);
}

zip_stat_t Zip::stat(Zip::FileID id) {
   zip_stat_t result;

   if (auto filename = std::get_if<std::string>(&id))
   {
      if (zip_stat(**this, filename->c_str(), 0, &result) != 0)
         throw exception::ZipException(zip_get_error(**this));
   }
   else if (auto index = std::get_if<std::uint64_t>(&id))
   {
      if (zip_stat_index(**this, *index, 0, &result) != 0)
         throw exception::ZipException(zip_get_error(**this));
   }

   return result;
}

void Zip::set_password(const std::string &password) { this->_password = password; }
void Zip::clear_password(void) { this->_password = std::nullopt; }

void Zip::open(const std::filesystem::path &path, int flags) {
   int zip_error = ZIP_ER_OK;
   auto filename = path.string();
   auto zip_struct = zip_open(filename.c_str(), flags, &zip_error);

   if (zip_error != ZIP_ER_OK)
      throw exception::ZipException(zip_error);

   this->_zip = std::shared_ptr<zip_t>(zip_struct, zip_close);
}

void Zip::close() {
   this->_zip.reset();
}

std::int64_t Zip::locate(const std::string &filename) {
   return zip_name_locate(**this, filename.c_str(), 0);
}

std::string Zip::get_name(std::uint64_t index) {
   auto result = zip_get_name(**this, index, 0);

   if (result == nullptr)
      throw exception::ZipException(zip_get_error(**this));

   return std::string(result);
}

std::int64_t Zip::entries(void) {
   return zip_get_num_entries(**this, 0);
}

std::map<std::int64_t, std::string> Zip::files(void) {
   auto entries = this->entries();

   if (entries < 0)
      throw exception::ZipException(zip_get_error(**this));

   std::map<std::int64_t,std::string> result;
   
   for (std::uint64_t index=0; index<static_cast<std::uint64_t>(entries); ++index)
   {
      auto name = this->get_name(index);

      if (name.back() != '/')
         result[static_cast<std::int64_t>(index)] = name;
   }

   return result;
}

std::vector<std::uint8_t> Zip::extract_to_memory(Zip::FileID file_or_index) {
   auto file_stat = this->stat(file_or_index);
   auto fp = this->open(file_or_index);

   std::vector<std::uint8_t> result(file_stat.size);

   if (zip_fread(fp.get(), result.data(), result.size()) < 0)
      throw exception::ZipException(zip_get_error(**this));

   return result;
}

void Zip::extract_to_disk(Zip::FileID file_or_index, const std::filesystem::path &path) {
   if (!path_exists(path.parent_path()))
      std::filesystem::create_directories(path.parent_path());

   std::ofstream disk_file(path.string(), std::ios::binary);

   if (!disk_file)
      throw exception::OpenFileFailure(path.string());

   auto file_stat = this->stat(file_or_index);
   auto fp = this->open(file_or_index);
   std::int64_t bytes_read;
   std::vector<std::uint8_t> buffer(1024*1024);

   while ((bytes_read = zip_fread(fp.get(), buffer.data(), buffer.size())) != 0)
   {
      if (bytes_read < 0)
         throw exception::ZipException(zip_get_error(**this));

      disk_file.write(reinterpret_cast<const char *>(buffer.data()), bytes_read);
   }

   disk_file.close();
}

std::int64_t Zip::insert_file(const std::filesystem::path &filename, const std::string &zip_filename)
{
   auto file_source = zip_source_file(**this, filename.string().c_str(), 0, 0);

   if (file_source == nullptr)
      throw exception::ZipException(zip_get_error(**this));

   auto file_index = zip_file_add(**this, zip_filename.c_str(), file_source, ZIP_FL_ENC_UTF_8);

   if (file_index == -1)
      throw exception::ZipException(zip_get_error(**this));

   if (this->_password.has_value() && zip_file_set_encryption(**this, file_index, ZIP_EM_AES_256, this->_password->c_str()) != 0)
      throw exception::ZipException(zip_get_error(**this));

   return file_index;
}

std::int64_t Zip::insert_directory(const std::string &dir) {
   auto directory = zip_dir_add(**this, dir.c_str(), ZIP_FL_ENC_UTF_8);

   if (directory == -1)
      throw exception::ZipException(zip_get_error(**this));

   return directory;
}

std::int64_t Zip::insert_buffer(const std::uint8_t *ptr, std::size_t size, const std::string &filename) {
   auto source = zip_source_buffer(**this, ptr, size, 0);

   if (source == nullptr)
      throw exception::ZipException(zip_get_error(**this));

   auto index = zip_file_add(**this, filename.c_str(), source, ZIP_FL_OVERWRITE);

   if (index == -1)
      throw exception::ZipException(zip_get_error(**this));

   if (this->_password.has_value() && zip_file_set_encryption(**this, index, ZIP_EM_AES_256, this->_password->c_str()) == -1)
      throw exception::ZipException(zip_get_error(**this));

   return index;
}

std::int64_t Zip::insert_buffer(const std::vector<std::uint8_t> &vec, const std::string &filename) {
   return this->insert_buffer(vec.data(), vec.size(), filename);
}
