#include "db.hpp"
#include <iostream>
using namespace malexandria;

std::unique_ptr<Database> Database::Instance = nullptr;

void Database::create(void) {
   const static std::string tables = "\n\
CREATE TABLE mlx_samples (\n\
   id INTEGER PRIMARY KEY ASC,\n\
   filename TEXT NOT NULL,\n\
   md5 CHAR(32) NOT NULL,\n\
   sha1 CHAR(40) NOT NULL,\n\
   sha256 CHAR(64) NOT NULL,\n\
   sha3_384 CHAR(96) NOT NULL,\n\
   created INTEGER,\n\
   modified INTEGER\n\
);\n\
\n\
CREATE TABLE mlx_tags (\n\
   id INTEGER PRIMARY KEY ASC,\n\
   tag TEXT UNIQUE NOT NULL\n\
);\n\
\n\
CREATE TABLE mlx_families (\n\
   id INTEGER PRIMARY KEY ASC,\n\
   family TEXT UNIQUE NOT NULL\n\
);\n\
\n\
CREATE TABLE mlx_sample_tags (\n\
   sample_id INTEGER NOT NULL,\n\
   tag_id INTEGER NOT NULL,\n\
   FOREIGN KEY(sample_id) REFERENCES mlx_samples(id),\n\
   FOREIGN KEY(tag_id) REFERENCES mlx_tags(id)\n\
);\n\
\n\
CREATE TABLE mlx_sample_families (\n\
   sample_id INTEGER NOT NULL,\n\
   family_id INTEGER NOT NULL,\n\
   FOREIGN KEY(sample_id) REFERENCES mlx_samples(id),\n\
   FOREIGN KEY(family_id) REFERENCES mlx_families(id)\n\
);\n\
\n\
CREATE TABLE mlx_sample_relationships (\n\
   parent_id INTEGER NOT NULL,\n\
   child_id INTEGER NOT NULL,\n\
   FOREIGN KEY(parent_id) REFERENCES mlx_samples(id),\n\
   FOREIGN KEY(child_id) REFERENCES mlx_samples(id)\n\
);\n\
\n\
CREATE TABLE mlx_aliases (\n\
   sample_id INTEGER UNIQUE NOT NULL,\n\
   alias TEXT UNIQUE NOT NULL,\n\
   FOREIGN KEY(sample_id) REFERENCES mlx_samples(id)\n\
);";

   Database::GetInstance().query(tables);
}

Database &Database::GetInstance(void) {
   if (Database::Instance == nullptr)
      Database::Instance = std::unique_ptr<Database>(new Database());

   return *Database::Instance;
}

void Database::open(void) {
   auto &config = MainConfig::GetInstance();
   auto db_file = config.database();
   bool is_new = !path_exists(db_file);
   auto result = sqlite3_open(db_file.c_str(), &this->_handle);

   if (result != SQLITE_OK)
      throw exception::SQLiteException(result, sqlite3_errmsg(this->_handle));

   if (is_new)
      this->create();
}
    
void Database::close(void) {
   if (this->_handle == nullptr)
      return;
   
   auto result = sqlite3_close(this->_handle);

   if (result != SQLITE_OK)
      throw exception::SQLiteException(result, sqlite3_errmsg(this->_handle));

   this->_handle = nullptr;
}

std::int64_t Database::last_insert_rowid() {
   return sqlite3_last_insert_rowid(this->_handle);
}
