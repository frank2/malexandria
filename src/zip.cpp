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

void Zip::open(std::optional<std::vector<std::uint8_t>> init_data, int flags) {
   zip_error_t zip_error = {0,0,0};
   auto zip_memory = zip_source_buffer_create(0, 0, 0, &zip_error);

   if (zip_memory == nullptr)
      throw exception::ZipException(&zip_error);

   this->_source = std::shared_ptr<zip_source_t>(zip_memory, zip_source_free);

   if (init_data.has_value())
   {
      if (zip_source_begin_write(this->_source.get()) != 0)
         throw exception::ZipException(zip_source_error(this->_source.get()));
      
      if (zip_source_write(this->_source.get(), init_data->data(), init_data->size()) < 0)
         throw exception::ZipException(zip_source_error(this->_source.get()));
      
      if (zip_source_commit_write(this->_source.get()) != 0)
         throw exception::ZipException(zip_source_error(this->_source.get()));
   }
   else
      flags |= ZIP_TRUNCATE;

   zip_source_keep(this->_source.get());
   auto zip_struct = zip_open_from_source(zip_memory, flags, &zip_error);

   if (zip_error.zip_err != ZIP_ER_OK)
      throw exception::ZipException(&zip_error);

   this->_zip = std::make_shared<zip_t *>(zip_struct);
}

void Zip::open(const std::filesystem::path &path, int flags) {
   int zip_error = ZIP_ER_OK;
   auto filename = path;
   filename = filename.make_preferred();
   auto zip_struct = zip_open(filename.string().c_str(), flags, &zip_error);

   if (zip_error != ZIP_ER_OK)
      throw exception::ZipException(zip_error);

   this->_zip = std::make_shared<zip_t *>(zip_struct);
}

void Zip::close() {
   if (zip_close(**this) == -1)
      throw exception::ZipException(zip_get_error(**this));
   
   *this->_zip = nullptr;
}

void Zip::discard(void) {
   zip_discard(**this);
   
   *this->_zip = nullptr;
   this->_source.reset();
}

std::vector<std::uint8_t> Zip::read(void) {
   if (this->_source == nullptr)
      throw exception::NoSourceBuffer();

   if (zip_source_open(this->_source.get()) != 0)
      throw exception::ZipException(zip_source_error(this->_source.get()));

   if (zip_source_seek(this->_source.get(), 0, SEEK_END) != 0)
      throw exception::ZipException(zip_source_error(this->_source.get()));

   auto memory_size = zip_source_tell(this->_source.get());

   if (memory_size == -1)
      throw exception::ZipException(zip_source_error(this->_source.get()));

   if (zip_source_seek(this->_source.get(), 0, SEEK_SET) != 0)
      throw exception::ZipException(zip_source_error(this->_source.get()));

   std::vector<std::uint8_t> zip_data(memory_size);

   if (zip_source_read(this->_source.get(), zip_data.data(), zip_data.size()) == -1)
      throw exception::ZipException(zip_source_error(this->_source.get()));

   if (zip_source_close(this->_source.get()) != 0)
      throw exception::ZipException(zip_source_error(this->_source.get()));

   return zip_data;
}

void Zip::save(const std::filesystem::path &filename) {
   auto data = this->read();

   std::ofstream outfile(filename.string(), std::ios::binary);

   if (!outfile)
      throw exception::OpenFileFailure(filename.string());

   outfile.write(reinterpret_cast<char *>(data.data()), data.size());
   outfile.close();
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

std::vector<Zip::FileEntry> Zip::files(void) {
   auto entries = this->entries();

   if (entries < 0)
      throw exception::ZipException(zip_get_error(**this));

   std::vector<Zip::FileEntry> result;
   
   for (std::uint64_t index=0; index<static_cast<std::uint64_t>(entries); ++index)
      result.push_back(std::make_pair(static_cast<std::int64_t>(index), this->get_name(index)));

   return result;
}

std::map<std::string,std::vector<Zip::FileEntry>> Zip::file_tree(void) {
   auto files = this->files();
   std::map<std::string,std::vector<Zip::FileEntry>> result;

   for (auto &entry : files)
   {
      auto name = entry.second;
      auto path = std::filesystem::path("/");
      path += std::filesystem::path(name);
      auto parent_path = path.parent_path();

      if (name.back() != '/')
         result[parent_path.string()].push_back(entry);
      else if (result.find(parent_path.string()) == result.end())
         result[parent_path.string()] = std::vector<Zip::FileEntry>();
   }

   return result;
}

std::vector<std::uint8_t> Zip::extract_to_memory(Zip::FileID file_or_index) {
   auto file_stat = this->stat(file_or_index);
   auto fp = this->open(file_or_index);

   std::vector<std::uint8_t> result(file_stat.size);

   if (file_stat.size == 0)
      return result;

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

   if (file_stat.size > 0)
   {
      auto fp = this->open(file_or_index);
      std::int64_t bytes_read;
      std::vector<std::uint8_t> buffer(1024*1024);

      while ((bytes_read = zip_fread(fp.get(), buffer.data(), buffer.size())) != 0)
      {
         if (bytes_read < 0)
            throw exception::ZipException(zip_get_error(**this));

         disk_file.write(reinterpret_cast<const char *>(buffer.data()), bytes_read);
      }
   }

   disk_file.close();
}

std::vector<std::pair<std::filesystem::path,std::int64_t>> Zip::create_directories(const std::filesystem::path &with_dirs) {
   std::vector<std::pair<std::filesystem::path,std::int64_t>> result;
   auto all_dirs = with_dirs;
   std::vector<std::string> create_stack;

   while (all_dirs.has_parent_path() && all_dirs.parent_path() != std::filesystem::path("/"))
   {
      auto parent = all_dirs.parent_path();
      all_dirs = parent;

      auto zip_dir = dos_to_unix_path(fmt::format("{}/", parent.string()));

      if (this->locate(zip_dir) == -1)
         create_stack.push_back(zip_dir);
      else
         break;
   }

   for (auto &path : create_stack)
      result.push_back(std::make_pair(std::filesystem::path(path), this->insert_directory(path)));

   return result;
}

void Zip::clear_directories(const std::filesystem::path &filename) {
   auto tree = this->file_tree();
   auto path = std::filesystem::path("/");
   path += std::filesystem::path(dos_to_unix_path(filename.string()));

   do {
      if (this->locate(skip_root("/", path).string()) < 0)
         break;
      
      if (tree.find(path.string()) != tree.end() && tree[path.string()].size() > 1)
         break;

      this->delete_file(dos_to_unix_path(skip_root("/", path).string()));
      path = path.parent_path();
   } while (path != std::filesystem::path("/"));
}

std::int64_t Zip::insert_file(const std::filesystem::path &filename, const std::string &zip_filename)
{
   if (!path_exists(filename))
      throw exception::FileNotFound(filename.string());

   this->_inserted_strings.push_back(std::make_shared<std::string>(filename.string()));
   auto disk_filename = this->_inserted_strings.back();
   
   auto file_source = zip_source_file(**this, disk_filename->c_str(), 0, 0);

   if (file_source == nullptr)
   {
      this->_inserted_strings.pop_back();
      throw exception::ZipException(zip_get_error(**this));
   }

   this->_inserted_strings.push_back(std::make_shared<std::string>(zip_filename));
   auto zip_filename_ptr = this->_inserted_strings.back();
   
   std::int64_t file_index = this->locate(zip_filename);
   std::int64_t add_status;

   if (file_index >= 0)
   {
      auto stat = this->stat(static_cast<std::uint64_t>(file_index));
      auto crc = crc32(filename);

      if (crc == stat.crc)
      {
         MLX_DEBUGN("file crc unchanged, skipping replacement.");
         zip_source_free(file_source);
         this->_inserted_strings.pop_back();
         this->_inserted_strings.pop_back();
         
         return file_index;
      }
      
      add_status = zip_file_replace(**this, file_index, file_source, ZIP_FL_ENC_UTF_8);
   }
   else
   {
      add_status = zip_file_add(**this, zip_filename_ptr->c_str(), file_source, ZIP_FL_ENC_UTF_8);
      file_index = add_status;
   }

   if (add_status == -1)
   {
      this->_inserted_strings.pop_back();
      this->_inserted_strings.pop_back();
      throw exception::ZipException(zip_get_error(**this));
   }

   if (this->_password.has_value() && zip_file_set_encryption(**this, file_index, ZIP_EM_AES_256, this->_password->c_str()) != 0)
   {
      this->_inserted_strings.pop_back();
      this->_inserted_strings.pop_back();
      throw exception::ZipException(zip_get_error(**this));
   }

   return file_index;
}

std::int64_t Zip::insert_directory(const std::string &dir) {
   auto directory = zip_dir_add(**this, dir.c_str(), ZIP_FL_ENC_UTF_8);

   if (directory == -1)
      throw exception::ZipException(zip_get_error(**this));

   return directory;
}

void Zip::delete_file(Zip::FileID file_or_index) {
   std::uint64_t entry_id;

   if (auto filename = std::get_if<std::string>(&file_or_index))
      entry_id = this->locate(*filename);
   else
      entry_id = std::get<std::uint64_t>(file_or_index);

   if (zip_delete(**this, entry_id) == -1)
      throw exception::ZipException(zip_get_error(**this));
}

void Zip::rename_file(Zip::FileID file_or_index, std::string filename) {
   std::uint64_t index;
   std::string old_filename;

   if (auto v_filename = std::get_if<std::string>(&file_or_index))
   {
      index = this->locate(*v_filename);
      old_filename = *v_filename;

      if (index < 0)
         throw exception::ZipException(zip_get_error(**this));
   }
   else
   {
      index = std::get<std::uint64_t>(file_or_index);
      old_filename = this->get_name(index);
   }

   auto old_file_path = std::filesystem::path(dos_to_unix_path(old_filename));
   auto new_file_path = std::filesystem::path(dos_to_unix_path(filename));

   this->create_directories(new_file_path);

   if (zip_file_rename(**this, index, filename.c_str(), 0) != 0)
      throw exception::ZipException(zip_get_error(**this));

   this->clear_directories(old_file_path);
}

std::int64_t Zip::insert_buffer(const std::uint8_t *ptr, std::size_t size, const std::string &filename) {
   zip_error_t error = {0,0,0};
   
   auto source = zip_source_buffer_create(0, 0, 0, &error);

   if (source == nullptr)
      throw exception::ZipException(&error);

   if (zip_source_begin_write(source) != 0)
      throw exception::ZipException(zip_source_error(source));
      
   if (zip_source_write(source, ptr, size) < 0)
      throw exception::ZipException(zip_source_error(source));
      
   if (zip_source_commit_write(source) != 0)
      throw exception::ZipException(zip_source_error(source));

   this->_inserted_strings.push_back(std::make_shared<std::string>(filename));
   auto filename_ptr = this->_inserted_strings.back();
   auto index = zip_file_add(**this, filename_ptr->c_str(), source, ZIP_FL_OVERWRITE);
   
   if (index == -1)
   {
      this->_inserted_strings.pop_back();
      throw exception::ZipException(zip_get_error(**this));
   }

   if (this->_password.has_value() && zip_file_set_encryption(**this, index, ZIP_EM_AES_256, this->_password->c_str()) == -1)
   {
      this->_inserted_strings.pop_back();
      throw exception::ZipException(zip_get_error(**this));
   }

   return index;
}

std::int64_t Zip::insert_buffer(const std::vector<std::uint8_t> &vec, const std::string &filename) {
   return this->insert_buffer(vec.data(), vec.size(), filename);
}
