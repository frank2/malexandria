#ifndef __MALEXANDRIA_DB_HPP
#define __MALEXANDRIA_DB_HPP

#include <cstring>
#include <memory>
#include <string>
#include <variant>

#include <sqlite3.h>

#include "config.hpp"
#include "exception.hpp"
#include "fs.hpp"

namespace malexandria
{
   using SQLiteType = std::variant<std::vector<std::uint8_t>,
                                   double,
                                   std::int64_t,
                                   std::string>;
   
      
   class Database
   {
      static std::unique_ptr<Database> Instance;

      sqlite3 *_handle;

      Database(): _handle(nullptr) { this->open(); }

      void create(void);
      
   public:
      static Database &GetInstance(void);
      ~Database(void) { this->close(); }

      sqlite3 *operator*(void) { return this->handle(); }
      const sqlite3 *operator*(void) const { return this->handle(); }
      
      void open(void);
      void close(void);
      template <typename... Args>
      std::vector<std::vector<SQLiteType>> query(const std::string &stmt, Args&&... bindings);
      std::int64_t last_insert_rowid();

      sqlite3 *handle(void) { return this->_handle; }
      const sqlite3 *handle(void) const { return this->_handle; }
   };

   class Statement
   {
      std::shared_ptr<sqlite3_stmt> _statement;
      std::size_t _bind_index;

   public:
      Statement() : _statement(nullptr), _bind_index(0) {}
      template <typename... Args>
      Statement(const std::string &stmt, const char **tail, Args&&... args)
         : Statement(stmt.c_str(), tail, args...)
      {}
      template <typename... Args>
      Statement(const char *stmt, const char **tail, Args&&... args) : _bind_index(0) {
         this->create_statement(stmt, tail);
         this->bind(args...);
      }
      Statement(const Statement &other) : _statement(other._statement), _bind_index(other._bind_index) {}

      Statement &operator=(const Statement &other) {
         this->_statement = other._statement;
         this->_bind_index = other._bind_index;

         return *this;
      }
      operator sqlite3_stmt *() { return this->statement_ptr(); }
      operator const sqlite3_stmt *() const { return this->statement_ptr(); }

      sqlite3_stmt *statement_ptr() { return this->_statement.get(); }
      const sqlite3_stmt *statement_ptr() const { return this->_statement.get(); }

      void create_statement(const std::string &stmt, const char **tail)
      {
         this->create_statement(stmt.c_str(), tail);
      }

      void create_statement(const char *stmt, const char **tail)
      {
         sqlite3_stmt *base_statement;
         auto result = sqlite3_prepare_v2(*Database::GetInstance(),
                                          stmt,
                                          std::strlen(stmt),
                                          &base_statement,
                                          tail);

         if (result != SQLITE_OK)
            throw exception::SQLiteException(result, sqlite3_errmsg(*Database::GetInstance()));

         this->_statement = std::shared_ptr<sqlite3_stmt>(base_statement, sqlite3_finalize);
      }

      void reset() {
         this->_bind_index = 0;
      }
      
      void bind() {
         return;
      }
      
      template <typename... Args>
      void bind(const std::vector<std::uint8_t> &blob, Args&&... args)
      {
         if (this->_statement == nullptr)
            throw exception::NullPointer();

         auto result = sqlite3_bind_blob64(*this, ++this->_bind_index, blob.data(), blob.size(), nullptr);

         if (result != SQLITE_OK)
            throw exception::SQLiteException(result, sqlite3_errmsg(*Database::GetInstance()));

         this->bind(args...);
      }
      
      template <typename... Args>
      void bind(double &d, Args&&... args)
      {
         if (this->_statement == nullptr)
            throw exception::NullPointer();

         auto result = sqlite3_bind_double(*this, ++this->_bind_index, d);

         if (result != SQLITE_OK)
            throw exception::SQLiteException(result, sqlite3_errmsg(*Database::GetInstance()));

         this->bind(args...);
      }
      
      template <typename... Args>
      void bind(std::int32_t i, Args&&... args)
      {
         if (this->_statement == nullptr)
            throw exception::NullPointer();

         auto result = sqlite3_bind_int(*this, ++this->_bind_index, i);

         if (result != SQLITE_OK)
            throw exception::SQLiteException(result, sqlite3_errmsg(*Database::GetInstance()));

         this->bind(args...);
      }
      
      template <typename... Args>
      void bind(std::int64_t i, Args&&... args)
      {
         if (this->_statement == nullptr)
            throw exception::NullPointer();

         auto result = sqlite3_bind_int64(*this, ++this->_bind_index, i);

         if (result != SQLITE_OK)
            throw exception::SQLiteException(result, sqlite3_errmsg(*Database::GetInstance()));

         this->bind(args...);
      }
      
      template <typename... Args>
      void bind(const std::string &str, Args&&... args)
      {
         if (this->_statement == nullptr)
            throw exception::NullPointer();

         auto result = sqlite3_bind_text(*this, ++this->_bind_index, str.c_str(), str.size(), nullptr);

         if (result != SQLITE_OK)
            throw exception::SQLiteException(result, sqlite3_errmsg(*Database::GetInstance()));

         this->bind(args...);
      }

      std::optional<std::vector<SQLiteType>> step() {
         auto result = sqlite3_step(*this);

         switch (result)
         {
         case SQLITE_DONE:
            return std::nullopt;

         case SQLITE_ROW:
         {
            std::vector<SQLiteType> return_val;

            for (int i=0; i<sqlite3_column_count(*this); ++i)
            {
               switch (sqlite3_column_type(*this, i))
               {
               case SQLITE_INTEGER:
                  return_val.push_back(sqlite3_column_int64(*this, i));
                  break;

               case SQLITE_FLOAT:
                  return_val.push_back(sqlite3_column_double(*this, i));
                  break;

               case SQLITE_TEXT:
                  return_val.push_back(std::string(reinterpret_cast<const char *>(sqlite3_column_text(*this, i)), sqlite3_column_bytes(*this, i)));
                  break;

               case SQLITE_BLOB:
                  auto blob_ptr = reinterpret_cast<const std::uint8_t *>(sqlite3_column_blob(*this, i));
                  return_val.push_back(std::vector<std::uint8_t>(blob_ptr, blob_ptr+sqlite3_column_bytes(*this, i)));
                  break;
               }
            }

            return return_val;
         }

         default:
            throw exception::SQLiteException(result, sqlite3_errmsg(*Database::GetInstance()));
         }
      }

      std::vector<std::vector<SQLiteType>> execute() {
         std::vector<std::vector<SQLiteType>> result;
         std::optional<std::vector<SQLiteType>> step_result;

         do
         {
            step_result = this->step();

            if (step_result.has_value())
               result.push_back(*step_result);
         } while (step_result.has_value());

         return result;
      }
   };

   template <typename... Args>
   std::vector<std::vector<SQLiteType>> Database::query(const std::string &stmt, Args&&... bindings)
   {
      std::vector<Statement> statements;
      const char *head = stmt.c_str();
      const char *tail = nullptr;

      do
      {
         statements.push_back(Statement(head, &tail));
         head = tail;
      } while (tail != nullptr && *tail != 0);

      statements.back().bind(bindings...);

      std::vector<std::vector<SQLiteType>> result;

      for (auto &statement : statements)
      {
         auto statement_result = statement.execute();
         result.insert(result.end(), statement_result.begin(), statement_result.end());
      }

      return result;
   }
}

#endif
