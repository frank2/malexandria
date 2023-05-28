#ifndef __MALEXANDRIA_EXCEPTION_HPP
#define __MALEXANDRIA_EXCEPTION_HPP

#include <exception>
#include <sstream>
#include <string>

#include <sqlite3.h>
#include <zip.h>

namespace malexandria
{
namespace exception
{
   class Exception : public std::exception
   {
   public:
      std::string error;
      Exception() : std::exception() {}
      Exception(std::string error) : error(error), std::exception() {}

      virtual const char *what() noexcept {
         return this->error.c_str();
      }
   };

   class OpenFileFailure : public Exception
   {
   public:
      std::string filename;

      OpenFileFailure(const std::string &filename) : filename(filename), Exception() {
         std::stringstream stream;

         stream << "Open file failure: failed to open file \"" << filename << "\"";

         this->error = stream.str();
      }
   };

   class FileNotFound : public Exception
   {
   public:
      std::string filename;

      FileNotFound(const std::string &filename) : filename(filename), Exception() {
         std::stringstream stream;

         stream << "File not found: the file \"" << filename << "\" could not be found.";

         this->error = stream.str();
      }
   };

   class JSONException : public Exception
   {
   public:
      JSONException(const std::exception &exc) : Exception() {
         std::stringstream stream;

         stream << "JSON Exception: " << exc.what();

         this->error = stream.str();
      }
   };

   class NoHomeDirectory : public Exception
   {
   public:
      NoHomeDirectory() : Exception("No home directory: couldn't resolve home path environment variable.") {}
   };

   class CreateDirectoryFailure : public Exception
   {
   public:
      std::string directory;

      CreateDirectoryFailure(const std::string &directory) : directory(directory), Exception() {
         std::stringstream stream;

         stream << "Create directory failure: failed to create one or more directories in \"" << directory << "\"";

         this->error = stream.str();
      }
   };

   class SQLiteException : public Exception
   {
   public:
      int code;

      SQLiteException(int code, const char *msg) : code(code), Exception() {
         std::stringstream stream;

         stream << "SQLite Exception: code " << std::hex << std::showbase << code
                << ": " << msg;

         this->error = stream.str();
      }
   };

   class UnrecognizedHashType : public Exception
   {
   public:
      std::string hash;
      
      UnrecognizedHashType(std::string hash) : hash(hash), Exception() {
         std::stringstream stream;

         stream << "Unreocgnized hash type: the following hash was not recognized as "
                << "an md5, sha1, sha256 or sha3-384 hash: " << hash;

         this->error = stream.str();
      }
   };

   class HashCollision : public Exception
   {
   public:
      std::string hash;

      HashCollision(const std::string &hash) : hash(hash), Exception() {
         std::stringstream stream;

         stream << "Hash collision: the following hash was encountered more than once in the sample set: " << hash;

         this->error = stream.str();
      }
   };

   class HashNotFound : public Exception
   {
   public:
      std::string hash;

      HashNotFound(const std::string &hash) : hash(hash), Exception() {
         std::stringstream stream;

         stream << "Hash not found: the following hash was not found in the database: " << hash;

         this->error = stream.str();
      }
   };

   class SampleIDNotFound : public Exception
   {
   public:
      std::int64_t id;

      SampleIDNotFound(std::int64_t id) : id(id), Exception() {
         std::stringstream stream;

         stream << "Sample ID not found: the following ID was not found in the database: " << id;

         this->error = stream.str();
      }
   };

   class OriginalFileNotPresent : public Exception
   {
   public:
      OriginalFileNotPresent() : Exception("Original file not present: the original sample file is no longer present with the sample.") {}
   };

   class NullSample : public Exception
   {
   public:
      NullSample() : Exception("Null sample: the data present in the sample is null.") {}
   };

   class ZipException : public Exception
   {
   public:
      zip_error_t *zip_error;

      ZipException(zip_error_t *zip_error) : zip_error(zip_error), Exception() {
         std::stringstream stream;
         
         stream << "Zip error: " << zip_error_strerror(zip_error);

         this->error = stream.str();
      }

      ZipException(int error) : zip_error(nullptr), Exception() {
         std::stringstream stream;

         stream << "Zip error: return code " << std::hex << std::showbase << error;

         this->error = stream.str();
      }
   };

   class InvalidHexString : public Exception
   {
   public:
      std::string input;

      InvalidHexString(const std::string &input) : input(input), Exception() {
         std::stringstream stream;

         stream << "Invalid hex string: the given hex string was not valid in length: " << input;

         this->error = stream.str();
      }
   };

   class InvalidHexCharacter : public Exception
   {
   public:
      char c;

      InvalidHexCharacter(char c) : c(c), Exception() {
         std::stringstream stream;

         stream << "Invalid hex character: the encountered character was not a valid hex character: '" << c << "'";

         this->error = stream.str();
      }
   };

   class SampleNotSaved : public Exception
   {
   public:
      SampleNotSaved() : Exception("Sample not saved: the sample has not yet been saved to the database.") {}
   };

   class NoAlias : public Exception
   {
   public:
      NoAlias() : Exception("No alias: the sample does not have an alias.") {}
   };

   class AliasNotFound : public Exception
   {
   public:
      std::string alias;

      AliasNotFound(const std::string &alias) : alias(alias), Exception() {
         std::stringstream stream;

         stream << "Alias not found: the following alias was not found in the database: " << alias;

         this->error = stream.str();
      }
   };

   class AliasAlreadyExists : public Exception
   {
   public:
      std::string alias;

      AliasAlreadyExists(const std::string &alias) : alias(alias), Exception() {
         std::stringstream stream;

         stream << "Alias already exists: the following alias already exists within the database: " << alias;

         this->error = stream.str();
      }
   };

   class TagNotSaved : public Exception
   {
   public:
      TagNotSaved() : Exception("Tag not saved: a tag was used before it was added to the database.") {}
   };

   class FamilyNotSaved : public Exception
   {
   public:
      FamilyNotSaved() : Exception("Family not saved: a family was used before it was added to the database.") {}
   };

   class TagIDNotFound : public Exception
   {
   public:
      std::int64_t id;

      TagIDNotFound(std::int64_t id) : id(id), Exception() {
         std::stringstream stream;

         stream << "Tag ID not found: the following ID was not found in the tag database: " << id;

         this->error = stream.str();
      }
   };

   class TagExists : public Exception
   {
   public:
      std::string tag;

      TagExists(const std::string &tag) : tag(tag), Exception() {
         std::stringstream stream;

         stream << "Tag exists: the following tag already exists in the database: " << tag;

         this->error = stream.str();
      }
   };

   class BlankTag : public Exception
   {
   public:
      BlankTag() : Exception("Blank tag: the tag being saved is blank.") {}
   };

   class FamilyIDNotFound : public Exception
   {
   public:
      std::int64_t id;

      FamilyIDNotFound(std::int64_t id) : id(id), Exception() {
         std::stringstream stream;

         stream << "Family ID not found: the following ID was not found in the family database: " << id;

         this->error = stream.str();
      }
   };

   class FamilyExists : public Exception
   {
   public:
      std::string family;

      FamilyExists(const std::string &family) : family(family), Exception() {
         std::stringstream stream;

         stream << "Family exists: the following family already exists in the database: " << family;

         this->error = stream.str();
      }
   };

   class BlankFamily : public Exception
   {
   public:
      BlankFamily() : Exception("Blank family: the family being saved is blank.") {}
   };

   class TagNotFound : public Exception
   {
   public:
      std::string tag;

      TagNotFound(const std::string &tag) : tag(tag), Exception() {
         std::stringstream stream;

         stream << "Tag not found: the following tag was not found in the sample: " << tag;

         this->error = stream.str();
      }
   };

   class FamilyNotFound : public Exception
   {
   public:
      std::string family;

      FamilyNotFound(const std::string &family) : family(family), Exception() {
         std::stringstream stream;

         stream << "Family not found: the following family was not found in the sample: " << family;

         this->error = stream.str();
      }
   };

   class SampleNotActive : public Exception
   {
   public:
      SampleNotActive() : Exception("Sample not active: the sample is not currently active.") {}
   };

   class NotInitialized : public Exception
   {
   public:
      NotInitialized() : Exception("Not initialized: the main console application was not initialized.") {}
   };

   class InvalidIdentifier : public Exception
   {
   public:
      std::string ident;

      InvalidIdentifier(const std::string &ident) : ident(ident), Exception() {
         std::stringstream stream;

         stream << "Invalid identifier: the following input was neither "
                << "a sample alias, a filename, md5, sha1, sha256 nor sha3-384 hash: "
                << ident;

         this->error = stream.str();
      }
   };

   class SampleExists : public Exception
   {
   public:
      std::string hash;

      SampleExists(const std::string &hash) : hash(hash), Exception() {
         std::stringstream stream;

         stream << "Sample exists: the following sample hash already exists in the database: "
                << hash;

         this->error = stream.str();
      }
   };

   class NullPointer : public Exception
   {
   public:
      NullPointer() : Exception("Null pointer: encountered an unexpected null pointer.") {}
   };

   class AliasExists : public Exception
   {
   public:
      std::string alias;

      AliasExists(const std::string &alias) : alias(alias), Exception() {
         std::stringstream stream;

         stream << "Alias exists: the following alias already exists in the database: " << alias;

         this->error = stream.str();
      }
   };

   class UnknownLogLevel : public Exception
   {
   public:
      std::string level;

      UnknownLogLevel(const std::string &level) : level(level), Exception() {
         std::stringstream stream;

         stream << "Unknown log level: " << level;

         this->error = stream.str();
      }
   };
   
   class NoSSHConfig : public Exception
   {
   public:
      std::string config;

      NoSSHConfig(const std::string &config) : config(config), Exception() {
         std::stringstream stream;

         stream << "No SSH config: no such SSH site configured in the malexandria config: " << config;

         this->error = stream.str();
      }
   };

   class NoSSHHost : public Exception
   {
   public:
      std::string config;

      NoSSHHost(const std::string &config) : config(config), Exception()
      {
         std::stringstream stream;

         stream << "No SSH host: the following SSH site does not have a host configured: " << config;

         this->error = stream.str();
      }
   };

   class SSHException : public Exception
   {
   public:
      std::string ssh_error;

      SSHException(const std::string &ssh_error) : ssh_error(ssh_error), Exception() {
         std::stringstream stream;

         stream << "SSH Error: " << ssh_error;

         this->error = stream.str();
      }
   };

   class KeyboardInterrupt : public Exception
   {
   public:
      KeyboardInterrupt() : Exception("Keyboard interrupt: encountered a keyboard interrupt character during input.") {}
   };

   class SSHAllocationFailure : public Exception
   {
   public:
      SSHAllocationFailure() : Exception("SSH allocation failure: libssh failed to allocate memory.") {}
   };

   class SSHKeyImportFailure : public Exception
   {
   public:
      std::string keyfile;

      SSHKeyImportFailure(const std::string &keyfile) : keyfile(keyfile), Exception() {
         std::stringstream stream;

         stream << "SSH key import failure: the following key failed to import: " << keyfile;

         this->error = stream.str();
      }
   };

   class SSHHashError : public Exception
   {
   public:
      SSHHashError() : Exception("SSH hash error: failed to get the hash of a public key.") {}
   };

   class SSHHostUnknown : public Exception
   {
   public:
      SSHHostUnknown() : Exception("SSH host unknown: the SSH host is not known or trusted.") {}
   };

   class UpdateSSHKnownHostsFailed : public Exception
   {
   public:
      UpdateSSHKnownHostsFailed() : Exception("Update SSH known_hosts failed: there was an error attempting to update the SSH known hosts file.") {}
   };

   class SSHAuthenticationFailure : public Exception
   {
   public:
      SSHAuthenticationFailure() : Exception("SSH authentication failure: an SSH authentication method failed and another must be tried.") {}
   };

   class SSHPartialAuthentication : public Exception
   {
   public:
      SSHPartialAuthentication() : Exception("SSH partial authentication: further authentication methods necessary.") {}
   };

   class OutOfBounds : public Exception
   {
   public:
      std::size_t offense;
      std::size_t boundary;

      OutOfBounds(std::size_t offense, std::size_t boundary) : offense(offense), boundary(boundary), Exception() {
         std::stringstream stream;

         stream << "Out of bounds: given size " << offense << ", but boundary was " << boundary;

         this->error = stream.str();
      }
   };

   class NotAnSSHURI : public Exception
   {
   public:
      std::string offense;

      NotAnSSHURI(const std::string &offense) : offense(offense), Exception() {
         std::stringstream stream;

         stream << "Not an SSH URI: " << offense;

         this->error = stream.str();
      }
   };

   class SSHHostKeyChanged : public Exception
   {
   public:
      SSHHostKeyChanged() : Exception("SSH host key changed: the key for the SSH site changed unexpectedly.") {}
   };

   class SSHHostKeyConfusion : public Exception
   {
   public:
      SSHHostKeyConfusion() : Exception("SSH host key confusion: a key wasn't found where another type of key exists.") {}
   };

   class ChannelNotOpen : public Exception
   {
   public:
      std::size_t channel;
      
      ChannelNotOpen(std::size_t channel) : channel(channel), Exception() {
         std::stringstream stream;

         stream << "Channel not open: the SSH channel ID " << channel << " was either destroyed or closed.";

         this->error = stream.str();
      }
   };

   class SSHOutputTruncated : public Exception
   {
   public:
      SSHOutputTruncated() : Exception("SSH output truncated: not all output was received from the SSH channel.") {}
   };

   class NoFilename : public Exception
   {
   public:
      NoFilename() : Exception("No filename: no filename was provided in the path expression.") {}
   };

   class RemoteCommandFailure : public Exception
   {
   public:
      std::string command;
      std::string error_output;

      RemoteCommandFailure(const std::string &command, const std::string &error_output)
         : command(command),
           error_output(error_output),
           Exception()
      {
         std::stringstream stream;

         stream << "Remote command failure: the following command failed on the remote server: " << command;

         this->error = stream.str();
      }
   };

   class UnknownSSHEnvironment : public Exception
   {
   public:
      UnknownSSHEnvironment() : Exception("Unknown SSH environment: failed to detect the remote SSH environment.") {}
   };

   class FileOutsideAnalysisPath : public Exception
   {
   public:
      std::string oob;

      FileOutsideAnalysisPath(const std::string &oob) : oob(oob), Exception() {
         std::stringstream stream;

         stream << "File outside analysis path: the following file is outside of the analysis path: " << oob;

         this->error = stream.str();
      }
   };

   class AnalysisNotSaved : public Exception
   {
   public:
      std::string analysis_id;
      
      AnalysisNotSaved(const std::string &analysis_id) : analysis_id(analysis_id), Exception() {
         std::stringstream stream;

         stream << "Analysis not saved: the analysis repo has not been saved: " << analysis_id;

         this->error = stream.str();
      }
   };

   class AnalysisAlreadyCreated : public Exception
   {
   public:
      AnalysisAlreadyCreated() : Exception("Analysis already created: an analysis for the given sample has already been created.") {}
   };

   class AnalysisNotFound : public Exception
   {
   public:
      AnalysisNotFound() : Exception("Analysis not found: an open analysis was not found within the current directory tree.") {}
   };

   class NoSamples : public Exception
   {
   public:
      NoSamples() : Exception("No samples: there are no samples within the analysis.") {}
   };

   class SampleNotFound : public Exception
   {
   public:
      std::string hash;

      SampleNotFound(const std::string &hash) : hash(hash), Exception() {
         std::stringstream stream;

         stream << "Hash not found: the following hash was not found in the analysis: " << hash;

         this->error = stream.str();
      }
   };

   class SampleNotAnalyzed : public Exception
   {
   public:
      std::string sample_id;

      SampleNotAnalyzed(const std::string &sample_id) : sample_id(sample_id), Exception() {
         std::stringstream stream;

         stream << "Sample not analyzed: the following sample is not currently being analyzed: " << sample_id;

         this->error = stream.str();
      }
   };

   class FileExists : public Exception
   {
   public:
      std::string filename;

      FileExists(const std::string &filename) : filename(filename), Exception() {
         std::stringstream stream;

         stream << "File exists: the following file already exists in the analysis: " << filename;

         this->error = stream.str();
      }
   };

   class FileNotAnalyzed : public Exception
   {
   public:
      std::string filename;

      FileNotAnalyzed(const std::string &filename) : filename(filename), Exception() {
         std::stringstream stream;

         stream << "File not analyzed: the following file is not in the analysis: " << filename;

         this->error = stream.str();
      }
   };

   class NoDependencies : public Exception
   {
   public:
      std::string filename;

      NoDependencies(const std::string &filename) : filename(filename), Exception() {
         std::stringstream stream;

         stream << "No dependencies: the following file does not have any dependencies: " << filename;

         this->error = stream.str();
      }
   };

   class InvalidUUID : public Exception
   {
   public:
      std::string id;

      InvalidUUID(const std::string &id) : id(id), Exception() {
         std::stringstream stream;

         stream << "Invalid UUID: the following string is not a valid uuid: " << id;

         this->error = stream.str();
      }
   };

   class NullUUID : public Exception
   {
   public:
      NullUUID() : Exception("Null UUID: the UUID is a nil value.") {}
   };

   class AnalysisNotOpen : public Exception
   {
   public:
      std::string analysis_id;

      AnalysisNotOpen(const std::string &analysis_id) : analysis_id(analysis_id), Exception() {
         std::stringstream stream;

         stream << "Analysis not open: the following analysis ID was not in an open state: " << analysis_id;

         this->error = stream.str();
      }
   };

   class AnalysisAliasNotFound : public Exception
   {
   public:
      std::string alias;

      AnalysisAliasNotFound(const std::string &alias) : alias(alias), Exception() {
         std::stringstream stream;

         stream << "Analysis alias not found: the following analysis alias was not found in the database: " << alias;

         this->error = stream.str();
      }
   };

   class FileNotLinked : public Exception
   {
   public:
      std::string filename;

      FileNotLinked(const std::string &filename) : filename(filename), Exception() {
         std::stringstream stream;

         stream << "File not linked: the following file was not configured with an analysis link: " << filename;

         this->error = stream.str();
      }
   };

   class AnalysisAliasNotSet : public Exception
   {
   public:
      AnalysisAliasNotSet() : Exception("Analysis alias not set: the analysis does not have an alias set.") {}
   };

   class CannotSaveAnalysisConfig : public Exception
   {
   public:
      CannotSaveAnalysisConfig() : Exception("Cannot save analysis config: the analysis has neither declare a config file nor is it saved in the database.") {}
   };

   class AnalysisAliasTaken : public Exception
   {
   public:
      std::string alias;
      
      AnalysisAliasTaken(const std::string &alias) : alias(alias), Exception() {
         std::stringstream stream;

         stream << "Analysis alias taken: the following alias is already taken by another analysis: " << alias;

         this->error = stream.str();
      }
   };

   class TaintSampleRemoved : public Exception
   {
   public:
      std::string sample;

      TaintSampleRemoved(const std::string &sample) : sample(sample), Exception() {
         std::stringstream stream;

         stream << "Taint sample removed: the taint sample for the following hash was removed before it could be saved: " << sample;

         this->error = stream.str();
      }
   };

   class TaintFileRename : public Exception
   {
   public:
      TaintFileRename() : Exception("Taint file rename: attempting to rename the taint file is invalid.") {}
   };

   class NoSourceBuffer : public Exception
   {
   public:
      NoSourceBuffer() : Exception("No source buffer: the zip object does not have a backing source buffer.") {}
   };

   class SampleNotExported : public Exception
   {
   public:
      std::string sample;
      
      SampleNotExported(const std::string &sample) : sample(sample), Exception() {
         std::stringstream stream;

         stream << "Sample not exported: the following hash was not found in the export archive: " << sample;

         this->error = stream.str();
      }
   };


   class AnalysisNotExported : public Exception
   {
   public:
      std::string analysis;
      
      AnalysisNotExported(const std::string &analysis) : analysis(analysis), Exception() {
         std::stringstream stream;

         stream << "Analysis not exported: the following analysis id was not found in the export archive: " << analysis;

         this->error = stream.str();
      }
   };

   class NotADirectory : public Exception
   {
   public:
      std::string directory;

      NotADirectory(const std::string &directory) : directory(directory), Exception() {
         std::stringstream stream;

         stream << "Not a directory: the following file is not a directory: " << directory;

         this->error = stream.str();
      }
   };
}}

#endif
